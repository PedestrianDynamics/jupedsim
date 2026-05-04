# SPDX-License-Identifier: LGPL-3.0-or-later

"""HDF5 trajectory writer.

Writes simulation output in an HDF5 layout that is compatible with the
loaders provided by `PedPy
<https://github.com/PedestrianDynamics/PedPy>`_ for the
`Pedestrian Dynamics Data Archive
<https://ped.fz-juelich.de/da/doku.php?id=info>`_ format. ``h5py`` is an
optional dependency; importing this module raises an informative error
if it is not installed.
"""

from __future__ import annotations

import datetime as _dt
import hashlib
import json
import pathlib
from typing import Final

from shapely import from_wkt

from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation

try:
    import h5py
    import numpy as np
except ImportError as e:  # pragma: no cover - dependency is optional
    raise ImportError(
        "Hdf5TrajectoryWriter requires the optional dependency 'h5py' "
        "(and numpy). Install with: pip install h5py"
    ) from e


SCHEMA_VERSION: Final = 1


def _trajectory_dtype() -> "np.dtype":
    """Compound dtype of the /trajectory dataset.

    The leading four columns (``frame``, ``id``, ``x``, ``y``) match the
    Pedestrian Dynamics Data Archive convention used by PedPy's loader.
    ``z`` is included for byte-level compatibility with the archive
    format and is always written as ``0.0`` (planar simulation). The
    orientation columns are JuPedSim-specific extras.
    """
    return np.dtype(
        [
            ("frame", "<u4"),
            ("id", "<u4"),
            ("x", "<f8"),
            ("y", "<f8"),
            ("z", "<f8"),
            ("ox", "<f8"),
            ("oy", "<f8"),
        ]
    )


def _stable_geometry_hash(wkt: str) -> int:
    """Deterministic 64-bit signed identifier for a WKT string.

    We use a truncated BLAKE2b digest rather than ``hash(wkt)``: Python's
    built-in string hash is salted with ``PYTHONHASHSEED`` and changes
    between processes, so it cannot be persisted into a file format and
    expected to match a future read.
    """
    digest = hashlib.blake2b(wkt.encode("utf-8"), digest_size=8).digest()
    return int.from_bytes(digest, "big", signed=True)


