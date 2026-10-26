# SPDX-License-Identifier: LGPL-3.0-or-later
"""PR previews: a website build moved under a URL prefix, published and pruned.

The www pages and the docs link to each other with root-absolute URLs
(``/docs/stable/``, ``/articles/``, ``/pagefind/pagefind.js``,
``/docs/versions.json``), which is right for jupedsim.org, served from a
domain root. Previews live under ``/pull-requests/<N>/``, so before a
preview is published every such URL is moved under that prefix:

- in every ``*.html`` file, root-absolute values of the ``href``, ``src``,
  ``action``, ``data-pagefind`` and ``data-json`` attributes get the prefix;
  protocol-relative (``//host``) and absolute (``https://``) URLs, relative
  links and fragments are left alone, and values that already carry the
  prefix are not changed again, so a second run changes nothing;
- ``docs/versions.json``: root-absolute ``url`` values get the prefix.

PR builds use the docs version ``stable``, so ``docs/stable/`` is a plain
directory in the build and the Pagefind index already covers it; nothing
here creates symlinks.

``publish_preview`` and ``prune_previews`` run through
``publish.publish()``: every attempt starts from the remote tip and stages
again, so a push rejected by another preview deploy is retried on top of
it. Pruning recomputes the deletions per attempt: rebasing a stale deletion
commit could strip files of a preview re-deployed in the meantime (PR
reopened, build in flight). The lookup of a PR's state is a callable, so the
logic does not depend on GitHub; ``github_pr_lookup`` builds it with
PyGithub.
"""

from __future__ import annotations

import json
import re
import shutil
import time
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from jps_website_tooling.checkout import check_checkout
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.out import write_info
from jps_website_tooling.publish import PublishResult, publish, rsync_tree

ATTRS = ("href", "src", "action", "data-pagefind", "data-json")

PullRequestLookup = Callable[[int], bool | None]
"""Maps a PR number to open (True), not open (False) or unknown (None)."""


@dataclass(frozen=True)
class RebaseResult:
    """What ``rebase_links`` changed.

    Attributes:
        files: HTML files with at least one rewritten attribute.
        links: Rewritten attribute values.
        urls: Rewritten ``versions.json`` URLs.
    """

    files: int
    links: int
    urls: int


def validate_prefix(prefix: str) -> str:
    """Check a URL prefix such as ``/pull-requests/42/``.

    Args:
        prefix: The prefix to check.

    Returns:
        The prefix unchanged.

    Raises:
        UsageError: If it does not start and end with ``/`` or is just ``/``.
    """
    if not (
        prefix.startswith("/") and prefix.endswith("/") and len(prefix) > 1
    ):
        raise UsageError(f"prefix must start and end with '/': {prefix!r}")
    return prefix


def attr_pattern(prefix: str) -> re.Pattern[str]:
    """Build the regex matching root-absolute attribute values to rebase.

    Args:
        prefix: The URL prefix (validated).

    Returns:
        A pattern with two groups: the attribute with ``=`` and the quote;
        the match ends after the leading ``/`` of a value that does not
        already start with ``//`` or the prefix.
    """
    rest = re.escape(prefix[1:])
    names = "|".join(re.escape(a) for a in ATTRS)
    return re.compile(rf"(\s(?:{names})=)([\"'])/(?!/|{rest})")


def rebase_html(path: Path, prefix: str) -> int:
    """Move the root-absolute attribute values of one HTML file under a prefix.

    Args:
        path: The HTML file, rewritten in place when anything changes.
        prefix: The URL prefix.

    Returns:
        The number of rewritten attribute values.
    """
    text = path.read_text(encoding="utf-8")
    new, n = attr_pattern(prefix).subn(
        lambda m: f"{m.group(1)}{m.group(2)}{prefix}", text
    )
    if n:
        path.write_text(new, encoding="utf-8")
    return n


def rebase_versions(path: Path, prefix: str) -> int:
    """Move the root-absolute ``url`` values of a ``versions.json`` under a prefix.

    Args:
        path: The ``versions.json`` file, rewritten in place when anything
            changes.
        prefix: The URL prefix.

    Returns:
        The number of rewritten URLs.
    """
    entries = json.loads(path.read_text(encoding="utf-8"))
    n = 0
    for entry in entries:
        url = entry.get("url") if isinstance(entry, dict) else None
        if (
            isinstance(url, str)
            and url.startswith("/")
            and not url.startswith("//")
            and not url.startswith(prefix)
        ):
            entry["url"] = prefix + url[1:]
            n += 1
    if n:
        path.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    return n


def rebase_links(tree: Path, prefix: str) -> RebaseResult:
    """Move a website build under a URL prefix.

    Args:
        tree: The website build (its root holds ``index.html``).
        prefix: The URL prefix, e.g. ``/pull-requests/42/``.

    Returns:
        What was changed.

    Raises:
        UsageError: If the prefix is malformed or the tree has no
            ``index.html``; nothing is touched then.
    """
    validate_prefix(prefix)
    if not (tree / "index.html").is_file():
        raise UsageError(f"{tree} is not a website build (no index.html)")
    files = links = 0
    for path in sorted(tree.rglob("*.html")):
        if path.is_symlink():
            continue
        n = rebase_html(path, prefix)
        files += bool(n)
        links += n
    versions = tree / "docs" / "versions.json"
    urls = rebase_versions(versions, prefix) if versions.is_file() else 0
    return RebaseResult(files, links, urls)


def preview_prefix(number: int) -> str:
    """The URL prefix of a pull request's preview.

    Args:
        number: The pull request number.

    Returns:
        ``/pull-requests/<number>/``.

    Raises:
        UsageError: If ``number`` is not positive.
    """
    if number <= 0:
        raise UsageError(f"pull request number must be positive, got {number}")
    return f"/pull-requests/{number}/"


