# SPDX-License-Identifier: LGPL-3.0-or-later
"""Round-trip tests for Hdf5TrajectoryWriter."""

import pathlib

import h5py
import jupedsim as jps
import pytest
import shapely
from shapely import GeometryCollection


@pytest.fixture
def square_simulation(tmp_path: pathlib.Path):
    area = GeometryCollection(
        shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    )
    out_filename = tmp_path / "traj.h5"
    writer = jps.Hdf5TrajectoryWriter(
        output_file=out_filename,
        every_nth_frame=1,
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=area,
        trajectory_writer=writer,
        dt=0.01,
    )
    exit_id = sim.add_exit_stage(
        shapely.Polygon([(9, 0), (10, 0), (10, 10), (9, 10)]),
        region_id=0,
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    for x, y in [(2, 5), (3, 4), (3, 6)]:
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=(x, y),
            state=jps.CollisionFreeSpeedModelV2State(),
            region_id=0,
        )
    for _ in range(50):
        sim.iterate()
    writer.close()
    return out_filename


def test_payload_shape_and_values(square_simulation):
    with h5py.File(square_simulation, "r") as hf:
        data = hf["trajectory"][:]
        offsets = hf["frame_offsets"][:]

    assert data.dtype.names == ("frame", "id", "x", "y", "z", "region_id")
    assert [data.dtype[name].str for name in data.dtype.names] == [
        "<u8",
        "<u8",
        "<f4",
        "<f4",
        "<f4",
        "<u8",
    ]

    # Iteration 0 plus 50 iterations, each recorded (every_nth_frame=1),
    # with all 3 agents still on their way to the exit.
    frame_count = 51
    assert data.shape == (frame_count * 3,)

    # frame_offsets holds the start row of every frame plus a final end marker.
    assert offsets.shape == (frame_count + 1,)
    assert offsets[0] == 0
    assert offsets[-1] == data.shape[0]
    for frame in range(frame_count):
        rows = data[offsets[frame] : offsets[frame + 1]]
        assert (rows["frame"] == frame).all()
        assert sorted(rows["id"]) == sorted(data[:3]["id"])

    assert (data["id"] > 0).all()
    assert (0 <= data["x"]).all() and (data["x"] <= 10).all()
    assert (0 <= data["y"]).all() and (data["y"] <= 10).all()
    assert (data["z"] == 0.0).all()
    assert (data["region_id"] == 0).all()


def test_metadata_attributes(square_simulation):
    with h5py.File(square_simulation, "r") as hf:
        for key in (
            "schema_version",
            "producer",
            "dt",
            "every_nth_frame",
            "created",
        ):
            assert key in hf.attrs, f"missing root attribute '{key}'"
        assert hf.attrs["producer"] == "JuPedSim"
        assert hf.attrs["schema_version"] == 2


def test_close_is_idempotent(tmp_path):
    area = GeometryCollection(shapely.Polygon([(0, 0), (5, 0), (5, 5), (0, 5)]))
    out = tmp_path / "empty.h5"
    writer = jps.Hdf5TrajectoryWriter(output_file=out, every_nth_frame=1)
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=area,
        trajectory_writer=writer,
        dt=0.01,
    )
    for _ in range(3):
        sim.iterate()
    writer.close()
    writer.close()  # second call must not raise


def test_close_without_begin_writing(tmp_path):
    """Closing an unused writer must not raise (no /trajectory dataset)."""
    out = tmp_path / "untouched.h5"
    writer = jps.Hdf5TrajectoryWriter(output_file=out, every_nth_frame=1)
    writer.close()
    assert not out.exists()


def test_mesh(square_simulation):
    # The 10x10 square is triangulated into 2 triangles over its 4 corners.
    with h5py.File(square_simulation, "r") as hf:
        assert hf["mesh/vertices"].shape == (4, 3)
        assert hf["mesh/triangles"].shape == (2, 3)
        assert hf["mesh/regions"].shape == (2,)
        assert (hf["mesh/regions"][:] == 0).all()


def test_regions_round_trip(tmp_path):
    """Two floors joined by stairs are written to /regions and read back."""

    def rectangle(p1, p2):
        return [p1, (p1[0], p2[1]), p2, (p2[0], p1[1])]

    surface = jps.WalkableSurface()
    ground_floor = surface.add_region(
        exterior=rectangle((0, 0), (5, 5)), height=0.0
    )
    upper_floor = surface.add_region(
        exterior=rectangle((10, 0), (15, 5)), height=3.0
    )
    stairs = surface.connect_regions(
        from_region=ground_floor,
        from_edge=((5, 0), (5, 5)),
        to_region=upper_floor,
        to_edge=((10, 0), (10, 5)),
    )
    expected = {
        ground_floor: shapely.box(0, 0, 5, 5),
        upper_floor: shapely.box(10, 0, 15, 5),
        stairs: shapely.box(5, 0, 10, 5),
    }

    out = tmp_path / "regions.h5"
    writer = jps.Hdf5TrajectoryWriter(output_file=out, every_nth_frame=1)
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=surface,
        trajectory_writer=writer,
        dt=0.01,
    )
    sim.iterate()
    writer.close()

    with h5py.File(out, "r") as hf:
        regions = hf["regions"][:]
        triangle_regions = hf["mesh/regions"][:]

    assert regions.dtype.names == ("id", "wkt")
    read = {
        int(row["id"]): shapely.from_wkt(row["wkt"].decode()) for row in regions
    }
    assert read.keys() == expected.keys()
    for region_id, polygon in expected.items():
        assert read[region_id].equals(polygon), region_id

    # Every region owns at least one triangle of the mesh.
    assert set(triangle_regions.tolist()) == set(expected)


def _square_simulation(writer):
    area = GeometryCollection(shapely.Polygon([(0, 0), (5, 0), (5, 5), (0, 5)]))
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=area,
        trajectory_writer=writer,
        dt=0.01,
    )


def test_with_block_closes_writer(tmp_path):
    out = tmp_path / "with.h5"
    with jps.Hdf5TrajectoryWriter(output_file=out, every_nth_frame=1) as writer:
        assert isinstance(writer, jps.Hdf5TrajectoryWriter)
        sim = _square_simulation(writer)
        for _ in range(3):
            sim.iterate()

    # The frame index is only written on close: frames 0..3 plus end marker.
    with h5py.File(out, "r") as hf:
        assert hf["frame_offsets"].shape == (5,)
    writer.close()  # closing again after the block must not raise


def test_with_block_closes_writer_on_exception(tmp_path):
    out = tmp_path / "raised.h5"
    with pytest.raises(RuntimeError, match="boom"):
        with jps.Hdf5TrajectoryWriter(
            output_file=out, every_nth_frame=1
        ) as writer:
            sim = _square_simulation(writer)
            sim.iterate()
            raise RuntimeError("boom")

    with h5py.File(out, "r") as hf:
        assert hf["frame_offsets"].shape == (3,)
