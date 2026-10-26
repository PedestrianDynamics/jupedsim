# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``resolve`` subcommand: a thin wrapper over ``resolve``.

Usage: ``jps-website-tooling resolve --checkout DIR`` decides from the
checkout alone which docs version the run publishes (on a release branch:
the line's latest release plus the commits since; detached at a release
tag: that release) and writes the outputs ``version``, ``commits_since``
and ``source_sha``; a refusal exits 1.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_website_tooling import resolve
from jps_website_tooling.checkout import check_checkout
from jps_website_tooling.out import write_output
from jps_website_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS: tuple[str, ...] = ()


def _resolve(args: argparse.Namespace) -> int:
    """Handle ``resolve``.

    Args:
        args: Parsed arguments with ``checkout``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    result = resolve.resolve(check_checkout(Path(args.checkout)))
    write_output("version", result.version)
    write_output("commits_since", str(result.commits_since))
    write_output("source_sha", result.source_sha)
    return 0


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``resolve`` subcommand.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    parser = subparsers.add_parser(
        "resolve",
        help="decide which docs version a run publishes, or refuse it",
        description=resolve.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--checkout",
        required=True,
        help="source checkout with full history and tags, on rls-X.Y or "
        "detached at a vX.Y.Z tag",
    )
    parser.set_defaults(func=_resolve)
