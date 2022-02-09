import difflib
import os
import pathlib
import shutil
from typing import List

from itertools import tee
from driver.driver import JpsCoreDriver
from driver.environment import Environment


def setup_jpscore_driver(
    *,
    env: Environment,
    working_directory: pathlib.Path,
    test_directory: pathlib.Path,
):
    """
    Sets up jpscore driver by preparing the working directory and creating a driver.

    :param env: global environment object
    :param working_directory: working directory where jpscore should be executed
    :param test_directory: directory of the test
    :return: JpsCoreDriver object
    """
    copy_all_files(
        src=env.systemtest_path / test_directory / "input",
        dest=working_directory,
    )

    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=working_directory
    )
    return jpscore_driver


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
            expected_text,
            actual_text,
            fromfile=str(expected),
            tofile=str(actual),
        )
    )

    return diff_text


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


def copy_files(*, sources: List[pathlib.Path], dest: pathlib.Path):
    """
    Copies all specified files to the destination folder.

    NOTES:
    If multiple input files have the same name the later ones will override the
    previous one in the target destination.

    Only files in 'sources' are copied. Folders are silently ignored.

    :param sources a list of paths to copy
    :param dest to copy to
    """
    for path in sources:
        if path.is_file():
            shutil.copy2(path, dest)

def pairwise(iterable):
    "s -> (s0,s1), (s1,s2), (s2, s3), ..."
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)
