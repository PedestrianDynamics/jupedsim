# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``tag`` subcommand: a thin wrapper over ``tag``.

Usage:
``jps-site-tooling tag --kind docs|site --checkout DIR --branch NAME
--source-sha SHA --pages-sha SHA [--version vX.Y.Z --count N]
[--remote origin]`` tags the source commit of a finished publish and writes
the output ``tag``.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_site_tooling import tag
from jps_site_tooling.out import write_output
from jps_site_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS: tuple[str, ...] = ()


def _tag(args: argparse.Namespace) -> int:
    """Handle ``tag``.

    Args:
        args: Parsed arguments with ``kind``, ``checkout``, ``branch``,
            ``source_sha``, ``pages_sha``, ``version``, ``count`` and
            ``remote``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    name = tag.tag_publish(
        args.kind,
        Path(args.checkout),
        args.remote,
        args.branch,
        args.source_sha,
        args.pages_sha,
        version=args.version,
        count=args.count,
    )
    write_output("tag", name)
    return 0


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``tag`` subcommand.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    parser = subparsers.add_parser(
        "tag",
        help="tag the source commit of a publish",
        description=tag.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--kind",
        required=True,
        choices=("docs", "site"),
        help="what was published",
    )
    parser.add_argument(
        "--checkout",
        required=True,
        help="clone of the source repository holding the built commit",
    )
    parser.add_argument("--branch", required=True, help="the Pages branch")
    parser.add_argument(
        "--source-sha", required=True, help="the built source commit"
    )
    parser.add_argument(
        "--pages-sha", required=True, help="the Pages commit written by publish"
    )
    parser.add_argument("--version", help="vX.Y.Z (docs)")
    parser.add_argument(
        "--count", type=int, help="commits since the release tag (docs)"
    )
    parser.add_argument(
        "--remote", default="origin", help="remote to push to (origin)"
    )
    parser.set_defaults(func=_tag)
