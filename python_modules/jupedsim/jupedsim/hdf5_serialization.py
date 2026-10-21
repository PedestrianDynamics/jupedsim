# SPDX-License-Identifier: LGPL-3.0-or-later

"""HDF5 trajectory writer and loader, schema version 2.

File Layout:
```txt
    /
    │  @schema_version   int  layout version, currently 2
    │  @producer         str  "JuPedSim"
    │  @producer_version str  JuPedSim version that wrote the file
    │  @dt               f8   simulation time step [s]
    │  @every_nth_frame  int  iterations between two recorded frames
    │  @created          str  creation time, ISO 8601 in UTC
    ├── trajectory
    │   │  One row per recorded agent and frame, ordered by frame
    │   │  shuffle + zstd
    │   │  type compound {
    │   │    frame     <u8 recorded-frame index
    │   │    id        <u8 agent id
    │   │    x         <f4 position x [m]
    │   │    y         <f4 position y [m]
    │   │    z         <f4 position z [m]
    │   │    region_id <u8 region the agent is in, see /regions
    │   │  }
    │   │  @frame     str "Frame this record belongs to"
    │   │  @id        str "Id of this agent"
    │   │  @x         str "X position of the agent [m] x/y forms the ground plane"
    │   │  @y         str "Y position of the agent [m] x/y forms the ground plane"
    │   │  @z         str "Z position of the agent [m] z is up"
    │   └─ @region_id str "Region the agent is in"
    ├── frame_offsets
    │   │  <u8, shuffle + zstd
    │   │  Row in /trajectory at which each recorded frame starts, followed
    │   │  by the total row count: rows of frame i are
    │   └─ trajectory[frame_offsets[i]:frame_offsets[i + 1]]
    ├── regions
    │   │  One row per region
    │   │  type compound {
    │   │    id  <u8 region id
    │   │    wkt str 2D region polygon as WKT
    │   │  }
    │   │  @id  str "id of the region"
    │   └─ @wkt str "2D WKT describing the region polygon."
    │               "This is always a projection onto X/Y for connectors."
    ├── mesh
    │   ├── vertices
    │   │   │  All vertices of the geometry
    │   │   └─ [N, 3] <f4
    │   ├── triangles
    │   │   │  Vertex indices of each triangle
    │   │   └─ [M, 3] <u8
    │   └── regions
    │       │  Region id of each triangle
    │       └─ [M] <u8
    └── travel_times   only with write_travel_times=True, written on close
        │  Floor field travel times to every waypoint and exit stage, one
        │  grid per stage and region
        ├── grids
        │   │  One row per grid, ordered by stage id, then region id
        │   │  type compound {
        │   │    stage_id  <u8 stage the travel times lead to
        │   │    region_id <u8 region the grid covers, see /regions
        │   │    origin_x  <f8 x of the lower-left corner of cell (0, 0) [m]
        │   │    origin_y  <f8 y of the lower-left corner of cell (0, 0) [m]
        │   │    width     <u4 cells along x
        │   │    height    <u4 cells along y
        │   │    cell_size <f8 edge length of a cell [m]
        │   │    offset    <u8 first element of the grid in /travel_times/values
        │   │  }
        │   └─ @<field>   str description of each field, as for /trajectory
        ├── values
        │   │  <f4, shuffle + zstd
        │   │  Travel times [s] of all grids, one after another, each
        │   │  row-major from its lowest row: grid i is
        │   │  values[offset:offset + width * height].reshape(height, width)
        │   └─ Zero inside the stage's area, inf where a cell is not walkable
        └── directions
            │  [N, 2] <f4, shuffle + zstd, row k belongs to values[k]
            │  Walking direction (x, y) at each cell's centre, the unit
            │  vector an agent there is sent along: grid i is
            │  directions[offset:offset + width * height].reshape(height, width, 2)
            └─ Zero inside the stage's area, nan where a cell is not walkable
```

Reading the file requires the Zstd filter: ``import hdf5plugin`` before
opening it with h5py.
"""

from __future__ import annotations

