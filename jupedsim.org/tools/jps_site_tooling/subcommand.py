# SPDX-License-Identifier: LGPL-3.0-or-later
"""The interface between the CLI frame and the ``cmd_*`` modules.

Every ``cmd_*`` module exposes ``add_subcommand(subparsers)`` and a tuple
``REQUIREMENTS`` of the distributions it needs beyond the standard library
(the same specs as in ``pyproject.toml``); ``requirements_epilog`` turns
that into the ``--help`` footer, so a user of an uninstalled checkout sees
what to ``pip install``.
"""

from __future__ import annotations

import argparse
from collections.abc import Callable, Sequence
from typing import Protocol


class Subparsers(Protocol):
    """What a subcommand builder needs from argparse's subparsers action.

    Structurally satisfied by the object ``ArgumentParser.add_subparsers()``
    returns, without naming that private class. Only the keyword arguments
    the ``cmd_*`` modules use are listed; add more here when needed.
    """

    def add_parser(
        self,
        name: str,
        *,
        help: str | None = ...,
        description: str | None = ...,
        epilog: str | None = ...,
        formatter_class: type[argparse.HelpFormatter] = ...,
    ) -> argparse.ArgumentParser:
        """Add a subparser.

        Args:
            name: The subcommand name on the command line.
            help: One line for the parent's help listing.
            description: Text for the subcommand's own ``--help``.
            epilog: Text after the options in ``--help``.
            formatter_class: Help formatter of the subparser.

        Returns:
            The new subparser.
        """
        ...


def requirements_epilog(requirements: Sequence[str]) -> str:
    """Build the ``--help`` footer naming a subcommand's extra packages.

    Args:
        requirements: Distribution specs beyond the standard library, e.g.
            ``("pagefind[bin]~=1.5",)``, or requirements files of the
            checkout as ``"-r <path>"``; empty for none.

    Returns:
        One line, ``Additional packages: <specs>`` with a ``pip install``
        hint, or ``Additional packages: none``.
    """
    if not requirements:
        return "Additional packages: none (standard library only)."
    files = [r.removeprefix("-r ") for r in requirements if r.startswith("-r ")]
    specs = [r for r in requirements if not r.startswith("-r ")]
    shown = ", ".join(specs + ([f"see {', '.join(files)}"] if files else []))
    install = " ".join([f"'{s}'" for s in specs] + [f"-r {f}" for f in files])
    return f"Additional packages: {shown} (python -m pip install {install})."


SubcommandBuilder = Callable[[Subparsers], None]
"""A ``cmd_*`` module's ``add_subcommand``: adds its subparser(s) and sets
``func`` to a handler ``(args) -> int``."""
