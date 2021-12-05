import pathlib
import os
import platform
from enum import Enum, auto


class Platform(Enum):
    LINUX = auto()
    MACOS = auto()
    WINDOWS = auto()


class Environment:
    """
    Class to store paths and settings that are required for all tests.
    """

    def __init__(self):
        def canonicalize(p: pathlib.Path):
            return p.expanduser().absolute()

        self.jpscore_path = canonicalize(
            pathlib.Path(str(os.getenv("JPSCORE_EXECUTABLE_PATH")))
        )
        self.systemtest_path = canonicalize(
            pathlib.Path((str(os.getenv("JPSCORE_SOURCE_PATH"))))
            / "systemtest"
        )

        tmp_system = platform.system()
        if tmp_system == "Linux":
            self.operating_system = Platform.LINUX
        elif tmp_system == "Darwin":
            self.operating_system = Platform.MACOS
        elif tmp_system == "Windows":
            self.operating_system == Platform.WINDOWS
        else:
            raise OSError("OS not supported")
