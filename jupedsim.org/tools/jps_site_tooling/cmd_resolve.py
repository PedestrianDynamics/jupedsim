# SPDX-License-Identifier: LGPL-3.0-or-later
"""The ``resolve`` subcommand group: a thin wrapper over ``resolve``.

Usage:
``jps-site-tooling resolve dispatch --ref REF --checkout DIR`` resolves a
manual dispatch on a release branch; ``jps-site-tooling resolve release
--tag TAG --draft true|false --prerelease true|false --checkout DIR
[--remote origin]`` resolves a published release. Both write the outputs
``version``, ``n`` and ``source_sha``; a refusal exits 1.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from jps_site_tooling import resolve
from jps_site_tooling.checkout import check_checkout
from jps_site_tooling.out import write_output
from jps_site_tooling.subcommand import Subparsers, requirements_epilog

REQUIREMENTS: tuple[str, ...] = ()


def _report(result: resolve.Resolved) -> int:
    """Write the outputs of a resolution.

    Args:
        result: What the run publishes.

    Returns:
        0.
    """
    write_output("version", result.version)
    write_output("n", str(result.n))
    write_output("source_sha", result.source_sha)
    return 0


def _dispatch(args: argparse.Namespace) -> int:
    """Handle ``resolve dispatch``.

    Args:
        args: Parsed arguments with ``ref`` and ``checkout``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    checkout = check_checkout(Path(args.checkout))
    return _report(resolve.resolve_dispatch(checkout, args.ref))


def _release(args: argparse.Namespace) -> int:
    """Handle ``resolve release``.

    Args:
        args: Parsed arguments with ``tag``, ``draft``, ``prerelease``,
            ``checkout`` and ``remote``.

    Returns:
        0 on success; errors propagate to ``cli.main``.
    """
    checkout = check_checkout(Path(args.checkout))
    return _report(
        resolve.resolve_release(
            checkout,
            args.tag,
            draft=args.draft == "true",
            prerelease=args.prerelease == "true",
            remote=args.remote,
        )
    )


def add_subcommand(subparsers: Subparsers) -> None:
    """Register the ``resolve`` group with ``dispatch`` and ``release``.

    Args:
        subparsers: The top-level parser's subparsers action.
    """
    group = subparsers.add_parser(
        "resolve",
        help="decide which docs version a run publishes, or refuse it",
        description=resolve.__doc__,
        epilog=requirements_epilog(REQUIREMENTS),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    inner = group.add_subparsers(dest="resolve_command")
    checkout_help = "source checkout with full history and tags"

    dispatch = inner.add_parser(
        "dispatch",
        help="a manual dispatch on refs/heads/rls-X.Y",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    dispatch.add_argument(
        "--ref",
        required=True,
        help="github.ref of the dispatch (refs/heads/rls-X.Y)",
    )
    dispatch.add_argument("--checkout", required=True, help=checkout_help)
    dispatch.set_defaults(func=_dispatch)

    release = inner.add_parser(
        "release",
        help="a published GitHub release",
        epilog=requirements_epilog(REQUIREMENTS),
    )
    release.add_argument(
        "--tag", required=True, help="the release tag (vX.Y.Z)"
    )
    release.add_argument(
        "--draft",
        required=True,
        choices=("true", "false"),
        help="github.event.release.draft",
    )
    release.add_argument(
        "--prerelease",
        required=True,
        choices=("true", "false"),
        help="github.event.release.prerelease",
    )
    release.add_argument("--checkout", required=True, help=checkout_help)
    release.add_argument(
        "--remote",
        default="origin",
        help="remote whose rls-X.Y the tag must be on (origin)",
    )
    release.set_defaults(func=_release)
