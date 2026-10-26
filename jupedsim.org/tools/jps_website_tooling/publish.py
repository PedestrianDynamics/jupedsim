# SPDX-License-Identifier: LGPL-3.0-or-later
"""Publish a documentation version or the www pages to the Pages branch.

``publish_docs`` and ``publish_www`` work on a clean clone of the Pages
branch (the *checkout*) and a ``build-website`` output (the *build*); the
checkout is reset and cleaned (``git clean -ffdx``) on every attempt, so the
build and this package must live outside it (side by side, as the
workflows check them out).

Pages layout maintained::

    /CNAME, /.nojekyll        never touched
    /docs/index.html          redirect to /docs/stable/
    /docs/stable              relative symlink (versions.write_versions)
    /docs/versions.json       version switcher data (versions.write_versions)
    /docs/vX.Y.Z/             one directory per published version
    /docs/v1x/                hand-curated 1.x archive, never read or written
    /pagefind/                search index, rebuilt by every publish
    everything else           the www pages, owned by ``publish_www``

``publish_docs`` replaces ``docs/<version>/`` with ``rsync -a --delete`` (a
doc fix that removed a file removes it here too), rewrites ``docs/stable``
and ``docs/versions.json``, writes the ``docs/index.html`` redirect and
rebuilds the index. ``publish_www`` replaces everything at the root except
``docs/``, ``CNAME`` and ``.nojekyll`` and rebuilds the index.

The push loop makes five attempts; before each one the checkout is reset to
the remote tip and the content staged again, so a push rejected by a
competing publisher is retried on top of it (backoff 5, 10, 15, 20 s); after
the last rejection the checkout is reset to the remote tip. An
unchanged tree (the same www build published twice) is not an error: nothing is
committed and ``changed`` is False.
"""

from __future__ import annotations

import re
import subprocess
import time
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from jps_website_tooling import pagefind, versions
from jps_website_tooling.checkout import check_checkout
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.git import REFSPEC, REMOTE, check_pages_checkout, git
from jps_website_tooling.out import write_warning

ATTEMPTS = 5
WWW_EXCLUDES: tuple[str, ...] = (
    "/.git",
    "/CNAME",
    "/.nojekyll",
    "/docs",
    "/pagefind",
)
SHA_RE = re.compile(r"[0-9a-f]{7,40}")
"""A full or abbreviated commit SHA (matched with ``fullmatch``)."""

# The /docs/ landing page: from the first 2.x release on, /docs/ leads to
# /docs/stable/ (query and hash are kept).
REDIRECT_HTML = """\
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Redirecting to /docs/stable/</title>
<link rel="canonical" href="https://www.jupedsim.org/docs/stable/">
<meta name="robots" content="noindex">
<script>location.replace("/docs/stable/" + location.search + location.hash)</script>
<meta http-equiv="refresh" content="0; url=/docs/stable/">
</head>
<body><a href="/docs/stable/">Redirecting to /docs/stable/</a></body>
</html>
"""

Stage = Callable[[Path], "list[str] | None"]
"""Stages content into the checkout; returns the paths to add (None: all)."""


@dataclass(frozen=True)
class PublishResult:
    """What a publish did.

    Attributes:
        pages_sha: The Pages branch commit now holding the content.
        changed: Whether a new commit was pushed (False: nothing differed).
    """

    pages_sha: str
    changed: bool


