# SPDX-License-Identifier: LGPL-3.0-or-later
"""The git wrapper and the Pages checkout guard."""

from pathlib import Path

import pytest
from jps_site_tooling.errors import ToolError
from jps_site_tooling.git import check_pages_checkout, git
from tests.conftest import PAGES_BRANCH, PagesRepo, run_git, write


def test_git_failure_is_tool_error(pages_repo: PagesRepo) -> None:
    with pytest.raises(
        ToolError, match=r"git rev-parse --verify nope failed \(\d+\)"
    ):
        git(pages_repo.checkout, "rev-parse", "--verify", "nope")
    unchecked = git(
        pages_repo.checkout, "rev-parse", "--verify", "nope", check=False
    )
    assert unchecked.returncode != 0


def test_git_uses_bot_identity(
    pages_repo: PagesRepo, monkeypatch: pytest.MonkeyPatch
) -> None:
    (pages_repo.checkout / "f").write_text("x")
    git(pages_repo.checkout, "add", "f")
    git(pages_repo.checkout, "commit", "-q", "-m", "m")
    author = git(
        pages_repo.checkout, "log", "-1", "--format=%an <%ae>"
    ).stdout.strip()
    assert author == (
        "github-actions[bot] <github-actions[bot]@users.noreply.github.com>"
    )


def test_check_pages_checkout_accepts_clean_clone(
    pages_repo: PagesRepo,
) -> None:
    check_pages_checkout(pages_repo.checkout, "origin", PAGES_BRANCH)


def test_refuses_checkout_without_remote_branch(pages_repo: PagesRepo) -> None:
    with pytest.raises(ToolError, match="fetch"):
        check_pages_checkout(pages_repo.checkout, "origin", "no-such-branch")


def test_refuses_source_checkout(pages_repo: PagesRepo, tmp_path: Path) -> None:
    # A clone whose HEAD is on another (unrelated) branch of the same remote.
    src = tmp_path / "src"
    run_git(["clone", "-q", str(pages_repo.bare), str(src)], tmp_path)
    run_git(["checkout", "-q", "--orphan", "master"], src)
    write(src / "README", "source\n")
    run_git(["add", "-A"], src)
    run_git(["commit", "-q", "-m", "src"], src)
    with pytest.raises(
        ToolError, match="not a checkout of origin/gh-pages-new"
    ):
        check_pages_checkout(src, "origin", PAGES_BRANCH)


def test_refuses_dirty_pages_checkout(pages_repo: PagesRepo) -> None:
    write(pages_repo.checkout / "scratch.txt", "x")
    with pytest.raises(ToolError, match="scratch.txt"):
        check_pages_checkout(pages_repo.checkout, "origin", PAGES_BRANCH)


def test_refuses_non_repository(tmp_path: Path, git_env: None) -> None:
    with pytest.raises(ToolError, match="not a git checkout"):
        check_pages_checkout(tmp_path, "origin", PAGES_BRANCH)
