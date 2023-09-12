# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps

from textwrap import dedent


# TODO(kkratz): add typehints for function params
def set_debug_callback(fn) -> None:
    """
    Set reciever for debug messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_debug_callback(fn)


def set_info_callback(fn) -> None:
    """
    Set reciever for info messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_info_callback(fn)


def set_warning_callback(fn) -> None:
    """
    Set reciever for warning messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_warning_callback(fn)


def set_error_callback(fn) -> None:
    """
    Set reciever for error messages.

    Parameters
    ----------
    fn: fn<str>
        function that accepts a msg as string
    """
    py_jps.set_error_callback(fn)


# TODO(kkratz): refactor this into free functions in C-API / bindings
class BuildInfo:
    def __init__(self):
        self.__obj = py_jps.get_build_info()

    @property
    def git_commit_hash(self) -> str:
        return self.__obj.git_commit_hash

    @property
    def git_commit_date(self) -> str:
        return self.__obj.git_commit_date

    @property
    def git_branch(self) -> str:
        return self.__obj.git_branch

    @property
    def compiler(self) -> str:
        return self.__obj.compiler

    @property
    def compiler_version(self) -> str:
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
    return BuildInfo()
