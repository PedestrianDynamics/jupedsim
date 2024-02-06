# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from textwrap import dedent
from typing import Callable

import jupedsim.native as py_jps


def set_debug_callback(fn: Callable[[str], None]) -> None:
    """
    Set receiver for debug messages.

    Arguments:
        fn (fn<str>): function that accepts a msg as string

    """
    py_jps.set_debug_callback(fn)


def set_info_callback(fn: Callable[[str], None]) -> None:
    """
    Set receiver for info messages.

    Arguments:
        fn (fn<str>): function that accepts a msg as string

    """
    py_jps.set_info_callback(fn)


def set_warning_callback(fn: Callable[[str], None]) -> None:
    """
    Set receiver for warning messages.

    Arguments:
        fn (fn<str>): function that accepts a msg as string

    """
    py_jps.set_warning_callback(fn)


def set_error_callback(fn: Callable[[str], None]) -> None:
    """
    Set receiver for error messages.

    Arguments:
        fn (fn<str>): function that accepts a msg as string

    """
    py_jps.set_error_callback(fn)


class BuildInfo:
    def __init__(self) -> None:
        self.__obj = py_jps.get_build_info()

    @property
    def git_commit_hash(self) -> str:
        """SHA1 commit hash of this version.

        Returns:
            SHA1 of this version.

        """
        return self.__obj.git_commit_hash

    @property
    def git_commit_date(self) -> str:
        """Date this commit was created.

        Returns:
            Date the commit of this version as string.

        """
        return self.__obj.git_commit_date

    @property
    def git_branch(self) -> str:
        """Branch this commit was crated from.

        Returns:
            name of the branch this version was build from.

        """
        return self.__obj.git_branch

    @property
    def compiler(self) -> str:
        """Compiler the native code was compiled with.

        Returns:
            Compiler identification.
        """
        return self.__obj.compiler

    @property
    def compiler_version(self) -> str:
        """Compiler version the native code was compiled with.

        Returns:
            Compiler version number.
        """
        return self.__obj.compiler_version

    @property
    def library_version(self) -> str:
        return self.__obj.library_version

    def __repr__(self):
        return dedent(
            f"""\
            JuPedSim {self.library_version}:
            --------------------------------
            Commit: {self.git_commit_hash} from {self.git_branch} on {self.git_commit_date}
            Compiler: {self.compiler} ({self.compiler_version})"""
        )


def get_build_info() -> BuildInfo:
    """Get build information about jupedsim.

    The received :class:`BuildInfo` is printable, e.g.

    .. code:: python

        print(get_build_info())

    This will display a human-readable string stating
    basic information about this library.
    """
    return BuildInfo()
