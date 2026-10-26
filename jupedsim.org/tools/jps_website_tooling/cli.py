# SPDX-License-Identifier: LGPL-3.0-or-later
"""Command line frame of ``jps-website-tooling``.

Each subcommand lives in its own ``cmd_<name>`` module, a thin wrapper over
the library module ``<name>``, and exposes ``add_subcommand(subparsers)``: it
adds its subparser (or a whole group such as ``preview`` with ``publish`` and ``prune``
beneath it; one module owns one top-level name) and sets ``func``
to a handler ``(args) -> int``, and declares ``REQUIREMENTS`` (its extra
packages, shown in ``--help``). ``discover_subcommands()`` finds those
modules by their ``cmd_`` prefix; nothing is registered by hand. Subcommand
modules never import this module (they use ``jps_website_tooling.errors``,
``jps_website_tooling.out`` and the ``Subparsers`` protocol in
``jps_website_tooling.subcommand``). Tests pass probe builders to ``main`` and
``build_parser`` instead.

Handlers raise ``UsageError`` for bad input (exit 2) and ``ToolError`` for a
failed step (exit 1); ``main`` maps both to ``out.write_error`` (one GitHub
annotation inside Actions, readable text elsewhere). A group given without
its inner command is a usage error too.
"""

from __future__ import annotations

import argparse
import importlib
import pkgutil
import sys
from collections.abc import Iterable

import jps_website_tooling
from jps_website_tooling import __version__
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.out import write_error
from jps_website_tooling.subcommand import SubcommandBuilder

__all__ = ["ToolError", "UsageError", "main"]

PROG = "jps-website-tooling"
CMD_PREFIX = "cmd_"


def discover_subcommands() -> list[SubcommandBuilder]:
    """Find the subcommand modules of this package.

    Every module named ``cmd_<name>`` is imported and must expose
    ``add_subcommand``. Imports happen here, at call time, so a subcommand
    module may import ``errors`` and ``out`` without a cycle.

    Returns:
        The ``add_subcommand`` callables in module-name order, which is the
        help order.
    """
    builders: list[SubcommandBuilder] = []
    modules = sorted(
        pkgutil.iter_modules(jps_website_tooling.__path__), key=lambda m: m.name
    )
    for info in modules:
        if info.name.startswith(CMD_PREFIX):
            module = importlib.import_module(
                f"{jps_website_tooling.__name__}.{info.name}"
            )
            builders.append(module.add_subcommand)
    return builders


def build_parser(
    builders: Iterable[SubcommandBuilder] | None = None,
) -> argparse.ArgumentParser:
    """Build the top-level parser with its subcommands.

    Args:
        builders: The subcommand builders to add; the discovered ``cmd_*``
            modules when omitted (tests pass probes here).

    Returns:
        The parser; each subcommand sets ``func`` to its handler.
    """
    if builders is None:
        builders = discover_subcommands()
    parser = argparse.ArgumentParser(
        prog=PROG,
        description="Build, preview and publish jupedsim.org (www and docs).",
    )
    parser.add_argument(
        "--version", action="version", version=f"{PROG} {__version__}"
    )
    subparsers = parser.add_subparsers(dest="command")
    for builder in builders:
        builder(subparsers)
    return parser


def main(
    argv: list[str] | None = None,
    *,
    builders: Iterable[SubcommandBuilder] | None = None,
) -> int:
    """Run the command line interface.

    Args:
        argv: Arguments without the program name; ``sys.argv[1:]`` when
            omitted.
        builders: Subcommand builders to use instead of the discovered
            ``cmd_*`` modules (for tests).

    Returns:
        The exit code: 0 on success, 1 for a ``ToolError``, 2 for a
        ``UsageError``, a missing subcommand or an argparse error.
    """
    parser = build_parser(builders)
    try:
        args = parser.parse_args(argv)
    except SystemExit as exc:  # --version, --help, argparse errors
        return int(exc.code or 0)
    handler = getattr(args, "func", None)
    if handler is None:
        # No subcommand, or a group without its inner command.
        parser.print_usage(sys.stderr)
        return 2
    try:
        return int(handler(args))
    except UsageError as exc:
        write_error(str(exc))
        return 2
    except ToolError as exc:
        write_error(str(exc))
        return 1
