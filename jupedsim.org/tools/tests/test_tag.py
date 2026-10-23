# SPDX-License-Identifier: LGPL-3.0-or-later
"""Source tags: name, timestamp, creation and push."""

import re
from datetime import UTC, datetime
from pathlib import Path

import pytest
from jps_site_tooling.cli import main
from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.tag import tag_name, tag_publish, timestamp
from tests.conftest import PAGES_BRANCH, run_git, write

NOW = datetime(2026, 10, 20, 14, 2, 10, tzinfo=UTC)
PAGES_SHA = "f" * 40


@pytest.fixture
def source_repo(tmp_path: Path, git_env: None) -> tuple[Path, Path, str]:
    """A bare source repository and a clone with one commit on master.

    Args:
        tmp_path: Pytest's scratch directory.
        git_env: Git isolation (dependency only).

    Returns:
        The bare repository, the clone and the commit's SHA.
    """
    bare = tmp_path / "source.git"
    run_git(["init", "-q", "--bare", str(bare)], tmp_path)
    clone = tmp_path / "clone"
    run_git(["clone", "-q", str(bare), str(clone)], tmp_path)
    run_git(["checkout", "-q", "-b", "master"], clone)
    write(clone / "README", "source\n")
    run_git(["add", "-A"], clone)
    run_git(["commit", "-q", "-m", "src"], clone)
    run_git(["push", "-q", "origin", "master"], clone)
    sha = run_git(["rev-parse", "HEAD"], clone).strip()
    return bare, clone, sha


def test_timestamp_format() -> None:
    assert timestamp(NOW) == "2026-10-20T14-02-10Z"
    assert re.fullmatch(r"\d{4}-\d\d-\d\dT\d\d-\d\d-\d\dZ", timestamp())


def test_tag_name() -> None:
    assert tag_name("site", None, None, "T") == "site-T"
    assert tag_name("docs", "v2.0.0", 3, "T") == "docs-v2.0.0-3-T"
    bad: list[tuple[str, str | None, int | None]] = [
        ("docs", None, 0),
        ("docs", "v2.0.0", None),
        ("docs", "2.0.0", 0),
        ("docs", "v2.0.0", -1),
        ("other", None, None),
    ]
    for kind, version, count in bad:
        with pytest.raises(UsageError):
            tag_name(kind, version, count, "T")


def test_tag_pushed_to_remote(source_repo: tuple[Path, Path, str]) -> None:
    bare, clone, sha = source_repo
    name = tag_publish(
        "docs",
        clone,
        "origin",
        PAGES_BRANCH,
        sha,
        PAGES_SHA,
        version="v2.0.0",
        count=3,
        now=NOW,
    )
    assert name == "docs-v2.0.0-3-2026-10-20T14-02-10Z"
    assert run_git(["rev-parse", f"{name}^{{commit}}"], bare).strip() == sha
    assert run_git(["cat-file", "-t", name], bare).strip() == "tag"
    listing = run_git(["tag", "-l", "-n1", name], bare)
    assert f"docs publish of {sha} -> gh-pages-new@{PAGES_SHA}" in listing


def test_tag_existing_name_fails(source_repo: tuple[Path, Path, str]) -> None:
    bare, clone, sha = source_repo
    name = tag_publish(
        "site", clone, "origin", PAGES_BRANCH, sha, PAGES_SHA, now=NOW
    )
    with pytest.raises(ToolError, match="already exists"):
        tag_publish(
            "site", clone, "origin", PAGES_BRANCH, sha, PAGES_SHA, now=NOW
        )
    run_git(["tag", "-d", name], clone)
    with pytest.raises(ToolError, match="already exists on origin"):
        tag_publish(
            "site", clone, "origin", PAGES_BRANCH, sha, PAGES_SHA, now=NOW
        )
    assert run_git(["tag", "-l", "site-*"], bare).split() == [name]


def test_tag_rejects_bad_source_sha(
    source_repo: tuple[Path, Path, str],
) -> None:
    bare, clone, sha = source_repo
    with pytest.raises(UsageError, match="source sha"):
        tag_publish("site", clone, "origin", PAGES_BRANCH, "master", PAGES_SHA)
    with pytest.raises(UsageError, match="pages sha"):
        tag_publish("site", clone, "origin", PAGES_BRANCH, sha, "HEAD")
    assert run_git(["tag", "-l"], bare).strip() == ""


def test_cli_tag_output(
    source_repo: tuple[Path, Path, str], capsys: pytest.CaptureFixture[str]
) -> None:
    bare, clone, sha = source_repo
    common = [
        "--checkout",
        str(clone),
        "--branch",
        PAGES_BRANCH,
        "--source-sha",
        sha,
        "--pages-sha",
        PAGES_SHA,
    ]
    assert main(["tag", "--kind", "site", *common]) == 0
    lines = [
        line
        for line in capsys.readouterr().out.splitlines()
        if line.startswith("tag=site-")
    ]
    assert len(lines) == 1
    name = lines[0].removeprefix("tag=")
    assert run_git(["tag", "-l", name], bare).strip() == name
    # docs needs --version and --count
    assert main(["tag", "--kind", "docs", *common]) == 2
