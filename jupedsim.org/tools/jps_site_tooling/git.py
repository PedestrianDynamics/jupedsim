# SPDX-License-Identifier: LGPL-3.0-or-later
"""Run git for the publishers, and guard the checkout they reset and clean.

Every call carries its own configuration (``GIT_CONFIG``): the bot identity
for commits and tags (CI runners have no identity, and a maintainer running
the tooling locally must not have their own identity used or changed), no
signing, and symlinks checked out as such (``docs/stable``).

The publishers run ``git reset --hard`` and ``git clean -ffdx`` in the Pages
checkout on every attempt. ``check_pages_checkout`` makes sure that
checkout really is a clean clone of the Pages branch before anything is
reset: the Pages branch lives in the source repository, so a *source*
checkout resolves ``<remote>/<branch>`` too, and only a HEAD inside the
Pages branch's own (orphan) history passes, with nothing uncommitted,
untracked or ignored.
"""

from __future__ import annotations

import subprocess
from pathlib import Path

from jps_site_tooling.errors import ToolError

GIT_CONFIG: tuple[str, ...] = (
    "-c", "user.name=github-actions[bot]",
    "-c", "user.email=github-actions[bot]@users.noreply.github.com",
    "-c", "commit.gpgsign=false",
    "-c", "tag.gpgsign=false",
    "-c", "core.symlinks=true",
)  # fmt: skip

REFSPEC = "+refs/heads/{branch}:refs/remotes/{remote}/{branch}"
"""Fetch refspec: the remote branch into its remote-tracking ref, forced."""


def git(
    checkout: Path, *args: str, check: bool = True
) -> subprocess.CompletedProcess[str]:
    """Run one git command in ``checkout`` with the publisher configuration.

    Args:
        checkout: The working tree (or bare repository) to run in.
        *args: The git subcommand and its arguments.
        check: Raise when git exits non-zero.

    Returns:
        The completed process with captured, decoded stdout and stderr.

    Raises:
        ToolError: If ``check`` is set and git exits non-zero; the message
            carries the command and git's stderr.
    """
    result = subprocess.run(
        ["git", "-C", str(checkout), *GIT_CONFIG, *args],
        capture_output=True,
        text=True,
    )
    if check and result.returncode != 0:
        raise ToolError(
            f"git {' '.join(args)} failed ({result.returncode}):\n"
            f"{result.stderr.strip()}"
        )
    return result


def check_pages_checkout(checkout: Path, remote: str, branch: str) -> None:
    """Refuse anything that is not a clean checkout of the Pages branch.

    Args:
        checkout: The directory the publisher is about to reset and clean.
        remote: The remote holding the Pages branch, e.g. ``origin``.
        branch: The Pages branch name.

    Raises:
        ToolError: If ``checkout`` is not a git checkout, the branch cannot
            be fetched, HEAD is not in the branch's history, or the tree has
            uncommitted, untracked or ignored files.
    """
    if not (checkout / ".git").exists():
        raise ToolError(f"{checkout} is not a git checkout")
    git(checkout, "fetch", remote, REFSPEC.format(remote=remote, branch=branch))
    head = git(checkout, "rev-parse", "--verify", "-q", "HEAD", check=False)
    ancestor = git(
        checkout,
        "merge-base",
        "--is-ancestor",
        "HEAD",
        f"{remote}/{branch}",
        check=False,
    )
    if head.returncode != 0 or ancestor.returncode != 0:
        raise ToolError(
            f"{checkout} is not a checkout of {remote}/{branch} (HEAD is not "
            "in that branch's history); refusing to reset and clean it. "
            f"Check out {branch} there first."
        )
    status = git(checkout, "status", "--porcelain", "--ignored").stdout
    if status.strip():
        raise ToolError(
            f"{checkout} has uncommitted, untracked or ignored files; "
            "refusing to reset and clean it. Commit, move or remove them "
            f"first:\n{status.rstrip()}"
        )
