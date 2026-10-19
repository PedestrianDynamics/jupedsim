# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for the pedpy helpers in jupedsim.internal.notebook_utils."""

import pathlib

import jupedsim as jps
import pedpy
import pytest
import shapely
from jupedsim.internal.notebook_utils import read_hdf5_file

ROOM = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
ITERATIONS = 50
EVERY_NTH_FRAME = 2
DT = 0.01
AGENT_POSITIONS = [(2, 5), (3, 4), (3, 6)]


@pytest.fixture
def single_region_file(tmp_path: pathlib.Path) -> pathlib.Path:
    surface = jps.WalkableSurface()
    room = surface.add_region(polygon=ROOM)
    out = tmp_path / "traj.h5"
    with jps.Hdf5TrajectoryWriter(
        output_file=out, every_nth_frame=EVERY_NTH_FRAME
    ) as writer:
        sim = jps.Simulation(
            model=jps.CollisionFreeSpeedModelV2(),
            geometry=surface,
            trajectory_writer=writer,
            dt=DT,
        )
        exit_id = sim.add_exit_stage(
            [(shapely.Polygon([(9, 0), (10, 0), (10, 10), (9, 10)]), room)]
        )
        journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
        for position in AGENT_POSITIONS:
            sim.add_agent(
                journey_id=journey_id,
                stage_id=exit_id,
                position=position,
                state=jps.CollisionFreeSpeedModelV2State(),
                region_id=room,
            )
        for _ in range(ITERATIONS):
            sim.iterate()
    return out


@pytest.fixture
def two_region_file(tmp_path: pathlib.Path) -> tuple[pathlib.Path, int, int]:
    def rectangle(p1, p2):
        return [p1, (p1[0], p2[1]), p2, (p2[0], p1[1])]

    surface = jps.WalkableSurface()
    ground_floor = surface.add_region(
        boundary=rectangle((0, 0), (5, 5)), height=0.0
    )
    upper_floor = surface.add_region(
        boundary=rectangle((10, 0), (15, 5)), height=3.0
    )
    surface.connect_regions(
        from_region=ground_floor,
        from_edge=((5, 0), (5, 5)),
        to_region=upper_floor,
        to_edge=((10, 0), (10, 5)),
    )
    out = tmp_path / "regions.h5"
    with jps.Hdf5TrajectoryWriter(output_file=out) as writer:
        sim = jps.Simulation(
            model=jps.CollisionFreeSpeedModelV2(),
            geometry=surface,
            trajectory_writer=writer,
            dt=DT,
        )
        sim.iterate()
    return out, ground_floor, upper_floor


def test_read_hdf5_file_single_region(single_region_file):
    trajectory, walkable_area = read_hdf5_file(single_region_file)

    assert isinstance(trajectory, pedpy.TrajectoryData)
    assert isinstance(walkable_area, pedpy.WalkableArea)
    assert trajectory.frame_rate == pytest.approx(1 / (DT * EVERY_NTH_FRAME))
    recorded_frames = ITERATIONS // EVERY_NTH_FRAME + 1
    assert len(trajectory.data) == recorded_frames * len(AGENT_POSITIONS)
    assert {"id", "frame", "x", "y"} <= set(trajectory.data.columns)
    assert trajectory.data["frame"].max() == recorded_frames - 1
    assert walkable_area.polygon.equals(ROOM)


def test_read_hdf5_file_accepts_str_path(single_region_file):
    trajectory, _ = read_hdf5_file(str(single_region_file))
    assert len(trajectory.data) > 0


def test_read_hdf5_file_requires_region_id_for_several_regions(
    two_region_file,
):
    path, _, _ = two_region_file
    with pytest.raises(ValueError, match="3 regions"):
        read_hdf5_file(path)


def test_read_hdf5_file_selects_region(two_region_file):
    path, ground_floor, upper_floor = two_region_file
    _, ground = read_hdf5_file(path, region_id=ground_floor)
    _, upper = read_hdf5_file(path, region_id=upper_floor)
    assert ground.polygon.equals(shapely.box(0, 0, 5, 5))
    assert upper.polygon.equals(shapely.box(10, 0, 15, 5))


def test_read_hdf5_file_unknown_region(single_region_file):
    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error, match="Expected exactly 1 region"
    ):
        read_hdf5_file(single_region_file, region_id=42)