def publish_preview(
    build: Path,
    number: int,
    checkout: Path,
    *,
    sleep: Callable[[float], None] = time.sleep,
) -> PublishResult:
    """Publish a website build as the preview of a pull request.

    The links are rebased once, in place; every attempt of the push loop
    then only syncs the build into ``pull-requests/<number>/``.

    Args:
        build: The website build (the PR build artifact).
        number: The pull request number.
        checkout: A clean clone of the previews repository (``main``
            checked out).
        sleep: Backoff function for the retry loop.

    Returns:
        The previews commit and whether anything was pushed.

    Raises:
        UsageError: Bad number, unusable build or checkout.
        ToolError: A failing step in the loop.
    """
    prefix = preview_prefix(number)
    build = build.resolve()
    if not (build / "index.html").is_file():
        raise UsageError(f"{build} is not a website build (no index.html)")
    root = check_checkout(checkout, build)
    rebase_links(build, prefix)

    def stage(tree: Path) -> list[str]:
        """Sync the rebased build into the preview directory.

        Args:
            tree: The previews checkout.

        Returns:
            The preview directory to ``git add``.
        """
        rsync_tree(build, tree / "pull-requests" / str(number))
        return [f"pull-requests/{number}"]

    return publish(root, stage, f"docs: preview for PR #{number}", sleep=sleep)


def prune_previews(
    checkout: Path,
    is_open: PullRequestLookup,
    *,
    sleep: Callable[[float], None] = time.sleep,
) -> tuple[PublishResult, list[int]]:
    """Delete the previews of pull requests that are no longer open, and push.

    Args:
        checkout: A clean clone of the previews repository.
        is_open: The PR state lookup.
        sleep: Backoff function for the retry loop.

    Returns:
        The previews commit (``changed`` is False when nothing was deleted)
        and the numbers of the deleted previews.

    Raises:
        UsageError: Unusable checkout.
        ToolError: A failing lookup or step in the loop.
    """
    root = check_checkout(checkout)
    deleted: list[int] = []

    def stage(tree: Path) -> None:
        """Recompute and apply the deletions for this attempt.

        Args:
            tree: The previews checkout.
        """
        deleted[:] = [
            int(p.name) for p in prune(tree / "pull-requests", is_open)
        ]

    def message() -> str:
        """Name the previews this attempt deleted.

        Returns:
            The commit subject with the deleted PR numbers.
        """
        numbers = ", ".join(f"#{n}" for n in deleted)
        return f"docs: delete orphaned PR previews ({numbers})"

    result = publish(root, stage, message, sleep=sleep)
    return result, list(deleted)


def find_orphaned(root: Path, is_open: PullRequestLookup) -> list[Path]:
    """List the preview directories whose pull request is not open.

    Args:
        root: The ``pull-requests`` directory; a missing root holds nothing.
        is_open: The PR state lookup.

    Returns:
        The orphaned preview directories, sorted by name. Previews whose PR
        the lookup cannot find (None) are kept and reported.

    Raises:
        UsageError: If ``root`` exists but is not a directory.
        ToolError: Propagated from the lookup; nothing is decided then.
    """
    if not root.exists():
        return []
    if not root.is_dir():
        raise UsageError(f"{root} exists but is not a directory")
    orphaned: list[Path] = []
    for preview in sorted(root.iterdir()):
        if not preview.is_dir():
            continue
        try:
            number = int(preview.name)
        except ValueError:
            continue
        state = is_open(number)
        if state is None:
            write_info(f"pull request #{number} not found; keeping {preview}")
        elif not state:
            orphaned.append(preview)
    return orphaned


def prune(root: Path, is_open: PullRequestLookup) -> list[Path]:
    """Delete the preview directories whose pull request is not open.

    Args:
        root: The ``pull-requests`` directory.
        is_open: The PR state lookup.

    Returns:
        The deleted directories.

    Raises:
        UsageError: If ``root`` exists but is not a directory.
        ToolError: Propagated from the lookup, before anything is deleted.
    """
    orphaned = find_orphaned(root, is_open)
    for preview in orphaned:
        write_info(f"deleting orphaned preview {preview}")
        shutil.rmtree(preview)
    return orphaned


def github_pr_lookup(repo: str, token: str) -> PullRequestLookup:
    """Build a PR state lookup against GitHub.

    Args:
        repo: The repository as ``owner/name``.
        token: A token with read access to the repository's pull requests.

    Returns:
        A lookup returning True for an open PR, False for any other state
        and None when GitHub answers 404.

    Raises:
        ToolError: If the repository cannot be accessed (bad token, wrong
            name, missing permission).
    """
    # Imported here, not at module level: only pruning needs PyGithub.
    import github

    client = github.Github(auth=github.Auth.Token(token))
    try:
        repository = client.get_repo(repo)
    except github.GithubException as exc:
        raise ToolError(
            f"GitHub API error {exc.status} for repository {repo}: {exc.data}"
        ) from exc

    def is_open(number: int) -> bool | None:
        """Ask GitHub for the pull request's state.

        Args:
            number: The pull request number.

        Returns:
            True if open, False for any other state, None if GitHub answers
            404.

        Raises:
            ToolError: Any other GitHub API error.
        """
        try:
            pull = repository.get_pull(number)
        except github.GithubException as exc:
            if exc.status == 404:
                return None
            raise ToolError(
                f"GitHub API error {exc.status} for pull request #{number}: "
                f"{exc.data}"
            ) from exc
        return bool(pull.state == "open")

    return is_open
