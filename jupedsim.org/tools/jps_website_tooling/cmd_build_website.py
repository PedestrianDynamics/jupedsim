# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``build-website`` subcommand: a thin wrapper over ``website.build_website``.

Usage: ``jps-website-tooling build-website --out DIR [--version V]
[--no-notebooks] [--lenient] [--target {www,docs,all}] [--clean]``, run at the
repository root (the source is ``<cwd>/jupedsim.org``).
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_website_tooling import website
from jps_website_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = (
    "-r jupedsim.org/www/requirements.txt",
    "-r jupedsim.org/docs/requirements.txt",
)


def _run(args: argparse.Namespace) -> int:
    """Handle the ``build-website`` subcommand.

    Args:
        args: Parsed arguments.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    website.build_website(
        website.BuildOptions(
            source=(Path.cwd() / "jupedsim.org").resolve(),
            out=Path(args.out).resolve(),
            version=args.version,
            notebooks=not args.no_notebooks,
            strict=not args.lenient,
            target=args.target,
            clean=args.clean,
        )
    )
    return 0


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``build-website`` subcommand.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    parser = subparsers.add_parser(
        "build-website",
        help="assemble the website (www, docs, versions, search index)",
        description=website.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--out",
        required=True,
        help="output directory (the local Pages replica)",
    )
    parser.add_argument(
        "--version",
        default="",
        help="docs version directory name, e.g. v2.0.0 or dev "
        "(required unless --target www)",
    )
    parser.add_argument(
        "--no-notebooks",
        action="store_true",
        help="do not execute notebooks (nb_execution_mode=off)",
    )
    parser.add_argument(
        "--lenient",
        action="store_true",
        help="keep Sphinx warnings as warnings (the default fails on them)",
    )
    parser.add_argument(
        "--target",
        choices=["www", "docs", "all"],
        default="all",
        help="what to build: www, docs, or all (default: "
        "all; docs also writes versions.json, all also builds the index)",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="remove the output directory and <out>-cache before building",
    )
    parser.set_defaults(func=_run)
