import difflib
import os
import pathlib
import platform
import shutil
import subprocess
from enum import Enum

import pytest

import utils


class Environment:
    """
    Class to store paths and settings that are required for all tests.
    """

    class OS(Enum):
        LINUX = 1
        MAC = 2
        WINDOWS = 3

    def __init__(self):
        def canonicalize(p: pathlib.Path):
            return p.expanduser().absolute()

        self.jpscore_path = canonicalize(
            pathlib.Path(str(os.getenv("JPSCORE_EXECUTABLE_PATH")))
        )
        self.systemtest_path = canonicalize(
            pathlib.Path((str(os.getenv("JPSCORE_SOURCE_PATH")))) / "systemtest"
        )

        tmp_system = platform.system()
        if tmp_system == "Linux":
            self.operating_system = Environment.OS(1)
        elif tmp_system == "Darwin":
            self.operating_system = Environment.OS(2)
        elif tmp_system == "Windows":
            self.operating_system == Environment.OS(3)
        else:
            raise OSError("OS not supported")


@pytest.fixture
def env():
    return Environment()


class JpsCoreDriver:
    """
    Class to store parameters regarding the execution of jpscore.
    """

    def __init__(self, *, jpscore_path: pathlib.Path, working_directory: pathlib.Path):
        self.working_directory = working_directory
        self.jpscore_path = jpscore_path
        self.result = None
        self.logfile = self.working_directory / "logfile.txt"
        self.traj_file = self.working_directory / "results/traj.txt"

    def run(self):
        with open(self.logfile, "w") as logfile:
            self.result = subprocess.run(
                [self.jpscore_path, "inifile.xml"],
                cwd=self.working_directory,
                stdout=logfile,
                stderr=subprocess.STDOUT,
            )

        self.result.check_returncode()

    def logfile(self):
        return self.logfile

    def trajfile(self):
        return self.traj_file


def setup_jpscore_driver(
    *, env: Environment, working_directory: pathlib.Path, test_directory: pathlib.Path
):
    """
    Sets up jpscore driver by preparing the working directory and creating a driver.

    :param env: global environment object
    :param working_directory: working directory where jpscore should be executed
    :param test_directory: directory of the test
    :return: JpsCoreDriver object
    """
    copy_all_files(
        src=env.systemtest_path / test_directory / "input", dest=working_directory
    )

    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=working_directory
    )
    return jpscore_driver


def copy_all_files(*, src: pathlib.Path, dest: pathlib.Path):
    """
    Copies all files that exist in source directory to destination directory.
    No directories are copied.

    :param src: source directory
    :param dest: destination directory
    """
    if src == dest:
        return

    for filename in os.listdir(src):
        pathname = src / filename
        if os.path.isfile(pathname):
            shutil.copy2(pathname, dest)


def get_file_text_diff(*, expected: pathlib.Path, actual: pathlib.Path):
    """
    Compares the actual with an expected text file and returns their difference.

    :param expected: text file to expect
    :param actual: actual text file
    :return: string with differing text
    """
    with open(expected) as expected_data:
        expected_text = expected_data.readlines()
    with open(actual) as actual_data:
        actual_text = actual_data.readlines()

    diff_text = "".join(
        difflib.context_diff(
            expected_text, actual_text, fromfile=str(expected), tofile=str(actual)
        )
    )

    return diff_text


@pytest.mark.skipif(
    platform.system() == "Windows", reason="No reference data for Windows available"
)
@pytest.mark.parametrize(
    "test_directory",
    [
        (pathlib.Path("reference_tests/RT01_corridor_GCFM_global-shortest/")),
        (pathlib.Path("reference_tests/RT02_corridor_velocity_global-shortest/")),
    ],
)
def test_reference_data(tmp_path, env, test_directory: pathlib.Path):
    """
    Ensures that the output of jpscore has not changed.

    For this purpose reference trajectory files for Linux/Mac are compared with the actual genereated trajectory files.
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
    if env.operating_system == Environment.OS.LINUX:
        expected = env.systemtest_path / test_directory / "expected_linux/traj.txt"
    elif env.operating_system == Environment.OS.MAC:
        expected = env.systemtest_path / test_directory / "expected_mac/traj.txt"

    actual = jpscore_driver.traj_file
    diff = get_file_text_diff(
        expected=expected,
        actual=actual,
    )
    # diff must be empty
    assert diff == ""


@pytest.mark.parametrize(
    "test_directory, expected_evac_time, tolerance",
    [(pathlib.Path("juelich_tests/evacuation_time/single_ped_corridor"), 10.0, 0.5)],
)
def test_evac_time_single_ped(
    tmp_path,
    env,
    test_directory: pathlib.Path,
    expected_evac_time: float,
    tolerance: float,
):
    """
    Ensures that the evacuation for a single pedestrian in free flow lies withing an accepted range.

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

    fps, N, traj_data = utils.read_traj_file(jpscore_driver.traj_file)
    actual_evac_time = (max(traj_data[:, 1]) - min(traj_data[:, 1])) / fps
    diff_evac_times = abs(actual_evac_time - expected_evac_time)

    # difference in evac times must be in tolerance range
    assert diff_evac_times < tolerance
