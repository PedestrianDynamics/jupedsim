import difflib
import os
import pathlib
import shutil
import subprocess

import pytest


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
        # ... tbd

        pass

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


# Creates a test directory where the test is executed and results are written
# This test directory is temporary if tmp_path is used as parameter
def copyAllFiles(*, src: pathlib.Path, dest: pathlib.Path):
    if src == dest:
        return

    # copy all xml files to temporary test directory
    for filename in os.listdir(src):
        pathname = src / filename
        if os.path.isfile(pathname):
            shutil.copy2(pathname, dest)


def checkOutputFilesCreated(working_directory: pathlib.Path):
    # TODO: test if traj, ini and geo have been generated
    pass


def getFileTextDiff(*, expected: pathlib.Path, actual: pathlib.Path):
    with open(expected) as expected_data:
        expected_text = expected_data.readlines()
    with open(actual) as new_data:
        new_text = new_data.readlines()

    diff_text = "".join(
        difflib.context_diff(
            expected_text, new_text, fromfile=str(expected), tofile=str(actual)
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
    copyAllFiles(src=env.systemtest_path / test_directory / "input", dest=tmp_path)

    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    # checkOutputFilesCreated(test_directory)

    # check if there is no diff between expected and new output
    # TODO check for different OS
    expected = env.systemtest_path / test_directory / "expected_linux/traj.txt"
    actual = tmp_path / "results/traj.txt"
    diff = getFileTextDiff(
        expected=expected,
        actual=actual,
    )
    # diff must be empty
    assert diff == ""
