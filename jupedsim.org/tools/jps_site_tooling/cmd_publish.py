# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``publish`` subcommand group: a thin wrapper over ``publish``.

Usage:
``jps-site-tooling publish docs --build DIR --version vX.Y.Z --checkout DIR
--branch NAME --source-sha SHA [--remote origin]`` publishes
``docs/<version>/``; ``jps-site-tooling publish site --build DIR --checkout
DIR --branch NAME --source-sha SHA [--remote origin]`` publishes the site
root. Both write the outputs ``pages_sha`` and ``changed`` (``true`` or
``false``).
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_site_tooling import publish
from jps_site_tooling.out import write_output
from jps_site_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS = ("pagefind[bin]~=1.5",)


def _report(result: publish.PublishResult) -> int:
    """Write the outputs of a publish.

    Args:
        result: What the publish did.

    Returns:
        0.
    """
    write_output("pages_sha", result.pages_sha)
    write_output("changed", "true" if result.changed else "false")
    return 0


def _docs(args: argparse.Namespace) -> int:
    """Handle ``publish docs``.

    Args:
        args: Parsed arguments with ``build``, ``version``, ``checkout``,
            ``branch``, ``source_sha`` and ``remote``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    return _report(
        publish.publish_docs(
            Path(args.build),
            args.version,
            Path(args.checkout),
            args.remote,
            args.branch,
            args.source_sha,
        )
    )


def _site(args: argparse.Namespace) -> int:
    """Handle ``publish site``.

    Args:
        args: Parsed arguments with ``build``, ``checkout``, ``branch``,
            ``source_sha`` and ``remote``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    return _report(
        publish.publish_site(
            Path(args.build),
            Path(args.checkout),
            args.remote,
            args.branch,
            args.source_sha,
        )
    )


def _common(parser: argparse.ArgumentParser) -> None:
    """Add the arguments both publishers share.

    Args:
        parser: The ``docs`` or ``site`` parser.
    """
    parser.add_argument("--build", required=True, help="build-site output")
    parser.add_argument(
        "--checkout",
        required=True,
        help="clean clone of the Pages branch, next to the build",
    )
    parser.add_argument("--branch", required=True, help="the Pages branch")
    parser.add_argument(
        "--source-sha", required=True, help="the built source commit"
    )
    parser.add_argument(
        "--remote", default="origin", help="remote to push to (origin)"
    )


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``publish`` group with ``docs`` and ``site``.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    group = subparsers.add_parser(
        "publish",
        help="publish a docs version or the site to the Pages branch",
        description=publish.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    inner = group.add_subparsers(dest="publish_command")

    docs = inner.add_parser(
        "docs",
        help="publish docs/<version>/",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    docs.add_argument("--version", required=True, help="vX.Y.Z")
    _common(docs)
    docs.set_defaults(func=_docs)

    site = inner.add_parser(
        "site",
        help="publish the site root",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    _common(site)
    site.set_defaults(func=_site)
