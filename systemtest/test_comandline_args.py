"""Systemtests for jpscore."""
import os

import pytest

from jps.checker import output_contains_line_exact
from jps.driver import JpsCore


@pytest.yield_fixture
def fixture(tmpdir):
    """Testfixture providing a temp directory and the path to jpscore."""
    jpscore_path = os.getenv(key="JPSCORE", default="jpscore")
    assert os.path.exists(
        jpscore_path
    ), f"Cannot find jpscore under {jpscore_path}"
    yield (tmpdir, jpscore_path)


def test_checks_file_argument_exists(fixture):
    """
    Checks jpscore error message about ini.xml.

    This tests check that jpscore will report if the supplied path does
    not point to a file.
    """
    temp_folder, executable_path = fixture
    jps_core = JpsCore(
        executable=executable_path,
        input_file="I-DO-NOT-EXIST",
        work_dir=str(temp_folder),
    )
    jps_core.run()
    expected_message = "inifile: File does not exist: I-DO-NOT-EXIST"
    assert jps_core.returncode != 0
    has_expected_message = output_contains_line_exact(
        jps_core.output, expected_message
    )
    assert has_expected_message