import dataclasses
import types
import typing
from datetime import datetime, timezone
from pathlib import Path
from typing import Final, override

import h5py
import hdf5plugin
import numpy as np
import numpy.typing as npt

from jupedsim.pyjupedsim import Geometry, get_build_info
from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation

SCHEMA_VERSION: Final = 2


class Hdf5TrajectoryWriter(TrajectoryWriter):
    """Write trajectory data to an HDF5 file, see the module docs for the layout.

    Rows are buffered and written in chunks of 64 KiB. The file is flushed after
    every chunk, so an interrupted run keeps the metadata, the geometry and the
    trajectory rows up to the last chunk; /frame_offsets may be incomplete then.
    Call :meth:`close` when the simulation is done to write the remaining rows
    and the frame index, or use the writer in a ``with`` block.

    Args:
        output_file: File to write to; an existing file is overwritten.
        every_nth_frame: Record every n-th iteration, 1 records all of them.
        write_travel_times: Also store the floor field travel times to every
            waypoint and exit stage in /travel_times. They are written by
            :meth:`close`, so stages added during the run are included, and
            fields no agent needed are solved then. Not written when a
            ``with`` block ends on an exception, see
            :meth:`close_after_error`.
    """

    def __init__(
        self,
        *,
        output_file: Path,
        every_nth_frame: int = 4,
        write_travel_times: bool = False,
    ):
        if every_nth_frame < 1:
            raise TrajectoryWriter.Error("'every_nth_frame' has to be > 0")
        self._output_file: Path = output_file
        self._every_nth_frame: int = every_nth_frame
        self._write_travel_times: bool = write_travel_times
        self._simulation: Simulation | None = None
        self._records_to_cache: int = 2**16 // self.trajectory_dtype().itemsize
        self._trajectory_buffer: npt.NDArray[typing.Any] = np.zeros(
            self._records_to_cache, dtype=self.trajectory_dtype()
        )
        self._record_idx: int = 0
        self._frame_offset_buffer: npt.NDArray[typing.Any] = np.zeros(
            1024, dtype=np.dtype("<u8")
        )
        self._is_writing: bool = False

    @override
    def begin_writing(self, simulation: Simulation) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be written. E.g. Meta information
        such as frame rate etc...

        Args:
            simulation: Simulation object to query for data.
        """
        if self._is_writing:
            raise TrajectoryWriter.Error("'begin_writing()' already called!")
        self._file: h5py.File = h5py.File(str(self._output_file), "w")
        try:
            self._write_header(simulation)
        except BaseException:
            # Don't leave a locked, half-written file behind.
            self._file.close()
            Path(self._output_file).unlink(missing_ok=True)
            raise
        self._record_idx = 0
        self._records_written = 0
        self._frame_idx = 0
        self._is_writing = True
        if self._write_travel_times:
            # Kept until close, which writes the travel times.
            self._simulation = simulation
        self._file.flush()

    def _write_header(self, simulation: Simulation) -> None:
        """Write the root attributes, create the datasets and store the geometry.

        Args:
            simulation: Simulation object to query.
        """
        self._file.attrs["schema_version"] = SCHEMA_VERSION
        self._file.attrs["producer"] = "JuPedSim"
        self._file.attrs["producer_version"] = get_build_info().library_version
        self._file.attrs["dt"] = simulation.delta_time()
        self._file.attrs["every_nth_frame"] = self._every_nth_frame
        self._file.attrs["created"] = datetime.now(timezone.utc).isoformat()

        self._trajectory_ds: h5py.Dataset = self._file.create_dataset(
            "trajectory",
            shape=(0,),
            maxshape=(None,),
            dtype=self.trajectory_dtype(),
            chunks=self._records_to_cache,
            shuffle=True,
            **hdf5plugin.Zstd(clevel=3),  # pyright: ignore[reportUnknownArgumentType]
        )
        self._trajectory_ds.attrs["frame"] = "Frame this record belongs to"
        self._trajectory_ds.attrs["id"] = "Id of this agent"
        self._trajectory_ds.attrs["x"] = (
            "X position of the agent [m] x/y forms the ground plane"
        )
        self._trajectory_ds.attrs["y"] = (
            "Y position of the agent [m] x/y forms the ground plane"
        )
        self._trajectory_ds.attrs["z"] = "Z position of the agent [m] z is up"
        self._trajectory_ds.attrs["region_id"] = "Region the agent is in"

        self._frame_offsets_ds: h5py.Dataset = self._file.create_dataset(
            "frame_offsets",
            shape=(0,),
            maxshape=(None,),
            dtype=np.dtype("<u8"),
            chunks=1024,
            shuffle=True,
            **hdf5plugin.Zstd(clevel=3),  # pyright: ignore[reportUnknownArgumentType]
        )
        geo: Geometry = simulation.get_geometry()

        regions = [
            (idx, geo.polygon(region_id=idx).as_wkt())
            for idx in range(geo.region_count())
        ]
        regions_arr = np.array(regions, dtype=self.regions_dtype())
        regions_ds = self._file.create_dataset("regions", data=regions_arr)
        regions_ds.attrs["id"] = "Id of the region."
        regions_ds.attrs["wkt"] = (
            "2D WKT describing the region polygon. This is always a projection onto X/Y for connectors."
        )

        vertices = geo.vertices()
        vertices_arr = np.asarray(vertices, dtype="<f4")
        self._file.create_dataset("mesh/vertices", data=vertices_arr)

        triangles = geo.triangles()
        triangles_arr = np.asarray(triangles, dtype="<u8")
        self._file.create_dataset("mesh/triangles", data=triangles_arr)

        triangle_regions = geo.region_id_per_face()
        triangle_regions_arr = np.array(triangle_regions, dtype=np.dtype("<u8"))
        self._file.create_dataset("mesh/regions", data=triangle_regions_arr)

    @override
    def write_iteration_state(self, simulation: Simulation) -> None:
        """Write trajectory data of one simulation iteration.

        This method is intended to handle serialization of the trajectory data
        of a single iteration.

        Args:
            simulation: Simulation object to query.
        """
        if not self._is_writing:
            raise TrajectoryWriter.Error(
                "Need to call 'begin_writing()' before calling 'write_iteration_state()'"
            )
        iteration = simulation.iteration_count()
        if iteration % self._every_nth_frame != 0:
            return
        frame = iteration // self._every_nth_frame

        self._frame_offset_buffer[self._frame_idx] = self._records_written
        self._frame_idx += 1
        if self._frame_idx == 1024:
            self._append(
                self._frame_offsets_ds,
                self._frame_offset_buffer,
                self._frame_idx,
            )
            self._frame_idx = 0

        for agent in simulation._obj.agents():
            self._records_written += 1
            loc = agent.location
            self._trajectory_buffer[self._record_idx] = (
                frame,
                agent.id,
                loc.x,
                loc.y,
                loc.z,
                loc.region_id,
            )
            self._record_idx += 1
            if self._record_idx == self._records_to_cache:
                self._append(
                    self._trajectory_ds,
                    self._trajectory_buffer,
                    self._record_idx,
                )
                self._record_idx = 0

    @override
    def every_nth_frame(self) -> int:
        """Returns the interval of this writer in frames between writes.

        1 indicates all frames are written, 10 indicates every 10th frame is
        written and so on.

        Returns:
            Number of frames between writes as int

        """
        return self._every_nth_frame

    @override
    def close(self) -> None:
        """Write the remaining rows, the frame index and, if requested, the
        travel times, then close the file."""
        if not self._is_writing:
            return
        self._is_writing = False
        try:
            self._frame_offset_buffer[self._frame_idx] = self._records_written
            self._frame_idx += 1
            self._append(
                self._frame_offsets_ds,
                self._frame_offset_buffer,
                self._frame_idx,
            )
            if self._record_idx != 0:
                self._append(
                    self._trajectory_ds,
                    self._trajectory_buffer,
                    self._record_idx,
                )
            if self._simulation is not None:
                try:
                    self._write_travel_time_grids(self._simulation)
                except BaseException:
                    # All or nothing: no values without the grids that index them.
                    if "travel_times" in self._file:
                        del self._file["travel_times"]
                    raise
        finally:
            self._simulation = None
            self._file.close()

    @override
    def close_after_error(self) -> None:
        """Write the remaining rows and the frame index, but no travel times:
        solving every floor field while an error propagates would delay it and
        could replace it with an error of its own."""
        self._simulation = None
        self.close()

    def _write_travel_time_grids(self, simulation: Simulation) -> None:
        """Write /travel_times for every stage with an area and every region.

        Each grid is appended as soon as it is computed, so memory holds one grid at a
        time rather than all of them.
        """
        values_ds = self._file.create_dataset(
            "travel_times/values",
            shape=(0,),
            maxshape=(None,),
            dtype="<f4",
            chunks=(2**16,),
            shuffle=True,
            **hdf5plugin.Zstd(clevel=3),  # pyright: ignore[reportUnknownArgumentType]
        )
        values_ds.attrs["unit"] = "s"
        directions_ds = self._file.create_dataset(
            "travel_times/directions",
            shape=(0, 2),
            maxshape=(None, 2),
            dtype="<f4",
            chunks=(2**15, 2),
            shuffle=True,
            **hdf5plugin.Zstd(clevel=3),  # pyright: ignore[reportUnknownArgumentType]
        )

        rows = []
        offset = 0
        # A stage without an area, such as direct steering, has no grids.
        for stage_id in simulation.stage_ids():
            for region_id, grid in simulation.travel_times(stage_id).items():
                rows.append(
                    (
                        stage_id,
                        region_id,
                        *grid.origin,
                        grid.width,
                        grid.height,
                        grid.cell_size,
                        offset,
                    )
                )
                values = grid.values.astype("<f4").ravel()
                self._append(values_ds, values, len(values))
                directions = grid.directions.astype("<f4").reshape(-1, 2)
                self._append(directions_ds, directions, len(directions))
                offset += grid.width * grid.height

        grids_ds = self._file.create_dataset(
            "travel_times/grids",
            data=np.array(rows, dtype=self.travel_time_grid_dtype()),
        )
        grids_ds.attrs["stage_id"] = "Stage the travel times lead to"
        grids_ds.attrs["region_id"] = "Region the grid covers"
        grids_ds.attrs["origin_x"] = (
            "X of the lower-left corner of cell (0, 0) [m]"
        )
        grids_ds.attrs["origin_y"] = (
            "Y of the lower-left corner of cell (0, 0) [m]"
        )
        grids_ds.attrs["width"] = "Number of cells along x"
        grids_ds.attrs["height"] = "Number of cells along y"
        grids_ds.attrs["cell_size"] = "Edge length of a cell [m]"
        grids_ds.attrs["offset"] = (
            "First element of the grid in /travel_times/values"
        )

    @staticmethod
    def trajectory_dtype() -> "np.dtype":
        """Compound dtype of the /trajectory dataset."""
        return np.dtype(
            [
                ("frame", "<u8"),
                ("id", "<u8"),
                ("x", "<f4"),
                ("y", "<f4"),
                ("z", "<f4"),
                ("region_id", "<u8"),
            ],
            align=True,
        )

    @staticmethod
    def regions_dtype() -> "np.dtype":
        """Compound dtype of the /regions dataset."""
        return np.dtype(
            [("id", "<u8"), ("wkt", h5py.string_dtype())], align=True
        )

    @staticmethod
    def travel_time_grid_dtype() -> "np.dtype":
        """Compound dtype of the /travel_times/grids dataset."""
        return np.dtype(
            [
                ("stage_id", "<u8"),
                ("region_id", "<u8"),
                ("origin_x", "<f8"),
                ("origin_y", "<f8"),
                ("width", "<u4"),
                ("height", "<u4"),
                ("cell_size", "<f8"),
                ("offset", "<u8"),
            ],
            align=True,
        )

    @staticmethod
    def _append(
        dset: h5py.Dataset, data: npt.NDArray[typing.Any], count: int
    ) -> None:
        """Append data to a Hdf5 dataset.

        Args:
            dset: dataset to append to
            data: data to append
            count: number of entries
        """
        n: int = dset.shape[0]
        dset.resize(n + count, axis=0)
        dset[n:] = data[:count]
        dset.file.flush()


class Hdf5TrajectoryLoader:
    """HDF5 trajectory loader.

    Loads files written by :class:`Hdf5TrajectoryWriter`. The file is opened
    read-only on construction and validated against the schema; use
    :meth:`close` or a ``with`` block to release it. Travel times are only in
    files written with ``write_travel_times=True``, see
    :meth:`has_travel_times`.
    """

    class Error(Exception):
        """Type for all errors raised while loading a trajectory from an HDF5 file."""

    @dataclasses.dataclass(frozen=True)
    class TravelTimeGrid:
        """Travel times and walking directions to one stage over the floor field
        grid of one region, as stored in /travel_times.

        Has the fields of :class:`~jupedsim.TravelTimeGrid`, so code written
        against :meth:`~jupedsim.Simulation.travel_times` works on a loaded file
        too. Values and directions are stored as 32-bit floats.
        """

        origin: tuple[float, float]
        """Lower-left corner of cell (0, 0) in metres."""
        width: int
        """Number of cells along x."""
        height: int
        """Number of cells along y."""
        cell_size: float
        """Edge length of a cell in metres."""
        values: npt.NDArray[np.float32]
        """Travel times in seconds, shape ``(height, width)``; row 0 is the
        lowest. Zero inside the stage's area, ``inf`` where a cell is not
        walkable."""
        directions: npt.NDArray[np.float32]
        """Walking direction at each cell's centre, shape ``(height, width, 2)``.
        Zero inside the stage's area, ``nan`` where a cell is not walkable."""

        @property
        def extent(self) -> tuple[float, float, float, float]:
            """``(x_min, x_max, y_min, y_max)`` of the grid in metres, as
            matplotlib's ``extent`` expects it."""
            x, y = self.origin
            return (
                x,
                x + self.width * self.cell_size,
                y,
                y + self.height * self.cell_size,
            )

    def __init__(self, path: Path) -> None:
        """Opens and validates an HDF5 trajectory file.

        Args:
            path: path to the hdf5 file to load.

        Raises:
            Hdf5TrajectoryLoader.Error: if the file does not follow the schema.
        """
        self._file: h5py.File = h5py.File(str(path), "r")
        try:
            self._validate()
        except Exception:
            # Don't leak the open handle (and the HDF5 file lock) on failure.
            self._file.close()
            raise

    def _validate(self) -> None:
        producer = self._read_root_attr("producer", str)
        if producer != "JuPedSim":
            raise Hdf5TrajectoryLoader.Error(
                f"Not a JuPedSim HDF5 file. Found producer={producer}"
            )

        schema_version = self._read_root_attr("schema_version", int)
        if schema_version != SCHEMA_VERSION:
            raise Hdf5TrajectoryLoader.Error(
                f"Unexpected schema version in file. Encountered {schema_version}, expected {SCHEMA_VERSION}"
            )

        dt = self._read_root_attr("dt", float)
        if dt <= 0:
            raise Hdf5TrajectoryLoader.Error(
                f"Root attribute 'dt' has to be > 0, found {dt}"
            )
        every_nth_frame = self._read_root_attr("every_nth_frame", int)
        if every_nth_frame < 1:
            raise Hdf5TrajectoryLoader.Error(
                f"Root attribute 'every_nth_frame' has to be > 0, found {every_nth_frame}"
            )
        self._fps = 1 / (dt * every_nth_frame)

        trajectory_ds = self._file.get("trajectory")
        if not isinstance(trajectory_ds, h5py.Dataset):
            raise Hdf5TrajectoryLoader.Error("Trajectory dataset missing.")
        if trajectory_ds.dtype != Hdf5TrajectoryWriter.trajectory_dtype():
            raise Hdf5TrajectoryLoader.Error(
                f"Trajectory dataset has wrong datatype: {trajectory_ds.dtype}, expected {Hdf5TrajectoryWriter.trajectory_dtype()}"
            )

        regions_ds = self._file.get("regions")
        if not isinstance(regions_ds, h5py.Dataset):
            raise Hdf5TrajectoryLoader.Error("Regions dataset missing.")
        if regions_ds.dtype != Hdf5TrajectoryWriter.regions_dtype():
            raise Hdf5TrajectoryLoader.Error(
                f"Regions dataset has wrong datatype: {regions_ds.dtype}, expected {Hdf5TrajectoryWriter.regions_dtype()}"
            )
        self._trajectory: h5py.Dataset = trajectory_ds
        self._regions: h5py.Dataset = regions_ds

        self._travel_time_grids: npt.NDArray[typing.Any] | None = None
        if "travel_times" in self._file:
            self._validate_travel_times()

    def _validate_travel_times(self) -> None:
        def dataset(name: str) -> h5py.Dataset:
            ds = self._file.get(f"travel_times/{name}")
            if not isinstance(ds, h5py.Dataset):
                raise Hdf5TrajectoryLoader.Error(
                    f"Travel times dataset '{name}' missing."
                )
            return ds

        grids_ds = dataset("grids")
        if grids_ds.dtype != Hdf5TrajectoryWriter.travel_time_grid_dtype():
            raise Hdf5TrajectoryLoader.Error(
                f"Travel times grids dataset has wrong datatype: {grids_ds.dtype}, expected {Hdf5TrajectoryWriter.travel_time_grid_dtype()}"
            )
        values_ds = dataset("values")
        if values_ds.ndim != 1 or values_ds.dtype != np.dtype("<f4"):
            raise Hdf5TrajectoryLoader.Error(
                f"Travel times values dataset has to be 1-D <f4, found {values_ds.ndim}-D {values_ds.dtype}"
            )
        directions_ds = dataset("directions")
        if (
            directions_ds.ndim != 2
            or directions_ds.shape[1] != 2
            or directions_ds.dtype != np.dtype("<f4")
        ):
            raise Hdf5TrajectoryLoader.Error(
                f"Travel times directions dataset has to be [N, 2] <f4, found {list(directions_ds.shape)} {directions_ds.dtype}"
            )
        if directions_ds.shape[0] != values_ds.shape[0]:
            raise Hdf5TrajectoryLoader.Error(
                f"Travel times hold {values_ds.shape[0]} values but {directions_ds.shape[0]} directions"
            )

        # One row per grid, small enough to keep: loading a stage then needs no
        # second read of /travel_times/grids.
        grids = grids_ds[:]
        ends = grids["offset"] + grids["width"].astype("<u8") * grids["height"]
        if len(grids) and ends.max() > values_ds.shape[0]:
            bad = grids[np.argmax(ends > values_ds.shape[0])]
            raise Hdf5TrajectoryLoader.Error(
                f"Travel time grid of stage_id={bad['stage_id']}, region_id={bad['region_id']} runs past the {values_ds.shape[0]} stored values"
            )
        self._travel_time_grids = grids
        self._travel_time_values: h5py.Dataset = values_ds
        self._travel_time_directions: h5py.Dataset = directions_ds

    def _read_root_attr[T](
        self, name: str, cast: typing.Callable[[typing.Any], T]
    ) -> T:
        if name not in self._file.attrs:
            raise Hdf5TrajectoryLoader.Error(
                f"Root attribute '{name}' missing."
            )
        value = self._file.attrs[name]
        if isinstance(value, bytes):
            value = value.decode("utf-8")
        try:
            return cast(value)
        except Exception as e:
            raise Hdf5TrajectoryLoader.Error(
                f"Root attribute '{name}' cannot be cast to {cast.__name__}: {e}"
            ) from e

    def close(self) -> None:
        """Closes the underlying HDF5 file. Safe to call more than once."""
        self._file.close()

    def __enter__(self) -> typing.Self:
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        traceback: types.TracebackType | None,
    ) -> None:
        self.close()

    def load_region_ids(self) -> list[int]:
        """Returns the ids of all regions stored in the file."""
        return np.asarray(self._regions.fields("id")[:]).tolist()

    def load_region_trajectory(self, region_id: int) -> dict[str, typing.Any]:
        """Loads the trajectory of all agents while inside one region.

        Args:
            region_id: id of the region, see :meth:`load_region_ids`.

        Returns:
            A dict with keys ``wkt`` (the region polygon as WKT string),
            ``fps`` (recorded frames per second) and ``frame``, a dict with
            ``columns`` (``["id", "frame", "x", "y"]``) and ``data`` (a
            structured array with exactly these fields) suitable for
            ``pandas.DataFrame(**result["frame"])``.

        Raises:
            Hdf5TrajectoryLoader.Error: if ``region_id`` is not in the file.
        """
        column_index = ["id", "frame", "x", "y"]

        regions = self._regions[:]
        wkt_list = regions[regions["id"] == region_id]["wkt"].tolist()
        if len(wkt_list) != 1:
            raise Hdf5TrajectoryLoader.Error(
                f"Expected exactly 1 region for id={region_id}, found {len(wkt_list)}"
            )
        wkt = wkt_list[0].decode("utf-8")
        # One sequential read + numpy mask is far cheaper than h5py point
        # selection with a large index array.
        trajectory = self._trajectory[:]
        data = trajectory[trajectory["region_id"] == region_id][column_index]
        return {
            "wkt": wkt,
            "fps": self._fps,
            "frame": {"columns": column_index, "data": data},
        }

    def has_travel_times(self) -> bool:
        """Whether the file holds /travel_times, i.e. was written with
        ``write_travel_times=True`` and closed without an error."""
        return self._travel_time_grids is not None

    def load_travel_time_stage_ids(self) -> list[int]:
        """Returns the ids of all stages with travel times in the file, ascending.

        Raises:
            Hdf5TrajectoryLoader.Error: if the file holds no travel times.
        """
        grids = self._require_travel_time_grids()
        return np.unique(grids["stage_id"]).tolist()

    def load_travel_times(
        self, stage_id: int
    ) -> dict[int, Hdf5TrajectoryLoader.TravelTimeGrid]:
        """Loads the travel times to one stage, the counterpart of
        :meth:`~jupedsim.Simulation.travel_times`.

        Args:
            stage_id: id of the stage, see :meth:`load_travel_time_stage_ids`.

        Returns:
            The grid of every region by region id, in ascending order.

        Raises:
            Hdf5TrajectoryLoader.Error: if the file holds no travel times, or none
                for ``stage_id``.
        """
        grids = self._require_travel_time_grids()
        rows = grids[grids["stage_id"] == stage_id]
        if len(rows) == 0:
            raise Hdf5TrajectoryLoader.Error(
                f"No travel times for stage_id={stage_id}, found stages {self.load_travel_time_stage_ids()}"
            )
        result: dict[int, Hdf5TrajectoryLoader.TravelTimeGrid] = {}
        for row in np.sort(rows, order="region_id"):
            width, height = int(row["width"]), int(row["height"])
            begin = int(row["offset"])
            end = begin + width * height
            result[int(row["region_id"])] = Hdf5TrajectoryLoader.TravelTimeGrid(
                origin=(float(row["origin_x"]), float(row["origin_y"])),
                width=width,
                height=height,
                cell_size=float(row["cell_size"]),
                values=self._travel_time_values[begin:end].reshape(
                    height, width
                ),
                directions=self._travel_time_directions[begin:end].reshape(
                    height, width, 2
                ),
            )
        return result

    def _require_travel_time_grids(self) -> npt.NDArray[typing.Any]:
        if self._travel_time_grids is None:
            raise Hdf5TrajectoryLoader.Error(
                "File holds no travel times; write it with write_travel_times=True."
            )
        return self._travel_time_grids
