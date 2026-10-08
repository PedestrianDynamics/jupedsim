# SPDX-License-Identifier: LGPL-3.0-or-later

"""HDF5 trajectory writer, schema version 2.

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
    └── mesh
        ├── vertices
        │   │  All vertices of the geometry
        │   └─ [N, 3] <f4
        ├── triangles
        │   │  Vertex indices of each triangle
        │   └─ [M, 3] <u8
        └── regions
            │  Region id of each triangle
            └─ [M] <u8
```

Reading the file requires the Zstd filter: ``import hdf5plugin`` before
opening it with h5py.
"""

from __future__ import annotations

import typing
from datetime import datetime, timezone
from pathlib import Path
from typing import Final, override

import h5py
import hdf5plugin
import numpy as np
import numpy.typing as npt

from jupedsim.native import Geometry, get_build_info
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
    """

    def __init__(
        self,
        *,
        output_file: Path,
        every_nth_frame: int = 4,
    ):
        if every_nth_frame < 1:
            raise TrajectoryWriter.Exception("'every_nth_frame' has to be > 0")
        self._output_file: Path = output_file
        self._every_nth_frame: int = every_nth_frame
        self._records_to_cache: int = 2**16 // self._trajectory_dtype().itemsize
        self._trajectory_buffer: npt.NDArray[typing.Any] = np.empty(
            self._records_to_cache, dtype=self._trajectory_dtype()
        )
        self._record_idx: int = 0
        self._frame_offset_buffer: npt.NDArray[typing.Any] = np.empty(
            1024, dtype=np.dtype("<u8")
        )
        self._is_writing: bool = False

    @override
    def begin_writing(self, simulation: Simulation) -> None:
        """Begin writing trajectory data.

        This method is intended to handle all data writing that has to be done
        once before the trajectory data can be written. E.g. Meta information
        such as frame rate etc...

        """
        if self._is_writing:
            raise TrajectoryWriter.Exception(
                "'begin_writing()' already called!"
            )
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
        self._file.flush()

    def _write_header(self, simulation: Simulation) -> None:
        """Write the root attributes, create the datasets and store the geometry."""
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
            dtype=self._trajectory_dtype(),
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
        regions_dt = np.dtype(
            [("id", "<u8"), ("wkt", h5py.string_dtype())], align=True
        )
        regions_arr = np.array(regions, dtype=regions_dt)
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

        """
        if not self._is_writing:
            raise TrajectoryWriter.Exception(
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
        """Write the remaining rows and the frame index, then close the file."""
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
        finally:
            self._file.close()

    @staticmethod
    def _trajectory_dtype() -> "np.dtype":
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
    def _append(
        dset: h5py.Dataset, data: npt.NDArray[typing.Any], count: int
    ) -> None:
        n: int = dset.shape[0]
        dset.resize(n + count, axis=0)
        dset[n:] = data[:count]
        dset.file.flush()
