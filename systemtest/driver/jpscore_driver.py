import pytest
import subprocess 
import pathlib
import os

class JpsCoreDriver():
    def __init__(self, executable_path: pathlib.Path, working_directory: pathlib.Path, inifile: pathlib.Path):
        # self.proc = subprocess.Popen()
        # ... tbd
        pass

    def run(self):
        pass

    def logfile(self):
        pass

    

exe_path = pathlib.Path("~/jsc/jpscore-build/bin/jpscore")


# Test case 1
@pytest.mark.parametrize("a,b", [("a1", "b1"), ("a2", "b2")])
def test_all_pedestrians_evacuated(tmp_path, a, b):
    out = pathlib.Path(tmp_path) / "out.txt"
    out.write_text(f"{a}:{b}")
    print(os.getenv("JPSCORE_EXECUTABLE_PATH"))
