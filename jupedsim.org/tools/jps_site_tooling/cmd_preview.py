# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``preview`` subcommand group: a thin wrapper over ``previews``.

Usage:
``jps-site-tooling preview rebase TREE PREFIX [--stable NAME]`` moves a site
build under a URL prefix; ``jps-site-tooling preview prune ROOT --repo
OWNER/NAME`` deletes the previews of pull requests that are no longer open
(the token comes from ``GITHUB_TOKEN``).
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path

from jps_site_tooling import previews
from jps_site_tooling.errors import UsageError
from jps_site_tooling.out import write_info
from jps_site_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = ("PyGithub~=2.10",)


def _rebase(args: argparse.Namespace) -> int:
    """Handle ``preview rebase``.

    Args:
        args: Parsed arguments with ``tree``, ``prefix`` and ``stable``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    result = previews.rebase_links(Path(args.tree), args.prefix, args.stable)
    summary = (
        f"rebase: {result.links} links in {result.files} HTML files, "
        f"{result.urls} versions.json urls -> {args.prefix}"
    )
    if result.stable_linked:
        summary += f"; docs/stable -> {args.stable}"
    write_info(summary)
    return 0


def _prune(args: argparse.Namespace) -> int:
    """Handle ``preview prune``.

    Args:
        args: Parsed arguments with ``root`` and ``repo``.

    Returns:
        0 on success; errors propagate to ``cli.main``.

    Raises:
        UsageError: If ``GITHUB_TOKEN`` is not set or empty.
    """
    token = os.environ.get("GITHUB_TOKEN")
    if not token:
        raise UsageError("GITHUB_TOKEN is not set or empty")
    is_open = previews.github_pr_lookup(args.repo, token)
    previews.prune(Path(args.root), is_open)
    return 0


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``preview`` group with ``rebase`` and ``prune``.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    group = subparsers.add_parser(
        "preview",
        help="PR previews: rebase links under a prefix, prune old previews",
        description=previews.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    inner = group.add_subparsers(dest="preview_command")

    rebase = inner.add_parser(
        "rebase",
        help="move a site build under a URL prefix",
        epilog=requirements_epilog(()),
    )
    rebase.add_argument("tree", help="the site build (root with index.html)")
    rebase.add_argument("prefix", help="URL prefix, e.g. /pull-requests/42/")
    rebase.add_argument(
        "--stable",
        metavar="NAME",
        help="create docs/stable -> NAME when docs/stable is missing",
    )
    rebase.set_defaults(func=_rebase)

    prune = inner.add_parser(
        "prune",
        help="delete previews whose pull request is not open",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    prune.add_argument("root", help="the pull-requests directory")
    prune.add_argument(
        "--repo", required=True, help="GitHub repository as OWNER/NAME"
    )
    prune.set_defaults(func=_prune)
