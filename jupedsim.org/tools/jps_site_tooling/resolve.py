# SPDX-License-Identifier: LGPL-3.0-or-later
"""Decide which docs version a ``docs.yml`` run publishes, or refuse the run.

Documentation is published from a release line only: a branch ``rls-X.Y``
and tags ``vX.Y.Z`` on it. Two triggers reach Resolve:

- a **manual dispatch** on ``refs/heads/rls-X.Y``: the version is the
  highest release tag of that line reachable from HEAD (``git tag --list
  'vX.Y.*' --merged HEAD``, filtered to ``vX.Y.Z``); ``n`` counts the
  commits since that tag (a doc fix after the release). Listing instead of
  ``git describe`` makes pre-release tags on the same commit and merges of
  other lines irrelevant;
- a **published release** ``vX.Y.Z``: the release must be neither a draft
  nor a pre-release and its tagged commit must be an ancestor of
  ``origin/rls-X.Y``; ``n`` is 0.

Both refuse lines before 2.0: 1.x is closed and its docs are frozen in
``docs/v1x/``. Every refusal is a ``ToolError`` (the run is legitimate
but must not publish; it fails with one annotation).

Nothing here fetches. The checkout must already hold the history, the tags
and the remote-tracking branch (``actions/checkout`` with ``fetch-depth: 0``
in the workflow); missing refs are reported by name.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from jps_site_tooling.errors import ToolError
from jps_site_tooling.git import git

NUM = r"(0|[1-9][0-9]*)"
"""A decimal number without leading zeros (ASCII digits only)."""

RELEASE_TAG = re.compile(rf"v{NUM}\.{NUM}\.{NUM}")
"""A release tag ``vX.Y.Z`` (matched with ``fullmatch``)."""

RELEASE_BRANCH_REF = re.compile(rf"refs/heads/rls-{NUM}\.{NUM}")
"""A release branch ref ``refs/heads/rls-X.Y`` (matched with ``fullmatch``)."""

MIN_MAJOR = 2
"""Lowest major version whose docs are published; 1.x is frozen."""


@dataclass(frozen=True)
class Resolved:
    """What a run publishes.

    Attributes:
        version: The docs version directory name, ``vX.Y.Z``.
        n: Commits since the release tag (0 for a release).
        source_sha: The source commit to build and tag.
    """

    version: str
    n: int
    source_sha: str


def parse_release_tag(tag: str) -> tuple[int, int, int]:
    """Split a release tag into its numbers.

    Args:
        tag: A tag name.

    Returns:
        ``(major, minor, patch)``.

    Raises:
        ToolError: If ``tag`` is not of the form ``vX.Y.Z``.
    """
    m = RELEASE_TAG.fullmatch(tag)
    if m is None:
        raise ToolError(f"{tag!r} is not a release tag (vX.Y.Z)")
    return int(m.group(1)), int(m.group(2)), int(m.group(3))


def parse_release_branch(ref: str) -> tuple[int, int]:
    """Split a release branch ref into its line numbers.

    Args:
        ref: A full ref such as ``refs/heads/rls-2.0``.

    Returns:
        ``(major, minor)``.

    Raises:
        ToolError: If ``ref`` is not ``refs/heads/rls-X.Y``.
    """
    m = RELEASE_BRANCH_REF.fullmatch(ref)
    if m is None:
        raise ToolError(f"{ref} is not a release branch (refs/heads/rls-X.Y)")
    return int(m.group(1)), int(m.group(2))


def check_major(major: int, what: str) -> None:
    """Refuse versions of the closed 1.x line.

    Args:
        major: The major version.
        what: What is being refused, for the message (a tag or a ref).

    Raises:
        ToolError: If ``major`` is below ``MIN_MAJOR``.
    """
    if major < MIN_MAJOR:
        raise ToolError(
            f"refusing {what}: lines before 2.0 are closed (the 1.x docs are "
            "frozen in docs/v1x/)"
        )


def check_history(checkout: Path) -> None:
    """Make sure the checkout can answer history questions.

    Args:
        checkout: The source checkout.

    Raises:
        ToolError: If it is not a git checkout, or a shallow clone (the
            workflow must check out with ``fetch-depth: 0``).
    """
    inside = git(checkout, "rev-parse", "--is-inside-work-tree", check=False)
    if inside.returncode != 0 or inside.stdout.strip() != "true":
        raise ToolError(f"{checkout} is not a git checkout")
    shallow = git(checkout, "rev-parse", "--is-shallow-repository")
    if shallow.stdout.strip() == "true":
        raise ToolError(
            f"{checkout} is a shallow clone; tags and history are incomplete "
            "(check out with fetch-depth: 0)"
        )


def _highest_release_tag(checkout: Path, major: int, minor: int) -> str | None:
    """Find the highest ``vX.Y.Z`` tag of a line reachable from HEAD.

    Args:
        checkout: The source checkout.
        major: The line's major version.
        minor: The line's minor version.

    Returns:
        The tag name, or None when no release tag of the line is reachable.
        Tags that merely start with ``vX.Y.`` (pre-releases, odd names) are
        ignored.
    """
    listed = git(
        checkout, "tag", "--list", f"v{major}.{minor}.*", "--merged", "HEAD"
    ).stdout.split()
    releases: list[tuple[int, str]] = []
    for name in listed:
        m = RELEASE_TAG.fullmatch(name)
        if m and (int(m.group(1)), int(m.group(2))) == (major, minor):
            releases.append((int(m.group(3)), name))
    return max(releases)[1] if releases else None


def resolve_dispatch(checkout: Path, ref: str) -> Resolved:
    """Resolve a manual dispatch on a release branch.

    Args:
        checkout: The source checkout at the dispatched branch head.
        ref: ``github.ref`` of the dispatch.

    Returns:
        The line's highest reachable release tag as version, the commits
        since it, and HEAD.

    Raises:
        ToolError: If ``ref`` is not a release branch, the line is 1.x, the
            checkout is not a full clone, or no release tag of the line is
            reachable from HEAD.
    """
    major, minor = parse_release_branch(ref)
    check_major(major, ref)
    check_history(checkout)
    tag = _highest_release_tag(checkout, major, minor)
    if tag is None:
        raise ToolError(
            f"no release tag of line {major}.{minor} reachable from {ref}"
        )
    n = int(git(checkout, "rev-list", f"{tag}..HEAD", "--count").stdout)
    source_sha = git(checkout, "rev-parse", "HEAD").stdout.strip()
    return Resolved(tag, n, source_sha)


def resolve_release(
    checkout: Path,
    tag: str,
    *,
    draft: bool,
    prerelease: bool,
    remote: str = "origin",
) -> Resolved:
    """Resolve a published GitHub release.

    Args:
        checkout: The source checkout holding the tag and ``rls-X.Y``.
        tag: The release tag.
        draft: Whether GitHub marks the release as a draft.
        prerelease: Whether GitHub marks the release as a pre-release.
        remote: The remote whose ``rls-X.Y`` the tag must be on.

    Returns:
        The tag as version, ``n = 0`` and the tagged commit.

    Raises:
        ToolError: If the tag is not a release tag, the line is 1.x, the
            release is a draft or pre-release, the checkout is not a full
            clone, the tag or the branch is not in the checkout, or the tag
            is not on the branch.
    """
    major, minor, _ = parse_release_tag(tag)
    check_major(major, tag)
    if draft:
        raise ToolError(f"release {tag} is a draft")
    if prerelease:
        raise ToolError(f"release {tag} is a pre-release")
    check_history(checkout)
    commit = git(
        checkout,
        "rev-parse",
        "--verify",
        "-q",
        f"{tag}^{{commit}}",
        check=False,
    )
    if commit.returncode != 0:
        raise ToolError(f"tag {tag} is not in the checkout; fetch tags first")
    source_sha = commit.stdout.strip()
    branch = f"{remote}/rls-{major}.{minor}"
    if git(
        checkout, "rev-parse", "--verify", "-q", branch, check=False
    ).returncode:
        raise ToolError(f"branch {branch} is not in the checkout")
    on_branch = git(
        checkout, "merge-base", "--is-ancestor", source_sha, branch, check=False
    )
    if on_branch.returncode != 0:
        raise ToolError(f"{tag} ({source_sha}) is not on rls-{major}.{minor}")
    return Resolved(tag, 0, source_sha)
