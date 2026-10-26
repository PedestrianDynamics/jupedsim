# SPDX-License-Identifier: LGPL-3.0-or-later
"""Run git for the publishers, and guard the checkout they reset and clean.

Every call carries its own configuration (``GIT_CONFIG``): the bot identity
for commits and tags (CI runners have no identity, and a maintainer running
the tooling locally must not have their own identity used or changed), no
signing, and symlinks checked out as such (``docs/stable``).

The publishers run ``git reset --hard`` and ``git clean -ffdx`` in the Pages
checkout on every attempt. ``check_pages_checkout`` makes sure that
checkout really is a clean clone of the branch to publish to before
anything is reset: the branch is the one checked out (a detached HEAD is
refused), its HEAD must be inside the remote branch's history, and nothing
may be uncommitted, untracked or ignored.
"""

from __future__ import annotations

import subprocess
from pathlib import Path

from jps_website_tooling.errors import ToolError

GIT_CONFIG: tuple[str, ...] = (
    "-c", "user.name=github-actions[bot]",
    "-c", "user.email=github-actions[bot]@users.noreply.github.com",
    "-c", "commit.gpgsign=false",
    "-c", "tag.gpgsign=false",
    "-c", "core.symlinks=true",
)  # fmt: skip

REMOTE = "origin"
"""The only remote the tooling talks to."""

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


def check_pages_checkout(checkout: Path) -> str:
    """Refuse anything that is not a clean checkout of a published branch.

    The branch to publish to is the one checked out; nothing else names it.
    The source repository is recognised by its layout (``jupedsim.org/tools``
    at the root) and refused whatever branch it is on: it is never a
    publishing target, and the loop would wipe it.

    Args:
        checkout: The directory the publisher is about to reset and clean.

    Returns:
        The checked-out branch name.

    Raises:
        ToolError: If ``checkout`` is not a git checkout, is the source
            repository, HEAD is detached, the branch cannot be fetched from
            ``origin``, HEAD is not in the remote branch's history, or the
            tree has uncommitted, untracked or ignored files.
    """
    if not (checkout / ".git").exists():
        raise ToolError(f"{checkout} is not a git checkout")
    if (checkout / "jupedsim.org" / "tools" / "pyproject.toml").is_file():
        raise ToolError(
            f"{checkout} is the source repository (jupedsim.org/tools/ at its "
            "root), not a Pages or previews checkout; refusing to reset and "
            "clean it"
        )
    head_ref = git(checkout, "symbolic-ref", "-q", "HEAD", check=False)
    if head_ref.returncode != 0:
        sha = git(
            checkout, "rev-parse", "--short", "HEAD", check=False
        ).stdout.strip()
        raise ToolError(
            f"{checkout} has a detached HEAD ({sha or 'no commit'}); check out "
            "the branch to publish to"
        )
    branch = head_ref.stdout.strip().removeprefix("refs/heads/")
    git(checkout, "fetch", REMOTE, REFSPEC.format(remote=REMOTE, branch=branch))
    ancestor = git(
        checkout,
        "merge-base",
        "--is-ancestor",
        "HEAD",
        f"{REMOTE}/{branch}",
        check=False,
    )
    if ancestor.returncode != 0:
        raise ToolError(
            f"{checkout}: {branch} is not in the history of {REMOTE}/{branch} "
            "(unpushed or rewritten commits); refusing to reset and clean it."
        )
    status = git(checkout, "status", "--porcelain", "--ignored").stdout
    if status.strip():
        raise ToolError(
            f"{checkout} has uncommitted, untracked or ignored files; "
            "refusing to reset and clean it. Commit, move or remove them "
            f"first:\n{status.rstrip()}"
        )
    return branch
