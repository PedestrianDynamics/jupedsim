# SPDX-License-Identifier: LGPL-3.0-or-later
"""PR previews: a site moved under a URL prefix, and pruning of old previews.

The site and the docs link to each other with root-absolute URLs
(``/docs/stable/``, ``/articles/``, ``/pagefind/pagefind.js``,
``/docs/versions.json``), which is right for jupedsim.org, served from a
domain root. Previews live under ``/pull-requests/<N>/``, so before a
preview is published every such URL is moved under that prefix:

- in every ``*.html`` file, root-absolute values of the ``href``, ``src``,
  ``action``, ``data-pagefind`` and ``data-json`` attributes get the prefix;
  protocol-relative (``//host``) and absolute (``https://``) URLs, relative
  links and fragments are left alone, and values that already carry the
  prefix are not changed again, so a second run changes nothing;
- ``docs/versions.json``: root-absolute ``url`` values get the prefix;
- ``--stable NAME``: when ``docs/NAME`` exists and ``docs/stable`` does
  not, ``docs/stable`` becomes a symlink to ``NAME`` (PR builds name their
  docs ``docs/dev/``; the site links to ``docs/stable/``).

Pruning removes the preview directories (``<root>/<PR number>/``) whose
pull request is no longer open. The lookup of a PR's state is a callable,
so the logic does not depend on GitHub; ``github_pr_lookup`` builds it with
PyGithub.
"""

from __future__ import annotations

import json
import re
import shutil
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.out import write_info

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
        stable_linked: Whether ``docs/stable`` was created.
    """

    files: int
    links: int
    urls: int
    stable_linked: bool


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


def rebase_links(
    tree: Path, prefix: str, stable: str | None = None
) -> RebaseResult:
    """Move a built site under a URL prefix.

    Args:
        tree: The site build (its root holds ``index.html``).
        prefix: The URL prefix, e.g. ``/pull-requests/42/``.
        stable: Create ``docs/stable -> stable`` when ``docs/stable`` is
            missing and ``docs/<stable>`` is a directory.

    Returns:
        What was changed.

    Raises:
        UsageError: If the prefix is malformed or the tree has no
            ``index.html``; nothing is touched then.
    """
    validate_prefix(prefix)
    if not (tree / "index.html").is_file():
        raise UsageError(f"{tree} is not a site build (no index.html)")
    files = links = 0
    for path in sorted(tree.rglob("*.html")):
        if path.is_symlink():
            continue
        n = rebase_html(path, prefix)
        files += bool(n)
        links += n
    versions = tree / "docs" / "versions.json"
    urls = rebase_versions(versions, prefix) if versions.is_file() else 0
    link = tree / "docs" / "stable"
    linked = False
    if (
        stable
        and (tree / "docs" / stable).is_dir()
        and not link.exists()
        and not link.is_symlink()
    ):
        link.symlink_to(stable, target_is_directory=True)
        linked = True
    return RebaseResult(files, links, urls, linked)


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
        """Ask GitHub for the pull request's state."""
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
