# SPDX-License-Identifier: LGPL-3.0-or-later
"""Exceptions shared by the library functions and the command line frame.

Library modules raise these; ``cli.main`` maps them to exit codes. They live
here, not in ``cli``, so a library module never imports the CLI (it uses
``errors`` and ``out`` only).
"""


class ToolError(Exception):
    """A step failed after work may have started.

    The message is shown to the user; the CLI exits with 1.
    """


class UsageError(ToolError):
    """Bad arguments or unusable input, detected before anything was touched.

    The message is shown to the user; the CLI exits with 2.
    """
