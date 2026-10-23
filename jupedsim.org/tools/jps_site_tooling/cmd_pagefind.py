# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``pagefind`` subcommand: a thin wrapper over ``pagefind``.

Usage: ``jps-site-tooling pagefind ROOT`` rebuilds ``ROOT/pagefind`` and
reports the number of indexed pages.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_site_tooling import pagefind
from jps_site_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = ("pagefind[bin]~=1.5",)


def _run(args: argparse.Namespace) -> int:
    """Handle the ``pagefind`` subcommand.

    Args:
        args: Parsed arguments with ``root``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    pagefind.build_index(Path(args.root))
    return 0


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``pagefind`` subcommand.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    parser = subparsers.add_parser(
        "pagefind",
        help="rebuild the Pagefind search index of a Pages tree",
        description=pagefind.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("root", help="the Pages tree (site root with docs/)")
    parser.set_defaults(func=_run)
