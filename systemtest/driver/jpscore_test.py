import difflib
import os
import pathlib
import shutil
import subprocess

import pytest


class JpsCoreDriver:
    def __init__(self):
        self.jpscore_path = pathlib.Path(str(os.getenv("JPSCORE_EXECUTABLE_PATH")))
        self.system_test_dir = pathlib.Path().absolute().parent
        self.result = None
        # ... tbd

        pass

    def run(self, working_directory: pathlib.Path, test_directory: pathlib.Path):

        self._setupTestDir(working_directory, test_directory)

        self.result = subprocess.run(
            [self.jpscore_path, "inifile.xml"],
            cwd=str(working_directory / test_directory),
            capture_output=True,
            text=True,
        )

        # checkReturnCode?

        # TODO write log to file here?

        return

    # Creates a test directory where the test is executed and results are written
    # This test directory is temporary if tmp_path is used as parameter
    def _setupTestDir(
        self, working_directory: pathlib.Path, test_directory: pathlib.Path
    ):

        # TODO check src != dest
        src_dir = self.system_test_dir / test_directory
        dst_dir = working_directory / test_directory

        # creat new test directory in directory of execution
        subprocess.run(["mkdir -p " + str(dst_dir)], shell=True)
        # TODO check result?

        # copy all xml files to temporary test directory
        # TODO maybe this should be realized with a subprocess as well?
        for basename in os.listdir(src_dir):
            if basename.endswith(".xml"):
                pathname = os.path.join(src_dir, basename)
                if os.path.isfile(pathname):
                    shutil.copy2(pathname, dst_dir)

    def logfile(self):
        return self.result.stdout

    def trajfile(self):
        pass

    def stats(self):
        pass

    def systemTestDir(self) -> pathlib.Path:
        return self.system_test_dir


@pytest.fixture()
def jpscore_driver():
    # TODO do we really need a fixture?
    return JpsCoreDriver()


def checkReturnCode():
    # needed?
    pass


def checkOutputFilesCreated(working_directory: pathlib.Path):
    # TODO: test if traj, ini and geo have been generated
    pass


def getFileTextDiff(expected_file: pathlib.Path, new_file: pathlib.Path):

    with open(expected_file) as expected_data:
        expected_text = expected_data.readlines()
    with open(new_file) as new_data:
        new_text = new_data.readlines()

    diff_text = "".join(
        difflib.context_diff(
            expected_text, new_text, fromfile=str(expected_file), tofile=str(new_file)
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
def test_reference_data(tmp_path, jpscore_driver, test_directory: pathlib.Path):

    jpscore_driver.run(tmp_path, test_directory)
    # checkOutputFilesCreated(test_directory)

    # check if there is no diff between expected and new output
    # TODO check for different OS
    diff = getFileTextDiff(
        pathlib.Path(
            jpscore_driver.systemTestDir(), test_directory, "output_expected/traj.txt"
        ),
        pathlib.Path(tmp_path, test_directory, "results/traj.txt"),
    )
    # diff must be empty
    assert diff == ""
