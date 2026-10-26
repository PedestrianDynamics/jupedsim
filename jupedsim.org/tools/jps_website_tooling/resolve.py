# SPDX-License-Identifier: LGPL-3.0-or-later
"""Decide which docs version a ``docs.yml`` run publishes, or refuse the run.

Documentation is published from a release line only: a branch ``rls-X.Y``
and tags ``vX.Y.Z`` on it. The checkout says which trigger it was:

- **on a branch** (a manual dispatch; actions/checkout creates the local
  branch for a branch ref): it must be ``rls-X.Y``; the version is the
  line's highest release tag reachable from HEAD (``git tag --list 'vX.Y.*'
  --merged HEAD``, filtered to ``vX.Y.Z``); ``commits_since`` counts the
  commits since that tag (a doc fix after the release);
- **detached HEAD** (a release; actions/checkout leaves a tag ref detached):
  exactly one ``vX.Y.Z`` tag must point at HEAD, and its commit must be an
  ancestor of ``origin/rls-X.Y``; ``commits_since`` is 0. A detached HEAD
  without a release tag, or with several, is refused.

Both refuse lines before 2.0: 1.x is closed and its docs are frozen in
``docs/v1x/``. Drafts and pre-releases never reach this code: the workflow
triggers on ``release: [released]`` only. Every refusal is a ``ToolError``
(the run is legitimate but must not publish; it fails with one annotation).

Nothing here fetches. The checkout must already hold the history, the tags
and the remote-tracking branch (``actions/checkout`` with ``fetch-depth: 0``
in the workflow); missing refs are reported by name.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from jps_website_tooling.errors import ToolError
from jps_website_tooling.git import REMOTE, git

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
        commits_since: Commits since the release tag (0 for a release).
        source_sha: The source commit to build and tag.
    """

    version: str
    commits_since: int
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


def _resolve_branch(checkout: Path, ref: str) -> Resolved:
    """Resolve a checkout that is on a branch (a manual dispatch).

    Args:
        checkout: The source checkout.
        ref: The symbolic ref of HEAD, ``refs/heads/<branch>``.

    Returns:
        The line's highest reachable release tag as version, the commits
        since it, and HEAD.

    Raises:
        ToolError: If the branch is not ``rls-X.Y``, the line is 1.x, or no
            release tag of the line is reachable from HEAD.
    """
    major, minor = parse_release_branch(ref)
    check_major(major, ref)
    tag = _highest_release_tag(checkout, major, minor)
    if tag is None:
        raise ToolError(
            f"no release tag of line {major}.{minor} reachable from {ref}"
        )
    commits_since = int(
        git(checkout, "rev-list", f"{tag}..HEAD", "--count").stdout
    )
    source_sha = git(checkout, "rev-parse", "HEAD").stdout.strip()
    return Resolved(tag, commits_since, source_sha)


def _release_tags_at_head(checkout: Path) -> list[str]:
    """List the release tags pointing at HEAD.

    Args:
        checkout: The source checkout.

    Returns:
        The ``vX.Y.Z`` tags at HEAD, sorted; pre-release tags are left out.
    """
    listed = git(checkout, "tag", "--points-at", "HEAD").stdout.split()
    return sorted(name for name in listed if RELEASE_TAG.fullmatch(name))


def _resolve_tag(checkout: Path, tag: str) -> Resolved:
    """Resolve a detached checkout at a release tag (a published release).

    Args:
        checkout: The source checkout.
        tag: The release tag at HEAD.

    Returns:
        The tag as version, ``commits_since`` 0 and the tagged commit.

    Raises:
        ToolError: If the line is 1.x, the branch is not in the checkout, or
            the tag is not on the branch.
    """
    major, minor, _ = parse_release_tag(tag)
    check_major(major, tag)
    source_sha = git(checkout, "rev-parse", "HEAD").stdout.strip()
    branch = f"{REMOTE}/rls-{major}.{minor}"
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


def resolve(checkout: Path) -> Resolved:
    """Resolve what the run publishes from the checkout alone.

    Args:
        checkout: The source checkout as the workflow (or a developer) left
            it: on ``rls-X.Y`` for a dispatch, detached at a ``vX.Y.Z`` tag
            for a release.

    Returns:
        The version, the commits since its release tag and the source SHA.

    Raises:
        ToolError: Every refusal (see the module docstring), including a
            detached HEAD without exactly one release tag.
    """
    check_history(checkout)
    head_ref = git(checkout, "symbolic-ref", "-q", "HEAD", check=False)
    if head_ref.returncode == 0:
        return _resolve_branch(checkout, head_ref.stdout.strip())
    tags = _release_tags_at_head(checkout)
    head = git(checkout, "rev-parse", "HEAD").stdout.strip()
    if not tags:
        raise ToolError(
            f"HEAD is detached at {head} and not at a release tag; check out "
            "rls-X.Y or a vX.Y.Z tag"
        )
    if len(tags) > 1:
        raise ToolError(
            f"HEAD at {head} carries several release tags, ambiguous: "
            + ", ".join(tags)
        )
    return _resolve_tag(checkout, tags[0])
