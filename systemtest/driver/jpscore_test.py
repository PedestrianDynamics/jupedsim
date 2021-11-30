import difflib
import os
import pathlib
import platform
import shutil
import subprocess

import pytest


# class to store paths that are required for all tests
class Environment:
    def __init__(self):
        self.jpscore_path = pathlib.Path(str(os.getenv("JPSCORE_EXECUTABLE_PATH")))
        self.systemtest_path = (
            pathlib.Path(str(os.getenv("JPSCORE_SOURCE_PATH"))) / "systemtest"
        )


@pytest.fixture
def env():
    return Environment()


class JpsCoreDriver:
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


# Copies all files that exist in source to destination
def copy_all_files(*, src: pathlib.Path, dest: pathlib.Path):
    if src == dest:
        return

    for filename in os.listdir(src):
        pathname = src / filename
        if os.path.isfile(pathname):
            shutil.copy2(pathname, dest)


def check_output_files_created(working_directory: pathlib.Path):
    # TODO: test if traj, ini and geo have been generated
    pass


def get_file_text_diff(*, expected: pathlib.Path, actual: pathlib.Path):
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


# Reference tests
@pytest.mark.parametrize(
    "test_directory",
    [
        (pathlib.Path("reference_tests/RT01_corridor_GCFM_global-shortest/")),
        (pathlib.Path("reference_tests/RT02_corridor_velocity_global-shortest/")),
    ],
)
def test_reference_data(tmp_path, env, test_directory):
    copy_all_files(src=env.systemtest_path / test_directory / "input", dest=tmp_path)

    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    # checkOutputFilesCreated(test_directory)

    # check if there is no diff between expected and new output
    # use different files for different OS
    operating_system = platform.system()
    if operating_system == "Linux":
        expected = env.systemtest_path / test_directory / "expected_linux/traj.txt"
    elif operating_system == "Darwin":
        expected = env.systemtest_path / test_directory / "expected_mac/traj.txt"
    else:
        raise NotImplementedError()

    actual = tmp_path / "results/traj.txt"
    diff = get_file_text_diff(
        expected=expected,
        actual=actual,
    )
    # diff must be empty
    assert diff == ""
