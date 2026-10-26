# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tag the source commit a publish was built from.

Every publish leaves an annotated tag on the source repository so what is
live can be traced back: ``docs-<version>-<commits_since>-<TS>`` (commits
since the release tag; 0 for the release itself) or ``www-<TS>``, with TS
the UTC time the tag is created, ``2026-10-20T14-02-10Z``. The tag message
names the Pages commit: ``<kind> publish of <source_sha> -> Pages commit
<pages_sha>``.

The same source commit may be published more than once; each publish gets
its own tag because TS differs. A tag that already exists, locally or on
``origin``, is an error (nothing is moved).
"""

from __future__ import annotations

from datetime import UTC, datetime
from pathlib import Path

from jps_website_tooling import versions
from jps_website_tooling.checkout import check_checkout
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.git import REMOTE, git
from jps_website_tooling.publish import validate_sha


def timestamp(now: datetime | None = None) -> str:
    """Format a tag timestamp.

    Args:
        now: The time to format; the current UTC time when omitted.

    Returns:
        ``YYYY-MM-DDTHH-MM-SSZ`` in UTC.
    """
    now = now or datetime.now(UTC)
    return now.astimezone(UTC).strftime("%Y-%m-%dT%H-%M-%SZ")


def docs_tag_name(version: str, commits_since: int, ts: str) -> str:
    """Build the tag name of a docs publish.

    Args:
        version: The docs version ``vX.Y.Z``.
        commits_since: Commits since the release tag (0 for the release).
        ts: The timestamp from ``timestamp``.

    Returns:
        ``docs-<version>-<commits_since>-<ts>``.

    Raises:
        UsageError: A malformed version or a negative count.
    """
    if not versions.VERSION_DIR.match(version):
        raise UsageError(f"version {version!r} is not a release tag (vX.Y.Z)")
    if commits_since < 0:
        raise UsageError(
            f"commits since the release must be >= 0, got {commits_since}"
        )
    return f"docs-{version}-{commits_since}-{ts}"


def www_tag_name(ts: str) -> str:
    """Build the tag name of a www publish.

    Args:
        ts: The timestamp from ``timestamp``.

    Returns:
        ``www-<ts>``.
    """
    return f"www-{ts}"


def create_tag(
    checkout: Path, name: str, source_sha: str, message: str
) -> None:
    """Create an annotated tag and push it to ``origin``.

    Args:
        checkout: A clone of the source repository holding ``source_sha``.
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
    if git(checkout, "ls-remote", "--tags", REMOTE, ref).stdout.strip():
        raise ToolError(f"tag {name} already exists on {REMOTE}")
    git(checkout, "tag", "-a", name, "-m", message, source_sha)
    git(checkout, "push", REMOTE, ref)


def _tag_publish(
    kind: str, name: str, checkout: Path, source_sha: str, pages_sha: str
) -> str:
    """Validate the SHAs and the checkout, then create and push the tag.

    Args:
        kind: ``docs`` or ``www``, for the message.
        name: The tag name.
        checkout: A clone of the source repository.
        source_sha: The source commit that was built.
        pages_sha: The Pages commit written by the publish.

    Returns:
        The tag name.

    Raises:
        UsageError: Bad source or pages sha, or checkout.
        ToolError: The tag exists already or git fails.
    """
    validate_sha(source_sha, "source sha")
    validate_sha(pages_sha, "pages sha")
    root = check_checkout(checkout)
    message = f"{kind} publish of {source_sha} -> Pages commit {pages_sha}"
    create_tag(root, name, source_sha, message)
    return name


def tag_docs(
    checkout: Path,
    source_sha: str,
    pages_sha: str,
    version: str,
    commits_since: int = 0,
    now: datetime | None = None,
) -> str:
    """Tag the source commit of a finished docs publish.

    Args:
        checkout: A clone of the source repository.
        source_sha: The source commit that was built.
        pages_sha: The Pages commit written by the publish.
        version: The docs version ``vX.Y.Z``.
        commits_since: Commits since the release tag; 0 (the default) for
            the release itself.
        now: The tag time; the current time when omitted.

    Returns:
        The tag name.

    Raises:
        UsageError: Bad version, count, source or pages sha, or checkout.
        ToolError: The tag exists already or git fails.
    """
    name = docs_tag_name(version, commits_since, timestamp(now))
    return _tag_publish("docs", name, checkout, source_sha, pages_sha)


def tag_www(
    checkout: Path,
    source_sha: str,
    pages_sha: str,
    now: datetime | None = None,
) -> str:
    """Tag the source commit of a finished www publish.

    Args:
        checkout: A clone of the source repository.
        source_sha: The source commit that was built.
        pages_sha: The Pages commit written by the publish.
        now: The tag time; the current time when omitted.

    Returns:
        The tag name.

    Raises:
        UsageError: Bad source or pages sha, or checkout.
        ToolError: The tag exists already or git fails.
    """
    return _tag_publish(
        "www", www_tag_name(timestamp(now)), checkout, source_sha, pages_sha
    )
