# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``tag`` subcommand group: a thin wrapper over ``tag``.

Usage:
``jps-website-tooling tag docs --version vX.Y.Z [--commits-since N] --checkout
DIR --source-sha SHA --pages-sha SHA`` and ``jps-website-tooling tag www
--checkout DIR --source-sha SHA --pages-sha SHA`` tag the source commit of a
finished publish and write the output ``tag``. ``--commits-since`` names the
commits after the release tag; omitted, the publish is the release itself.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_website_tooling import tag
from jps_website_tooling.errors import UsageError
from jps_website_tooling.out import write_output
from jps_website_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS: tuple[str, ...] = ()


def _docs(args: argparse.Namespace) -> int:
    """Handle ``tag docs``.

    Args:
        args: Parsed arguments with ``version``, ``commits_since``,
            ``checkout``, ``source_sha`` and ``pages_sha``.

    Returns:
        0 on success; errors propagate to ``cli.main``.

    Raises:
        UsageError: If ``--commits-since`` is given but below 1.
    """
    commits_since = 0
    if args.commits_since is not None:
        if args.commits_since < 1:
            raise UsageError(
                "--commits-since must be >= 1; omit it for the release itself"
            )
        commits_since = args.commits_since
    name = tag.tag_docs(
        Path(args.checkout),
        args.source_sha,
        args.pages_sha,
        args.version,
        commits_since=commits_since,
    )
    write_output("tag", name)
    return 0


def _www(args: argparse.Namespace) -> int:
    """Handle ``tag www``.

    Args:
        args: Parsed arguments with ``checkout``, ``source_sha`` and
            ``pages_sha``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    name = tag.tag_www(Path(args.checkout), args.source_sha, args.pages_sha)
    write_output("tag", name)
    return 0


def _common(parser: argparse.ArgumentParser) -> None:
    """Add the arguments both tag commands share.

    Args:
        parser: The ``docs`` or ``www`` parser.
    """
    parser.add_argument(
        "--checkout",
        required=True,
        help="clone of the source repository holding the built commit",
    )
    parser.add_argument(
        "--source-sha", required=True, help="the built source commit"
    )
    parser.add_argument(
        "--pages-sha", required=True, help="the Pages commit written by publish"
    )


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``tag`` group with ``docs`` and ``www``.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    group = subparsers.add_parser(
        "tag",
        help="tag the source commit of a publish",
        description=tag.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    inner = group.add_subparsers(dest="tag_command")

    docs = inner.add_parser(
        "docs",
        help="tag a docs publish: docs-<version>-<commits-since>-<TS>",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    docs.add_argument("--version", required=True, help="vX.Y.Z")
    docs.add_argument(
        "--commits-since",
        type=int,
        metavar="N",
        help="commits after the release tag (>= 1); omit for the release",
    )
    _common(docs)
    docs.set_defaults(func=_docs)

    www = inner.add_parser(
        "www",
        help="tag a www publish: www-<TS>",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    _common(www)
    www.set_defaults(func=_www)
