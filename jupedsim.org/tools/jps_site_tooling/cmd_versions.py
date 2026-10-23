# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``versions`` subcommand: a thin wrapper over ``versions``.

Usage: ``jps-site-tooling versions DOCS [--dev VERSION]``. Prints nothing on
success; errors are reported through ``out.write_error`` by ``cli.main``.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_site_tooling import versions
from jps_site_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = ("packaging",)


def _run(args: argparse.Namespace) -> int:
    """Handle the ``versions`` subcommand.

    Args:
        args: Parsed arguments with ``docs`` and ``dev``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    docs = Path(args.docs)
    if args.dev is not None:
        versions.write_dev_versions(docs, args.dev)
    else:
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
    parser.add_argument(
        "--dev",
        metavar="VERSION",
        help="development build: list only VERSION, create no stable link",
    )
    parser.set_defaults(func=_run)
