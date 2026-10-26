# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``preview`` subcommand group: a thin wrapper over ``previews``.

Usage:
``jps-website-tooling preview publish --build DIR --number N --checkout DIR``
publishes a build as the preview of pull request N (outputs ``pages_sha``,
``changed``); ``jps-website-tooling preview prune --checkout DIR`` deletes
the previews of pull requests that are no longer open and pushes (outputs
``pages_sha``, ``changed`` and ``deleted``, the numbers removed; the
repository and the token come from ``GITHUB_REPOSITORY`` and
``GITHUB_TOKEN``). The checkout is a clone of the previews repository with
``main`` checked out.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path

from jps_website_tooling import previews
from jps_website_tooling.errors import UsageError
from jps_website_tooling.out import write_info, write_output
from jps_website_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = ("PyGithub~=2.10",)


def _publish(args: argparse.Namespace) -> int:
    """Handle ``preview publish``.

    Args:
        args: Parsed arguments with ``build``, ``number`` and ``checkout``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    result = previews.publish_preview(
        Path(args.build),
        args.number,
        Path(args.checkout),
    )
    write_output("pages_sha", result.pages_sha)
    write_output("changed", "true" if result.changed else "false")
    return 0


def _prune(args: argparse.Namespace) -> int:
    """Handle ``preview prune``.

    Args:
        args: Parsed arguments with ``checkout``.

    Returns:
        0 on success; errors propagate to ``cli.main``.

    Raises:
        UsageError: If ``GITHUB_TOKEN`` or ``GITHUB_REPOSITORY`` is not set
            or empty.
    """
    token = os.environ.get("GITHUB_TOKEN")
    if not token:
        raise UsageError("GITHUB_TOKEN is not set or empty")
    repo = os.environ.get("GITHUB_REPOSITORY")
    if not repo:
        raise UsageError("GITHUB_REPOSITORY is not set or empty")
    is_open = previews.github_pr_lookup(repo, token)
    result, deleted = previews.prune_previews(Path(args.checkout), is_open)
    if deleted:
        write_info(
            "deleted orphaned previews: " + ", ".join(f"#{n}" for n in deleted)
        )
    else:
        write_info("no orphaned previews to delete")
    write_output("pages_sha", result.pages_sha)
    write_output("changed", "true" if result.changed else "false")
    write_output("deleted", ",".join(str(n) for n in deleted))
    return 0


def _repo_args(parser: argparse.ArgumentParser) -> None:
    """Add the previews-repository arguments.

    Args:
        parser: The ``publish`` or ``prune`` parser.
    """
    parser.add_argument(
        "--checkout",
        required=True,
        help="clean clone of the previews repository (main checked out)",
    )


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``preview`` group with ``publish`` and ``prune``.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    group = subparsers.add_parser(
        "preview",
        help="PR previews: publish a preview, prune old ones",
        description=previews.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    inner = group.add_subparsers(dest="preview_command")

    publish = inner.add_parser(
        "publish",
        help="publish a website build as the preview of a pull request",
        epilog=requirements_epilog(()),
    )
    publish.add_argument("--build", required=True, help="the website build")
    publish.add_argument(
        "--number", required=True, type=int, help="the pull request number"
    )
    _repo_args(publish)
    publish.set_defaults(func=_publish)

    prune = inner.add_parser(
        "prune",
        help="delete the previews of closed pull requests and push",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    _repo_args(prune)
    prune.set_defaults(func=_prune)