class Hdf5TrajectoryWriter(TrajectoryWriter):
    """Write trajectory data to an HDF5 file.

    The output is laid out as follows:

    - ``/trajectory`` -- compound, resizable dataset with one row per
      agent per recorded frame. Columns: ``frame``, ``id``, ``x``,
      ``y``, ``z``, ``ox``, ``oy``. Carries an ``fps`` attribute (the
      attribute PedPy reads).
    - Root attribute ``wkt_geometry`` -- the initial walkable area as
      WKT (the attribute PedPy reads).
    - Additional root attributes describing the producer, schema
      version, time step, frame interval, bounding box, and creation
      timestamp.
    - ``/geometry/{wkt,hash}`` and ``/frame_geometry`` -- only created
      when the simulation geometry actually changes during the run; for
      runs with a single static geometry the per-frame mapping is
      redundant with the root ``wkt_geometry`` attribute and is
      omitted. PedPy ignores these in any case.

    Parameters
    ----------
    output_file:
        Output HDF5 file path. The file is opened immediately; data is
        flushed in batches.
    every_nth_frame:
        Write every n-th simulation iteration (1 = every frame).
    commit_every_nth_write:
        How many recorded frames to buffer in memory before extending
        the on-disk dataset. Each recorded frame contributes one row
        per agent.
    compression_level:
        gzip level for the trajectory dataset, 0 (off) to 9 (max).
        On trajectory data the bulk of the size reduction is already
        achieved at level 1 (combined with the byte-shuffle filter,
        which is enabled whenever compression is on); higher levels
        give only a few percent extra savings at noticeable extra
        write cost.
    """

    def __init__(
        self,
        *,
        output_file: pathlib.Path,
        every_nth_frame: int = 4,
        commit_every_nth_write: int = 100,
        compression_level: int = 1,
    ) -> None:
        if every_nth_frame < 1:
            raise TrajectoryWriter.Exception("'every_nth_frame' has to be > 0")
        if commit_every_nth_write < 1:
            raise TrajectoryWriter.Exception(
                "'commit_every_nth_write' has to be > 0"
            )
        if not 0 <= compression_level <= 9:
            raise TrajectoryWriter.Exception(
                "'compression_level' must be between 0 and 9"
            )

        self._output_file = pathlib.Path(output_file)
        self._every_nth_frame = every_nth_frame
        self._commit_every_nth_write = commit_every_nth_write
        self._compression_level = compression_level

        self._file: h5py.File | None = h5py.File(self._output_file, "w")
        self._traj_ds: h5py.Dataset | None = None
        self._geom_wkt_ds: h5py.Dataset | None = None
        self._geom_hash_ds: h5py.Dataset | None = None
        self._frame_geom_ds: h5py.Dataset | None = None

        self._buffer: list[tuple] = []
        self._frames_since_flush: int = 0

        # Cache of bounds keyed by deterministic WKT hash so we don't
        # re-parse WKT every frame for static geometry.
        self._bounds_cache: dict[int, tuple[float, float, float, float]] = {}

        # Tracks the unique-geometry registry. Datasets in /geometry are
        # only created once a *second* distinct WKT is observed.
        self._initial_wkt_hash: int | None = None
        self._extra_geometry_hashes: set[int] = set()
        self._frame_geometry_buffer: list[tuple[int, int]] = []
        self._last_recorded_geometry_hash: int | None = None

        self._xmin = float("inf")
        self._xmax = float("-inf")
        self._ymin = float("inf")
        self._ymax = float("-inf")

    def begin_writing(self, simulation: Simulation) -> None:
        if self._file is None:
            raise TrajectoryWriter.Exception("File already closed.")

        fps = 1.0 / simulation.delta_time() / self._every_nth_frame
        wkt = simulation.get_geometry().as_wkt()

        comp_kwargs: dict[str, object] = {}
        if self._compression_level > 0:
            comp_kwargs = {
                "compression": "gzip",
                "compression_opts": self._compression_level,
                "shuffle": True,
            }

        self._traj_ds = self._file.create_dataset(
            "trajectory",
            shape=(0,),
            maxshape=(None,),
            dtype=_trajectory_dtype(),
            chunks=(max(1024, self._commit_every_nth_write * 64),),
            **comp_kwargs,
        )

        # PedPy reads `fps` from the *dataset* attrs; mirror it on the file
        # for parity with the Jülich data archive convention.
        self._traj_ds.attrs["fps"] = fps
        self._traj_ds.attrs["column_units"] = json.dumps(
            {
                "frame": "-",
                "id": "-",
                "x": "m",
                "y": "m",
                "z": "m",
                "ox": "-",
                "oy": "-",
            }
        )
        self._traj_ds.attrs["column_descriptions"] = json.dumps(
            {
                "frame": "frame index",
                "id": "agent id",
                "x": "position (x)",
                "y": "position (y)",
                "z": "position (z), always 0 for planar simulations",
                "ox": "orientation unit vector (x)",
                "oy": "orientation unit vector (y)",
            }
        )

        self._file.attrs["schema_version"] = SCHEMA_VERSION
        self._file.attrs["producer"] = "JuPedSim"
        try:
            from jupedsim import __version__ as _jps_version

            self._file.attrs["producer_version"] = _jps_version
        except Exception:
            pass
        self._file.attrs["dt"] = float(simulation.delta_time())
        self._file.attrs["every_nth_frame"] = self._every_nth_frame
        self._file.attrs["fps"] = fps
        self._file.attrs["wkt_geometry"] = wkt
        self._file.attrs["created"] = _dt.datetime.now(
            _dt.timezone.utc
        ).isoformat()

        self._initial_wkt_hash = _stable_geometry_hash(wkt)
        self._update_bounds(wkt, self._initial_wkt_hash)

    def write_iteration_state(self, simulation: Simulation) -> None:
        if self._file is None or self._traj_ds is None:
            raise TrajectoryWriter.Exception("File not opened.")

        iteration = simulation.iteration_count()
        if iteration % self._every_nth_frame != 0:
            return
        frame = iteration // self._every_nth_frame

        for agent in simulation.agents():
            self._buffer.append(
                (
                    frame,
                    agent.id,
                    agent.position[0],
                    agent.position[1],
                    0.0,
                    agent.orientation[0],
                    agent.orientation[1],
                )
            )

        wkt = simulation.get_geometry().as_wkt()
        wkt_hash = _stable_geometry_hash(wkt)
        self._update_bounds(wkt, wkt_hash)
        self._record_frame_geometry(frame, wkt, wkt_hash)

        self._frames_since_flush += 1
        if self._frames_since_flush >= self._commit_every_nth_write:
            self._flush()

    def close(self) -> None:
        """Flush remaining buffers, write final attributes, and close."""
        if self._file is None:
            return
        if self._traj_ds is not None:
            self._flush()
        if all(
            v not in (float("inf"), float("-inf"))
            for v in (self._xmin, self._xmax, self._ymin, self._ymax)
        ):
            self._file.attrs["xmin"] = self._xmin
            self._file.attrs["xmax"] = self._xmax
            self._file.attrs["ymin"] = self._ymin
            self._file.attrs["ymax"] = self._ymax
        try:
            self._file.close()
        finally:
            self._file = None
            self._traj_ds = None

    def every_nth_frame(self) -> int:
        return self._every_nth_frame

    # ----- internal helpers --------------------------------------------------

    def _update_bounds(self, wkt: str, wkt_hash: int) -> None:
        bounds = self._bounds_cache.get(wkt_hash)
        if bounds is None:
            xmin, ymin, xmax, ymax = from_wkt(wkt).bounds
            bounds = (xmin, ymin, xmax, ymax)
            self._bounds_cache[wkt_hash] = bounds
        xmin, ymin, xmax, ymax = bounds
        self._xmin = min(self._xmin, xmin)
        self._xmax = max(self._xmax, xmax)
        self._ymin = min(self._ymin, ymin)
        self._ymax = max(self._ymax, ymax)

    def _record_frame_geometry(
        self, frame: int, wkt: str, wkt_hash: int
    ) -> None:
        if self._initial_wkt_hash is None:
            # begin_writing always sets this; defensive only.
            self._initial_wkt_hash = wkt_hash
            self._last_recorded_geometry_hash = wkt_hash
            return
        if (
            wkt_hash == self._initial_wkt_hash
            and not self._extra_geometry_hashes
        ):
            # Static geometry so far -- nothing to record.
            self._last_recorded_geometry_hash = wkt_hash
            return

        # Geometry has changed at some point during the run. Make sure
        # the geometry datasets exist and back-fill the registry on the
        # first transition.
        if not self._extra_geometry_hashes:
            self._ensure_geometry_datasets()
            assert self._initial_wkt_hash is not None
            self._append_unique_geometry_record(
                "<initial>", self._initial_wkt_hash, use_initial=True
            )
        if (
            wkt_hash not in self._extra_geometry_hashes
            and wkt_hash != self._initial_wkt_hash
        ):
            self._extra_geometry_hashes.add(wkt_hash)
            self._append_unique_geometry_record(wkt, wkt_hash)

        if wkt_hash != self._last_recorded_geometry_hash:
            self._frame_geometry_buffer.append((frame, wkt_hash))
            self._last_recorded_geometry_hash = wkt_hash

    def _flush(self) -> None:
        if self._traj_ds is None:
            return
        if self._buffer:
            arr = np.array(self._buffer, dtype=_trajectory_dtype())
            old = self._traj_ds.shape[0]
            self._traj_ds.resize((old + arr.shape[0],))
            self._traj_ds[old:] = arr
            self._buffer.clear()

        if self._frame_geometry_buffer:
            self._append_frame_geometry(self._frame_geometry_buffer)
            self._frame_geometry_buffer.clear()

        self._frames_since_flush = 0
        if self._file is not None:
            self._file.flush()

    def _ensure_geometry_datasets(self) -> None:
        assert self._file is not None
        if self._geom_wkt_ds is None:
            grp = self._file.require_group("geometry")
            str_dtype = h5py.string_dtype(encoding="utf-8")
            self._geom_wkt_ds = grp.create_dataset(
                "wkt",
                shape=(0,),
                maxshape=(None,),
                dtype=str_dtype,
                chunks=(8,),
            )
            self._geom_hash_ds = grp.create_dataset(
                "hash",
                shape=(0,),
                maxshape=(None,),
                dtype="<i8",
                chunks=(8,),
            )
            fg_dtype = np.dtype([("frame", "<u4"), ("geometry_hash", "<i8")])
            self._frame_geom_ds = self._file.create_dataset(
                "frame_geometry",
                shape=(0,),
                maxshape=(None,),
                dtype=fg_dtype,
                chunks=(max(1024, self._commit_every_nth_write),),
            )

    def _append_unique_geometry_record(
        self, wkt: str, wkt_hash: int, *, use_initial: bool = False
    ) -> None:
        assert self._geom_wkt_ds is not None
        assert self._geom_hash_ds is not None
        old = self._geom_wkt_ds.shape[0]
        self._geom_wkt_ds.resize((old + 1,))
        self._geom_hash_ds.resize((old + 1,))
        # The initial WKT is already preserved as the root `wkt_geometry`
        # attribute; we only need a hash entry for the registry.
        if use_initial and self._file is not None:
            self._geom_wkt_ds[old] = self._file.attrs["wkt_geometry"]
        else:
            self._geom_wkt_ds[old] = wkt
        self._geom_hash_ds[old] = wkt_hash

    def _append_frame_geometry(self, rows: list[tuple[int, int]]) -> None:
        assert self._frame_geom_ds is not None
        arr = np.array(
            rows,
            dtype=np.dtype([("frame", "<u4"), ("geometry_hash", "<i8")]),
        )
        old = self._frame_geom_ds.shape[0]
        self._frame_geom_ds.resize((old + arr.shape[0],))
        self._frame_geom_ds[old:] = arr
