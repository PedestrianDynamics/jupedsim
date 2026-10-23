# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tag the source commit a publish was built from.

Every publish leaves an annotated tag on the source repository so what is
live can be traced back: ``docs-<version>-<n>-<TS>`` (``n`` = commits
since the release tag, 0 for a release) or ``site-<TS>``, with TS the UTC
time the tag is created, ``2026-10-20T14-02-10Z``. The tag message names
the Pages commit: ``<kind> publish of <source_sha> -> <branch>@<pages_sha>``.

The same source commit may be published more than once; each publish gets
its own tag because TS differs. A tag that already exists, locally or on
the remote, is an error (nothing is moved).
"""

from __future__ import annotations

from datetime import UTC, datetime
from pathlib import Path

from jps_site_tooling import versions
from jps_site_tooling.checkout import check_checkout
from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.git import git
from jps_site_tooling.publish import SHA_RE, validate_source_sha


def timestamp(now: datetime | None = None) -> str:
    """Format a tag timestamp.

    Args:
        now: The time to format; the current UTC time when omitted.

    Returns:
        ``YYYY-MM-DDTHH-MM-SSZ`` in UTC.
    """
    now = now or datetime.now(UTC)
    return now.astimezone(UTC).strftime("%Y-%m-%dT%H-%M-%SZ")


def tag_name(kind: str, version: str | None, count: int | None, ts: str) -> str:
    """Build the tag name for a publish.

    Args:
        kind: ``docs`` or ``site``.
        version: The docs version ``vX.Y.Z`` (docs only).
        count: Commits since the release tag (docs only).
        ts: The timestamp from ``timestamp``.

    Returns:
        ``docs-<version>-<count>-<ts>`` or ``site-<ts>``.

    Raises:
        UsageError: Unknown kind, a docs tag without version or count, a
            malformed version, or a negative count.
    """
    if kind == "site":
        return f"site-{ts}"
    if kind == "docs":
        if version is None or count is None:
            raise UsageError("a docs tag needs --version and --count")
        if not versions.VERSION_DIR.match(version):
            raise UsageError(f"version {version!r} is not of the form vX.Y.Z")
        if count < 0:
            raise UsageError(f"--count must be >= 0, got {count}")
        return f"docs-{version}-{count}-{ts}"
    raise UsageError(f"unknown tag kind {kind!r}")


def create_tag(
    checkout: Path, remote: str, name: str, source_sha: str, message: str
) -> None:
    """Create an annotated tag and push it.

    Args:
        checkout: A clone of the source repository holding ``source_sha``.
        remote: The remote to push the tag to.
        name: The tag name.
        source_sha: The commit to tag.
        message: The tag message.

    Raises:
        ToolError: If the tag exists locally or on the remote, or git fails.
    """
    ref = f"refs/tags/{name}"
    local = git(checkout, "rev-parse", "--verify", "-q", ref, check=False)
    if local.returncode == 0:
        raise ToolError(f"tag {name} already exists in {checkout}")
    if git(checkout, "ls-remote", "--tags", remote, ref).stdout.strip():
        raise ToolError(f"tag {name} already exists on {remote}")
    git(checkout, "tag", "-a", name, "-m", message, source_sha)
    git(checkout, "push", remote, ref)


def tag_publish(
    kind: str,
    checkout: Path,
    remote: str,
    branch: str,
    source_sha: str,
    pages_sha: str,
    version: str | None = None,
    count: int | None = None,
    now: datetime | None = None,
) -> str:
    """Tag the source commit of a finished publish.

    Args:
        kind: ``docs`` or ``site``.
        checkout: A clone of the source repository.
        remote: The remote to push the tag to.
        branch: The Pages branch that was published to (for the message).
        source_sha: The source commit that was built.
        pages_sha: The Pages commit written by the publish.
        version: The docs version (docs only).
        count: Commits since the release tag (docs only).
        now: The tag time; the current time when omitted.

    Returns:
        The tag name.

    Raises:
        UsageError: Bad kind, version, count, source or pages sha, or
            checkout.
        ToolError: The tag exists already or git fails.
    """
    validate_source_sha(source_sha)
    if not SHA_RE.fullmatch(pages_sha):
        raise UsageError(
            f"pages sha {pages_sha!r} is not a commit SHA (7 to 40 hex digits)"
        )
    name = tag_name(kind, version, count, timestamp(now))
    root = check_checkout(checkout)
    message = f"{kind} publish of {source_sha} -> {branch}@{pages_sha}"
    create_tag(root, remote, name, source_sha, message)
    return name
