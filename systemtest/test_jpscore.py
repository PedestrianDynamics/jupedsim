import pathlib
import platform
import pytest
from driver.fixtures import env
from driver.utils import setup_jpscore_driver, get_file_text_diff
from driver.trajectories import load_trajectory
from driver.environment import Platform


@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="No reference data for Windows available",
)
@pytest.mark.parametrize(
    "test_directory",
    [
        (pathlib.Path("reference_tests/RT01_corridor_GCFM_global-shortest/")),
        (
            pathlib.Path(
                "reference_tests/RT02_corridor_velocity_global-shortest/"
            )
        ),
    ],
)
def test_reference_data(tmp_path, env, test_directory: pathlib.Path):
    """
    Ensures that the output of jpscore has not changed.

    For this purpose reference trajectory files for Linux/Mac are compared with the actual generated trajectory files.
    The tests passes if there is no difference in the expected and actual trajectory files.
    The test is skipped for Windows.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param test_directory: directory of the test
    """
    jpscore_driver = setup_jpscore_driver(
        env=env, working_directory=tmp_path, test_directory=test_directory
    )
    jpscore_driver.run()

    # check if there is no diff between expected and new output
    # use different files for different OS
    expected = None
    if env.operating_system == Platform.LINUX:
        expected = (
            env.systemtest_path / test_directory / "expected_linux/traj.txt"
        )
    elif env.operating_system == Platform.MACOS:
        expected = (
            env.systemtest_path / test_directory / "expected_mac/traj.txt"
        )
    assert expected

    actual = jpscore_driver.traj_file
    diff = get_file_text_diff(
        expected=expected,
        actual=actual,
    )
    # diff must be empty
    assert diff == ""


@pytest.mark.parametrize(
    "test_directory, expected_evac_time, tolerance",
    [
        (
            pathlib.Path("juelich_tests/evacuation_time/single_ped_corridor"),
            10.0,
            0.5,
        )
    ],
)
def test_evac_time_single_ped(
    tmp_path,
    env,
    test_directory: pathlib.Path,
    expected_evac_time: float,
    tolerance: float,
):
    """
    Ensures that the evacuation for a single pedestrian in free flow lies within an accepted range.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param test_directory: directory of the test
    :param expected_evac_time: expected evacuation time
    :param tolerance: tolerated difference to actual evacuation time
    """
    jpscore_driver = setup_jpscore_driver(
        env=env, working_directory=tmp_path, test_directory=test_directory
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)
    actual_evac_time = (
        max(trajectories.data[:, 1]) - min(trajectories.data[:, 1])
    ) / trajectories.framerate
    diff_evac_times = abs(actual_evac_time - expected_evac_time)

    # difference in evac times must be in tolerance range
    assert diff_evac_times < tolerance
