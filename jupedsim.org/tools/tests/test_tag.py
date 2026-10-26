# SPDX-License-Identifier: LGPL-3.0-or-later
"""Source tags: name, timestamp, creation and push."""

import re
from datetime import UTC, datetime
from pathlib import Path

import pytest
from jps_website_tooling.cli import main
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.tag import (
    docs_tag_name,
    tag_docs,
    tag_www,
    timestamp,
    www_tag_name,
)
from tests.conftest import run_git, write

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


def test_tag_names() -> None:
    assert www_tag_name("T") == "www-T"
    assert docs_tag_name("v2.0.0", 0, "T") == "docs-v2.0.0-0-T"
    assert docs_tag_name("v2.0.0", 3, "T") == "docs-v2.0.0-3-T"
    with pytest.raises(UsageError, match="release tag"):
        docs_tag_name("2.0.0", 0, "T")
    with pytest.raises(UsageError, match="commits since"):
        docs_tag_name("v2.0.0", -1, "T")


def test_tag_pushed_to_remote(source_repo: tuple[Path, Path, str]) -> None:
    bare, clone, sha = source_repo
    name = tag_docs(clone, sha, PAGES_SHA, "v2.0.0", commits_since=3, now=NOW)
    assert name == "docs-v2.0.0-3-2026-10-20T14-02-10Z"
    assert run_git(["rev-parse", f"{name}^{{commit}}"], bare).strip() == sha
    assert run_git(["cat-file", "-t", name], bare).strip() == "tag"
    listing = run_git(["tag", "-l", "-n1", name], bare)
    assert f"docs publish of {sha} -> Pages commit {PAGES_SHA}" in listing


def test_tag_existing_name_fails(source_repo: tuple[Path, Path, str]) -> None:
    bare, clone, sha = source_repo
    name = tag_www(clone, sha, PAGES_SHA, now=NOW)
    with pytest.raises(ToolError, match="already exists"):
        tag_www(clone, sha, PAGES_SHA, now=NOW)
    run_git(["tag", "-d", name], clone)
    with pytest.raises(ToolError, match="already exists on origin"):
        tag_www(clone, sha, PAGES_SHA, now=NOW)
    assert run_git(["tag", "-l", "www-*"], bare).split() == [name]


def test_tag_rejects_bad_source_sha(
    source_repo: tuple[Path, Path, str],
) -> None:
    bare, clone, sha = source_repo
    with pytest.raises(UsageError, match="source sha"):
        tag_www(clone, "master", PAGES_SHA)
    with pytest.raises(UsageError, match="pages sha"):
        tag_docs(clone, sha, "HEAD", "v2.0.0")
    assert run_git(["tag", "-l"], bare).strip() == ""


def test_tag_docs_release_has_commits_since_zero(
    source_repo: tuple[Path, Path, str],
) -> None:
    bare, clone, sha = source_repo
    assert tag_docs(clone, sha, PAGES_SHA, "v2.0.0", now=NOW) == (
        "docs-v2.0.0-0-2026-10-20T14-02-10Z"
    )
    assert run_git(["tag", "-l", "docs-*"], bare).split() == [
        "docs-v2.0.0-0-2026-10-20T14-02-10Z"
    ]


def test_cli_tag_output(
    source_repo: tuple[Path, Path, str], capsys: pytest.CaptureFixture[str]
) -> None:
    bare, clone, sha = source_repo
    common = [
        "--checkout",
        str(clone),
        "--source-sha",
        sha,
        "--pages-sha",
        PAGES_SHA,
    ]
    assert main(["tag", "www", *common]) == 0
    lines = [
        line
        for line in capsys.readouterr().out.splitlines()
        if line.startswith("tag=www-")
    ]
    assert len(lines) == 1
    name = lines[0].removeprefix("tag=")
    assert run_git(["tag", "-l", name], bare).strip() == name
    assert main(["tag"]) == 2
    assert main(["tag", "--kind", "www", *common]) == 2
    assert main(["tag", "docs", *common]) == 2  # --version is required


def test_cli_tag_docs_commits_since(
    source_repo: tuple[Path, Path, str], capsys: pytest.CaptureFixture[str]
) -> None:
    bare, clone, sha = source_repo
    docs = ["tag", "docs", "--version", "v2.0.0", "--checkout", str(clone)]
    shas = ["--source-sha", sha, "--pages-sha", PAGES_SHA]
    assert main([*docs, *shas]) == 0  # omitted: the release itself
    out = capsys.readouterr().out
    assert re.search(r"^tag=docs-v2\.0\.0-0-", out, re.M)
    assert main([*docs, "--commits-since", "0", *shas]) == 2
    assert main([*docs, "--commits-since", "-2", *shas]) == 2
    assert main([*docs, "--commits-since", "3", *shas]) == 0
    out = capsys.readouterr().out
    assert re.search(r"^tag=docs-v2\.0\.0-3-", out, re.M)
    assert len(run_git(["tag", "-l", "docs-*"], bare).split()) == 2
