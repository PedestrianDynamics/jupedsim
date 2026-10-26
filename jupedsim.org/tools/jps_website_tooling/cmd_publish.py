# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``publish`` subcommand group: a thin wrapper over ``publish``.

Usage:
``jps-website-tooling publish docs --build DIR --version vX.Y.Z --checkout DIR
--source-sha SHA`` publishes
``docs/<version>/``; ``jps-website-tooling publish www --build DIR --checkout
DIR --source-sha SHA`` publishes the www
pages at the root. Both write the outputs ``pages_sha`` and ``changed`` (``true`` or
``false``).
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_website_tooling import publish
from jps_website_tooling.out import write_output
from jps_website_tooling.subcommand import Subparsers, requirements_epilog

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
            ``source_sha``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    return _report(
        publish.publish_docs(
            Path(args.build),
            args.version,
            Path(args.checkout),
            args.source_sha,
        )
    )


def _www(args: argparse.Namespace) -> int:
    """Handle ``publish www``.

    Args:
        args: Parsed arguments with ``build``, ``checkout`` and
            ``source_sha``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    return _report(
        publish.publish_www(
            Path(args.build),
            Path(args.checkout),
            args.source_sha,
        )
    )


def _common(parser: argparse.ArgumentParser) -> None:
    """Add the arguments both publishers share.

    Args:
        parser: The ``docs`` or ``www`` parser.
    """
    parser.add_argument("--build", required=True, help="build-website output")
    parser.add_argument(
        "--checkout",
        required=True,
        help="clean clone of the Pages branch (checked out), next to the build",
    )
    parser.add_argument(
        "--source-sha", required=True, help="the built source commit"
    )


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``publish`` group with ``docs`` and ``www``.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    group = subparsers.add_parser(
        "publish",
        help="publish a docs version or the www pages to the Pages branch",
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

    www = inner.add_parser(
        "www",
        help="publish the www project (everything outside docs/)",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    _common(www)
    www.set_defaults(func=_www)
