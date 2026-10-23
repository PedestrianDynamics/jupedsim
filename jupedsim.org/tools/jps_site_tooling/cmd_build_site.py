# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``build-site`` subcommand: a thin wrapper over ``site.build_site``.

Usage: ``jps-site-tooling build-site --source DIR --out DIR [--version V]
[--no-notebooks] [--strict] [--only {site,docs,all}] [--clean]``.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_site_tooling import site
from jps_site_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = (
    "-r jupedsim.org/site/requirements.txt",
    "-r jupedsim.org/docs/requirements.txt",
)


def _run(args: argparse.Namespace) -> int:
    """Handle the ``build-site`` subcommand.

    Args:
        args: Parsed arguments.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    site.build_site(
        site.BuildOptions(
            source=Path(args.source).resolve(),
            out=Path(args.out).resolve(),
            version=args.version,
            notebooks=not args.no_notebooks,
            strict=args.strict,
            only=args.only,
            clean=args.clean,
        )
    )
    return 0


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``build-site`` subcommand.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    parser = subparsers.add_parser(
        "build-site",
        help="assemble the website (site, docs, versions, search index)",
        description=site.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--source",
        required=True,
        help="the jupedsim.org/ folder of the checkout (site/, docs/, theme/)",
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
        "(required unless --only site)",
    )
    parser.add_argument(
        "--no-notebooks",
        action="store_true",
        help="do not execute notebooks (nb_execution_mode=off)",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Sphinx warnings are errors (-W --keep-going); used by CI",
    )
    parser.add_argument(
        "--only",
        choices=["site", "docs", "all"],
        default="all",
        help="build only the site, only the docs, or everything (default: "
        "all; docs also writes versions.json, all also builds the index)",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="remove the output directory and <out>-cache before building",
    )
    parser.set_defaults(func=_run)
