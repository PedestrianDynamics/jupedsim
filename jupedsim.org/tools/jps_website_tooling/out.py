# SPDX-License-Identifier: LGPL-3.0-or-later
"""All user-facing output of the tooling goes through these writers.

Inside a GitHub Actions job (``GITHUB_ACTIONS=true``) a message becomes one
workflow command (``::error::``, ``::warning::``, ``::notice::``), which the
runner turns into an annotation; the message is escaped as the runner
expects (``%`` -> ``%25``, CR -> ``%0D``, LF -> ``%0A``), so a multi-line
message stays one annotation. Anywhere else the message is printed readably:
``error: ...`` / ``warning: ...`` with real line breaks, info as is.

Everything is written to stdout: that is the stream the runner scans for
workflow commands.

Results a workflow needs in later steps (``pages_sha``, ``tag``, ...) go
through ``write_output``: a ``key=value`` line on stdout, also appended to
the file named by ``GITHUB_OUTPUT`` when that variable is set.
"""

from __future__ import annotations

import os
import sys
from functools import cache
from typing import TextIO

from jps_website_tooling.errors import ToolError


@cache
def running_in_github_action() -> bool:
    """Whether this process runs inside a GitHub Actions job.

    Returns:
        True if ``GITHUB_ACTIONS`` is exactly ``"true"``. Cached for the
        lifetime of the process.
    """
    return os.environ.get("GITHUB_ACTIONS") == "true"


def escape_annotation(message: str) -> str:
    """Escape a message for use as the data of a workflow command.

    Args:
        message: Free text, possibly spanning several lines.

    Returns:
        The message with ``%``, CR and LF replaced by ``%25``, ``%0D`` and
        ``%0A`` (in that order), as the Actions runner expects.
    """
    return message.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")


def _write(
    command: str, prefix: str, message: str, file: TextIO | None
) -> None:
    """Write one message as a workflow command or as plain text.

    Args:
        command: Workflow command name used inside Actions (``error``,
            ``warning``, ``notice``).
        prefix: Text put before the message outside Actions.
        message: The message; may span several lines.
        file: Stream to write to; stdout when None.
    """
    stream = file if file is not None else sys.stdout
    if running_in_github_action():
        stream.write(f"::{command}::{escape_annotation(message)}\n")
    else:
        stream.write(f"{prefix}{message}\n")
    stream.flush()


def write_error(message: str, *, file: TextIO | None = None) -> None:
    """Report an error.

    Args:
        message: What went wrong; may span several lines.
        file: Stream to write to; stdout when omitted.
    """
    _write("error", "error: ", message, file)


def write_warning(message: str, *, file: TextIO | None = None) -> None:
    """Report a warning.

    Args:
        message: What to warn about; may span several lines.
        file: Stream to write to; stdout when omitted.
    """
    _write("warning", "warning: ", message, file)


def write_info(message: str, *, file: TextIO | None = None) -> None:
    """Report progress or a result worth seeing in the log.

    Args:
        message: The information; may span several lines.
        file: Stream to write to; stdout when omitted.
    """
    _write("notice", "", message, file)


def write_output(name: str, value: str) -> None:
    """Publish a result for later workflow steps.

    Args:
        name: Output name, e.g. ``pages_sha``.
        value: Its value, a single line.

    Raises:
        ValueError: If ``value`` contains a line break (it would inject a
            second output).
        ToolError: If ``GITHUB_OUTPUT`` is set but cannot be appended to.
    """
    if "\n" in value or "\r" in value:
        raise ValueError(f"output {name} contains a line break: {value!r}")
    line = f"{name}={value}\n"
    sys.stdout.write(line)
    sys.stdout.flush()
    target = os.environ.get("GITHUB_OUTPUT")
    if target:
        try:
            with open(target, "a", encoding="utf-8") as f:
                f.write(line)
        except OSError as exc:
            raise ToolError(
                f"cannot append output {name} to GITHUB_OUTPUT ({target}): {exc}"
            ) from exc
