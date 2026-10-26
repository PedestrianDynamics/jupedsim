# SPDX-License-Identifier: LGPL-3.0-or-later
"""The git wrapper and the Pages checkout guard."""

from pathlib import Path

import pytest
from jps_website_tooling.errors import ToolError
from jps_website_tooling.git import check_pages_checkout, git
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
    check_pages_checkout(pages_repo.checkout)


def test_refuses_checkout_without_remote_branch(pages_repo: PagesRepo) -> None:
    run_git(["checkout", "-q", "-b", "local-only"], pages_repo.checkout)
    with pytest.raises(ToolError, match="fetch"):
        check_pages_checkout(pages_repo.checkout)


def test_refuses_source_checkout(pages_repo: PagesRepo, tmp_path: Path) -> None:
    # The source repository: a clean clone on a branch that exists on origin,
    # recognisable by its layout (jupedsim.org/tools). The branch itself
    # names nothing, so the layout is what stops a reset and clean here.
    src = tmp_path / "src"
    run_git(["clone", "-q", str(pages_repo.bare), str(src)], tmp_path)
    run_git(["checkout", "-q", "--orphan", "master"], src)
    write(src / "jupedsim.org" / "tools" / "pyproject.toml", "[project]\n")
    run_git(["add", "-A"], src)
    run_git(["commit", "-q", "-m", "src"], src)
    run_git(["push", "-q", "-u", "origin", "master"], src)
    with pytest.raises(ToolError, match="source repository"):
        check_pages_checkout(src)


def test_refuses_dirty_pages_checkout(pages_repo: PagesRepo) -> None:
    write(pages_repo.checkout / "scratch.txt", "x")
    with pytest.raises(ToolError, match="scratch.txt"):
        check_pages_checkout(pages_repo.checkout)


def test_refuses_non_repository(tmp_path: Path, git_env: None) -> None:
    with pytest.raises(ToolError, match="not a git checkout"):
        check_pages_checkout(tmp_path)


def test_check_pages_checkout_returns_the_branch(pages_repo: PagesRepo) -> None:
    assert check_pages_checkout(pages_repo.checkout) == PAGES_BRANCH


def test_refuses_detached_checkout(pages_repo: PagesRepo) -> None:
    run_git(["checkout", "-q", "--detach"], pages_repo.checkout)
    with pytest.raises(ToolError, match="detached.*check out the branch"):
        check_pages_checkout(pages_repo.checkout)