def rsync_tree(src: Path, dst: Path, excludes: tuple[str, ...] = ()) -> None:
    """``rsync -a --delete`` of ``src/`` over ``dst/`` (``dst`` is created).

    ``--ignore-times``: a replaced file of the same size whose mtime falls
    into the same second as the old one would otherwise be skipped (both
    trees are freshly written in CI); git decides what changed anyway.

    Args:
        src: Source tree.
        dst: Destination tree.
        excludes: rsync ``--exclude`` patterns, relative to ``dst``.

    Raises:
        ToolError: If rsync exits non-zero.
    """
    # rsync creates only the last path component of the destination.
    dst.parent.mkdir(parents=True, exist_ok=True)
    cmd = ["rsync", "-a", "--delete", "--ignore-times"]
    for pattern in excludes:
        cmd += ["--exclude", pattern]
    cmd += [f"{src}/", f"{dst}/"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise ToolError(
            f"rsync {src} -> {dst} failed ({result.returncode}):\n"
            f"{result.stderr.strip()}"
        )


def stage_docs(root: Path, build: Path, version: str) -> list[str]:
    """Stage ``docs/<version>`` and the docs metadata into the checkout.

    Args:
        root: The Pages checkout.
        build: The ``build-website`` output holding ``docs/<version>/``.
        version: The version directory name, e.g. ``v2.0.0``.

    Returns:
        The paths to ``git add``.

    Raises:
        ToolError: Propagated from rsync, versions or Pagefind.
    """
    rsync_tree(build / "docs" / version, root / "docs" / version)
    (root / "docs" / "index.html").write_text(REDIRECT_HTML, encoding="utf-8")
    versions.write_versions(root / "docs")
    pagefind.build_index(root)
    return [
        f"docs/{version}",
        "docs/stable",
        "docs/versions.json",
        "docs/index.html",
        "pagefind",
    ]


def stage_www(root: Path, build: Path) -> None:
    """Replace the www pages at the root, keeping ``docs/``, CNAME and .nojekyll.

    Args:
        root: The Pages checkout.
        build: The ``build-website --target www`` output.

    Raises:
        ToolError: Propagated from rsync or Pagefind.
    """
    rsync_tree(build, root, WWW_EXCLUDES)
    pagefind.build_index(root)


def publish(
    checkout: Path,
    stage: Stage,
    message: str | Callable[[], str],
    *,
    attempts: int = ATTEMPTS,
    sleep: Callable[[float], None] = time.sleep,
) -> PublishResult:
    """Stage, commit and push with retries.

    Every attempt starts from the remote tip (fetch, reset, clean; the
    checkout guard's fetch serves the first attempt) and stages again, so a push rejected by a competing publisher is retried on
    top of the competitor's commit.

    Args:
        checkout: A clean clone of the branch to publish to.
        stage: Writes the content into the checkout; returns the paths to
            add, or None for ``git add -A``.
        message: The commit message, or a callable evaluated after staging
            (on every attempt) for messages that depend on what was staged.
        attempts: Number of push attempts.
        sleep: Called with the backoff in seconds between attempts.

    Returns:
        The Pages commit and whether anything was pushed.

    Raises:
        ToolError: If the checkout is unusable, a git call fails, or every
            push was rejected.
    """
    branch = check_pages_checkout(checkout)
    last_error = ""
    for attempt in range(1, attempts + 1):
        if attempt > 1:  # the guard has just fetched for the first attempt
            git(
                checkout,
                "fetch",
                REMOTE,
                REFSPEC.format(remote=REMOTE, branch=branch),
            )
        git(checkout, "reset", "--hard", f"{REMOTE}/{branch}")
        git(checkout, "clean", "-ffdx")
        paths = stage(checkout)
        git(checkout, "add", "-A", *(paths if paths is not None else []))
        staged = git(checkout, "diff", "--cached", "--quiet", check=False)
        if staged.returncode == 0:
            return PublishResult(_head(checkout), False)
        subject = message() if callable(message) else message
        git(checkout, "commit", "-q", "-m", subject)
        pushed = git(checkout, "push", REMOTE, f"HEAD:{branch}", check=False)
        if pushed.returncode == 0:
            return PublishResult(_head(checkout), True)
        last_error = pushed.stderr.strip()
        again = ", retrying" if attempt < attempts else ""
        write_warning(
            f"Push rejected (attempt {attempt}/{attempts}){again}:\n{last_error}"
        )
        if attempt < attempts:
            sleep(attempt * 5)
    # Leave no unpushed commit behind: a re-run in the same checkout would
    # otherwise fail the branch guard with a misleading message.
    git(checkout, "reset", "--hard", f"{REMOTE}/{branch}")
    raise ToolError(
        f"Failed to push to {REMOTE} {branch} after {attempts} attempts; "
        f"last rejection:\n{last_error}"
    )


def _head(checkout: Path) -> str:
    """Read HEAD of a checkout.

    Args:
        checkout: The working tree.

    Returns:
        The commit SHA.
    """
    return git(checkout, "rev-parse", "HEAD").stdout.strip()


def validate_sha(sha: str, what: str) -> str:
    """Check that ``sha`` looks like a (possibly abbreviated) commit SHA.

    Args:
        sha: The commit as given on the command line.
        what: What it is, for the message (``source sha``, ``pages sha``).

    Returns:
        ``sha`` unchanged.

    Raises:
        UsageError: If it is not 7 to 40 lowercase hex digits.
    """
    if not SHA_RE.fullmatch(sha):
        raise UsageError(
            f"{what} {sha!r} is not a commit SHA (7 to 40 hex digits)"
        )
    return sha


def publish_docs(
    build: Path,
    version: str,
    checkout: Path,
    source_sha: str,
    *,
    sleep: Callable[[float], None] = time.sleep,
) -> PublishResult:
    """Publish ``docs/<version>/`` from a build to the Pages branch.

    Args:
        build: The ``build-website`` output.
        version: The docs version, ``vX.Y.Z``.
        checkout: A clean clone of the Pages branch (checked out).
        source_sha: The source commit the build came from.
        sleep: Backoff function for the retry loop.

    Returns:
        The Pages commit and whether anything was pushed.

    Raises:
        UsageError: Bad version or source sha, unusable build or checkout.
        ToolError: A failing step in the loop.
    """
    if not versions.VERSION_DIR.match(version):
        raise UsageError(f"version {version!r} is not of the form vX.Y.Z")
    validate_sha(source_sha, "source sha")
    build = build.resolve()
    if not (build / "docs" / version / "index.html").is_file():
        raise UsageError(
            f"{build} has no docs/{version}/index.html (built with a "
            "different version, or not a build-website output)"
        )
    root = check_checkout(checkout, build)
    return publish(
        root,
        lambda tree: stage_docs(tree, build, version),
        f"docs: publish {version} ({source_sha[:9]})",
        sleep=sleep,
    )


def publish_www(
    build: Path,
    checkout: Path,
    source_sha: str,
    *,
    sleep: Callable[[float], None] = time.sleep,
) -> PublishResult:
    """Publish the www pages from a build to the Pages branch.

    Args:
        build: The ``build-website --target www`` output.
        checkout: A clean clone of the Pages branch (checked out).
        source_sha: The source commit the build came from.
        sleep: Backoff function for the retry loop.

    Returns:
        The Pages commit and whether anything was pushed.

    Raises:
        UsageError: Bad source sha, unusable build or checkout.
        ToolError: A failing step in the loop.
    """
    validate_sha(source_sha, "source sha")
    build = build.resolve()
    if not (build / "index.html").is_file():
        raise UsageError(f"{build} has no index.html (not a www build)")
    root = check_checkout(checkout, build)

    def stage(tree: Path) -> None:
        """Stage the www pages; everything is added (``git add -A``).

        Args:
            tree: The Pages checkout.
        """
        stage_www(tree, build)

    return publish(
        root,
        stage,
        f"www: publish {source_sha[:9]}",
        sleep=sleep,
    )
