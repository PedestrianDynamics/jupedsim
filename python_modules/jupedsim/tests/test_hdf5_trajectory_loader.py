# SPDX-License-Identifier: LGPL-3.0-or-later
"""Hdf5TrajectoryLoader unit tests."""

import pathlib
from contextlib import contextmanager

import h5py
import jupedsim as jps
import numpy as np
import pandas as pd
import pytest

TRAJECTORY_DATA = np.array(
    [
        (0, 0, 5.0, 5.0, 0.0, 1337),
        (1, 0, 5.0, 5.0, 0.0, 1337),
        (2, 0, 5.0, 5.0, 0.0, 1337),
    ],
    dtype=jps.Hdf5TrajectoryWriter.trajectory_dtype(),
)
WKT_DATA = np.array(
    [(1337, "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))")],
    dtype=jps.Hdf5TrajectoryWriter.regions_dtype(),
)


@contextmanager
def h5_file(file_path: pathlib.Path):
    h5_file: h5py.File = h5py.File(str(file_path), "x")
    h5_file.attrs["producer"] = "JuPedSim"
    h5_file.attrs["schema_version"] = "2"
    h5_file.attrs["dt"] = 0.01
    h5_file.attrs["every_nth_frame"] = 5
    trajectory_ds = h5_file.create_dataset(
        "trajectory",
        shape=(0,),
        maxshape=(None,),
        dtype=jps.Hdf5TrajectoryWriter.trajectory_dtype(),
    )
    n_trajectory = trajectory_ds.shape[0]
    trajectory_ds.resize(size=n_trajectory + len(TRAJECTORY_DATA), axis=0)
    trajectory_ds[n_trajectory:] = TRAJECTORY_DATA
    regions_ds = h5_file.create_dataset(
        "regions",
        shape=(0,),
        maxshape=(None,),
        dtype=jps.Hdf5TrajectoryWriter.regions_dtype(),
    )
    n_regions = regions_ds.shape[0]
    regions_ds.resize(n_regions + len(WKT_DATA), axis=0)
    regions_ds[n_regions:] = WKT_DATA
    try:
        yield h5_file
    finally:
        h5_file.close()


def test_hdf5_trajectory_loading_fails_nicely_on_mising_schema_version(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f.attrs["schema_version"]

    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Root attribute 'schema_version' missing",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_on_wrong_schema_version_type(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        f.attrs["schema_version"] = "s"

    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Root attribute 'schema_version' cannot be cast",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_on_wrong_schema_version_value(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        f.attrs["schema_version"] = 1

    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match=r"Unexpected schema version in file",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_on_missing_producer(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f.attrs["producer"]

    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Root attribute 'producer' missing",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_on_wrong_producer_value(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        f.attrs["producer"] = "JuPedZim"

    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Not a JuPedSim HDF5 file",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_on_missing_trajectory(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f["trajectory"]
    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Trajectory dataset missing",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_with_wrong_trajectory_type(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f["trajectory"]
        f.create_dataset(
            "trajectory",
            shape=(0,),
            maxshape=(None,),
            dtype=np.float64,
        )
    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Trajectory dataset has wrong datatype",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_on_missing_regions(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f["regions"]
    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Regions dataset missing",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_fails_nicely_with_wrong_regions_type(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f["regions"]
        f.create_dataset(
            "regions",
            shape=(0,),
            maxshape=(None,),
            dtype=np.float64,
        )
    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match="Regions dataset has wrong datatype",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


@pytest.mark.parametrize("attr", ["dt", "every_nth_frame"])
def test_hdf5_trajectory_loading_fails_nicely_on_missing_frame_rate_attr(
    tmp_path, attr
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f.attrs[attr]
    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match=f"Root attribute '{attr}' missing",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


@pytest.mark.parametrize("attr", ["dt", "every_nth_frame"])
def test_hdf5_trajectory_loading_fails_nicely_on_zero_frame_rate_attr(
    tmp_path, attr
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        f.attrs[attr] = 0
    with pytest.raises(
        jps.Hdf5TrajectoryLoader.Error,
        match=f"Root attribute '{attr}' has to be > 0",
    ):
        jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_accepts_bytes_producer(tmp_path) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        f.attrs["producer"] = np.bytes_(b"JuPedSim")
    jps.Hdf5TrajectoryLoader(path=p).close()


def test_hdf5_trajectory_loading_closes_file_on_failure(tmp_path) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p) as f:
        del f.attrs["producer"]
    with pytest.raises(jps.Hdf5TrajectoryLoader.Error):
        jps.Hdf5TrajectoryLoader(path=p)
    # Would fail with a lock error if the loader left its handle open.
    with h5py.File(p, "r+") as f:
        f.attrs["producer"] = "JuPedSim"


def test_hdf5_trajectory_loading_context_manager_closes_file(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p):
        pass
    with jps.Hdf5TrajectoryLoader(path=p) as loader:
        assert np.array_equal(loader.load_region_ids(), [1337])
    with h5py.File(p, "r+") as f:
        f.attrs["producer"] = "JuPedSim"


def test_hdf5_trajectory_loading_unknown_region_reports_id(tmp_path) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p):
        pass
    with (
        jps.Hdf5TrajectoryLoader(path=p) as loader,
        pytest.raises(
            jps.Hdf5TrajectoryLoader.Error,
            match="Expected exactly 1 region for id=42, found 0",
        ),
    ):
        loader.load_region_trajectory(42)


def test_hdf5_trajectory_loading_good_case(
    tmp_path,
) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p):
        pass

    jps.Hdf5TrajectoryLoader(path=p)


def test_hdf5_trajectory_loading_create_pd_dataframe(tmp_path) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p):
        pass
    with jps.Hdf5TrajectoryLoader(path=p) as loader:
        res = loader.load_region_trajectory(1337)
    columns = ["id", "frame", "x", "y"]
    assert res["frame"]["columns"] == columns
    assert res["frame"]["data"].dtype.names == tuple(columns)
    frame = pd.DataFrame(**res["frame"])
    assert list(frame.columns) == columns
    for column in columns:
        assert np.array_equal(
            res["frame"]["data"][column], TRAJECTORY_DATA[column]
        )
        assert np.array_equal(frame[column].to_numpy(), TRAJECTORY_DATA[column])
    assert res["wkt"] == WKT_DATA["wkt"][0]
    assert res["fps"] == 20


def test_hdf5_trajectory_loading_load_region_ids(tmp_path) -> None:
    p = tmp_path / "sample.h5"
    with h5_file(p):
        pass
    with jps.Hdf5TrajectoryLoader(p) as loader:
        assert np.array_equal(loader.load_region_ids(), [1337])
