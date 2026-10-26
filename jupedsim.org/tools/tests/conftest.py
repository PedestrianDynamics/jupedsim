# SPDX-License-Identifier: LGPL-3.0-or-later
"""Shared pytest fixtures for jps_website_tooling.

Besides the environment-cache fixture, this module provides scratch git
repositories for the publishing tests: a bare ``origin`` holding the Pages
branch in the migrated layout and a clean clone of it.
"""

from __future__ import annotations

import os
import subprocess
import tempfile
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path

import pytest
from jps_website_tooling import out

PAGES_BRANCH = "gh-pages-new"
SOURCE_SHA = "abcdef1234567890abcdef1234567890abcdef12"


@pytest.fixture(autouse=True)
def _no_github_output(monkeypatch: pytest.MonkeyPatch) -> None:
    """Never append test outputs to a real ``GITHUB_OUTPUT`` file.

    Under Actions the variable is set for the whole job; a CLI test would
    otherwise leak ``key=value`` lines into the step's outputs.

    Args:
        monkeypatch: Pytest's environment patcher.
    """
    monkeypatch.delenv("GITHUB_OUTPUT", raising=False)


@pytest.fixture(autouse=True)
def _clear_env_cache() -> Iterator[None]:
    """Forget the cached GITHUB_ACTIONS lookup around every test.

    ``out.running_in_github_action`` is cached for the process; without
    this, the first test to call it would fix the answer for all later
    tests, and tests that set or unset ``GITHUB_ACTIONS`` would see stale
    results (in Actions the variable is set for the whole run).
    """
    out.running_in_github_action.cache_clear()
    yield
    out.running_in_github_action.cache_clear()


def run_git(args: list[str], cwd: Path) -> str:
    """Run ``git <args>`` in ``cwd`` as test user ``t`` and return its stdout.

    Args:
        args: Arguments after ``git``.
        cwd: Working directory (a checkout or a bare repository).

    Returns:
        Standard output of the command.

    Raises:
        subprocess.CalledProcessError: If git exits non-zero.
    """
    identity = ["-c", "user.name=t", "-c", "user.email=t@example.org"]
    return subprocess.run(
        ["git", *identity, *args],
        cwd=cwd,
        capture_output=True,
        text=True,
        check=True,
    ).stdout


def write(path: Path, content: str) -> None:
    """Write ``content`` to ``path``, creating parent directories.

    Args:
        path: The file to write.
        content: Its text.
    """
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def page(word: str) -> str:
    """A minimal HTML page with a Pagefind body containing ``word``.

    Args:
        word: The page's heading.

    Returns:
        The HTML text.
    """
    return (
        f"<!doctype html><html><body><main data-pagefind-body>"
        f"<h1>{word}</h1></main></body></html>\n"
    )


@pytest.fixture
def git_env(monkeypatch: pytest.MonkeyPatch, tmp_path: Path) -> None:
    """Isolate git from the developer's configuration and repositories.

    Only the config files are redirected; no identity is put into the
    environment, so that the per-command identity of ``git.GIT_CONFIG`` is
    what the tooling's commits carry (``run_git`` passes its own).
    Repository discovery stops at the scratch directory's parent: with a
    build directory inside the source checkout (CI), ``tmp_path`` lies
    inside a git work tree, and a test directory that is "not a git
    checkout" would otherwise resolve to the jupedsim repository.

    Args:
        monkeypatch: Pytest's environment patcher.
        tmp_path: Pytest's scratch directory.
    """
    monkeypatch.setenv("GIT_CONFIG_GLOBAL", os.devnull)
    monkeypatch.setenv("GIT_CONFIG_SYSTEM", os.devnull)
    monkeypatch.setenv("GIT_CEILING_DIRECTORIES", str(tmp_path.parent))
    for role in ("AUTHOR", "COMMITTER"):
        for field in ("NAME", "EMAIL", "DATE"):
            monkeypatch.delenv(f"GIT_{role}_{field}", raising=False)


@dataclass(frozen=True)
class PagesRepo:
    """A bare ``origin`` with the Pages branch and a clean clone of it.

    Attributes:
        bare: The bare repository.
        checkout: A clone with the Pages branch checked out.
        branch: The published branch (``gh-pages-new``; previews use ``main``).
    """

    bare: Path
    checkout: Path
    branch: str = PAGES_BRANCH

    def tip(self) -> str:
        """Read the Pages branch head on ``origin``.

        Returns:
            The commit SHA.
        """
        return run_git(["rev-parse", self.branch], self.bare).strip()

    def message(self) -> str:
        """Read the subject of the Pages branch's last commit on ``origin``.

        Returns:
            The commit subject line.
        """
        return run_git(
            ["log", "-1", "--format=%s", self.branch], self.bare
        ).strip()

    def tree(self, tip: str | None = None) -> Path:
        """A fresh clone of the Pages branch (or of ``tip``).

        Args:
            tip: A commit to check out instead of the branch head.

        Returns:
            The clone's directory.
        """
        dst = Path(tempfile.mkdtemp(prefix="tree-", dir=self.bare.parent))
        run_git(
            ["clone", "-q", "-b", self.branch, str(self.bare), str(dst)],
            self.bare.parent,
        )
        if tip:
            run_git(["checkout", "-q", tip], dst)
        return dst

    def push_competitor(self, name: str) -> None:
        """Push a commit adding ``name`` to the Pages branch from another clone.

        Args:
            name: The file the competitor adds.
        """
        other = Path(tempfile.mkdtemp(prefix="other-", dir=self.bare.parent))
        run_git(
            ["clone", "-q", "-b", self.branch, str(self.bare), str(other)],
            self.bare.parent,
        )
        write(other / name, "competitor\n")
        run_git(["add", "-A"], other)
        run_git(["commit", "-q", "-m", f"competitor {name}"], other)
        run_git(["push", "-q", "origin", self.branch], other)


@pytest.fixture
def pages_repo(tmp_path: Path, git_env: None) -> PagesRepo:
    """Create the Pages repository fixture.

    The seed commit holds ``CNAME``, ``.nojekyll``, ``old.html`` and the
    hand-curated ``docs/v1x/`` archive, on branch ``gh-pages-new``.

    Args:
        tmp_path: Pytest's scratch directory.
        git_env: Git isolation (dependency only).

    Returns:
        The bare repository and a clean clone.
    """
    bare = tmp_path / "origin.git"
    run_git(["init", "-q", "--bare", str(bare)], tmp_path)
    seed = tmp_path / "seed"
    run_git(["clone", "-q", str(bare), str(seed)], tmp_path)
    write(seed / "CNAME", "www.jupedsim.org\n")
    write(seed / ".nojekyll", "")
    write(seed / "old.html", page("oldpage"))
    write(seed / "docs/v1x/v1.4.2/index.html", page("archive"))
    write(seed / "docs/v1x/versions.json", "[]\n")
    run_git(["checkout", "-q", "-b", PAGES_BRANCH], seed)
    run_git(["add", "-A"], seed)
    run_git(["commit", "-q", "-m", "seed"], seed)
    run_git(["push", "-q", "origin", PAGES_BRANCH], seed)
    checkout = tmp_path / "checkout"
    run_git(
        ["clone", "-q", "-b", PAGES_BRANCH, str(bare), str(checkout)], tmp_path
    )
    return PagesRepo(bare, checkout)
