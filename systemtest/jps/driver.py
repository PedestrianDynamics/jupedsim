"""Support to run jpscore from python."""
import os
import subprocess


class JpsCore:
    """
    JpsCore executable wrapper.

    Use this class to run jpscore executable.
    All possible comandline options are available as arguments.

    Use the 'output' field to access the generated output.

    Use the 'returncode' field to check if execution was successful,
    this is a standart unix returncode.
    """

    LOG_LEVEL_DEBUG = "debug"
    LOG_LEVEL_INFO = "info"
    LOG_LEVEL_WARNING = "warning"
    LOG_LEVEL_ERROR = "error"
    LOG_LEVEL_OFF = "off"

    def __init__(
        self,
        executable,
        log_level=LOG_LEVEL_DEBUG,
        input_file="ini.xml",
        work_dir=None,
    ):
        """
        Create a wrapper around the jpscore executable.

        @param executable the path to the jpscore executable you want to test.
        @param log_level to set, this reflects all available log levels from
               the comand line.
        @param input_file to pass to jpscore, if none is provided the
               application is called without a path.
        @param work_dir to be used for execution.
        """
        self.cmd = [executable, f"--log-level={log_level}", input_file]
        self.output = None
        self.returncode = None
        self.work_dir = work_dir

    def run(self):
        """Execute jpscore with the configured parameters."""
        with open(os.path.join(self.work_dir, "log.txt"), "a+") as logfile:
            proc = subprocess.Popen(
                self.cmd,
                stderr=logfile,
                stdout=subprocess.PIPE,
                text=True,
                cwd=self.work_dir,
            )
            proc.communicate()
            logfile.seek(0)
            self.output = logfile.read()
            self.returncode = proc.returncode
