# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``versions`` subcommand: a thin wrapper over ``versions``.

Usage: ``jps-website-tooling versions DOCS``. Prints nothing on
success; errors are reported through ``out.write_error`` by ``cli.main``.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_website_tooling import versions
from jps_website_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = ("packaging",)


def _run(args: argparse.Namespace) -> int:
    """Handle the ``versions`` subcommand.

    Args:
        args: Parsed arguments with ``docs``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    docs = Path(args.docs)
    versions.write_versions(docs)
    return 0


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``versions`` subcommand.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    parser = subparsers.add_parser(
        "versions",
        help="rewrite docs/versions.json and the docs/stable symlink",
        description=versions.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("docs", help="the docs/ directory")
    parser.set_defaults(func=_run)
