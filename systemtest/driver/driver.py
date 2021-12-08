import pathlib
import subprocess


class JpsCoreDriver:
    """
    Class to store parameters regarding the execution of jpscore.
    """

    def __init__(
        self, *, jpscore_path: pathlib.Path, working_directory: pathlib.Path
    ):
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
