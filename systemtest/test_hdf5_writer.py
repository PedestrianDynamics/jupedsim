# SPDX-License-Identifier: LGPL-3.0-or-later
"""Round-trip tests for Hdf5TrajectoryWriter."""

import pathlib

import jupedsim as jps
import pytest
import shapely
from shapely import GeometryCollection

h5py = pytest.importorskip("h5py")


@pytest.fixture
def square_simulation(tmp_path: pathlib.Path):
    area = GeometryCollection(
        shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    )
    out = tmp_path / "traj.h5"
    writer = jps.Hdf5TrajectoryWriter(
        output_file=out, every_nth_frame=1, compression_level=1
    )
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModelV2(),
        geometry=area,
        trajectory_writer=writer,
        dt=0.01,
    )
    exit_id = sim.add_exit_stage(
        shapely.Polygon([(9, 0), (10, 0), (10, 10), (9, 10)])
    )
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    for x, y in [(2, 5), (3, 4), (3, 6)]:
        sim.add_agent(
            jps.CollisionFreeSpeedModelV2AgentParameters(
                position=(x, y), journey_id=journey_id, stage_id=exit_id
            )
        )
    for _ in range(50):
        sim.iterate()
    writer.close()
    return out


def test_required_layout_is_pedpy_compatible(square_simulation):
    with h5py.File(square_simulation, "r") as hf:
        assert "trajectory" in hf, "missing /trajectory dataset"
        ds = hf["trajectory"]

        for col in ("frame", "id", "x", "y"):
            assert col in ds.dtype.names, f"missing column '{col}'"

        assert "fps" in ds.attrs, "missing fps attribute on /trajectory"
        assert ds.attrs["fps"] > 0

        assert "wkt_geometry" in hf.attrs, "missing root wkt_geometry"
        assert "POLYGON" in hf.attrs["wkt_geometry"]


def test_payload_shape_and_values(square_simulation):
    with h5py.File(square_simulation, "r") as hf:
        ds = hf["trajectory"]
        data = ds[:]
        assert data.shape[0] > 0
        assert (data["frame"] >= 0).all()
        assert (data["id"] > 0).all()
        assert (0 <= data["x"]).all() and (data["x"] <= 10).all()
        assert (0 <= data["y"]).all() and (data["y"] <= 10).all()
        assert (data["z"] == 0.0).all()


def test_metadata_attributes(square_simulation):
    with h5py.File(square_simulation, "r") as hf:
        for key in (
            "schema_version",
            "producer",
            "dt",
            "every_nth_frame",
            "fps",
            "created",
            "xmin",
            "xmax",
            "ymin",
            "ymax",
        ):
            assert key in hf.attrs, f"missing root attribute '{key}'"
        assert hf.attrs["producer"] == "JuPedSim"
        assert hf.attrs["schema_version"] == 1


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
    with h5py.File(out, "r") as hf:
        assert "trajectory" not in hf


def test_static_geometry_omits_geometry_group(square_simulation):
    """For runs with a single geometry the /geometry group is absent."""
    with h5py.File(square_simulation, "r") as hf:
        assert "geometry" not in hf
        assert "frame_geometry" not in hf
        assert "wkt_geometry" in hf.attrs


def test_geometry_hash_is_deterministic():
    """The geometry hash must not depend on PYTHONHASHSEED."""
    from jupedsim.hdf5_serialization import _stable_geometry_hash

    wkt = "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))"
    assert _stable_geometry_hash(wkt) == _stable_geometry_hash(wkt)
    assert _stable_geometry_hash(wkt) != _stable_geometry_hash(
        "POLYGON ((0 0, 2 0, 2 2, 0 2, 0 0))"
    )
