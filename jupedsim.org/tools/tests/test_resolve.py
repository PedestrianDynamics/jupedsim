# SPDX-License-Identifier: LGPL-3.0-or-later
"""Resolve: which docs version a dispatch or a release publishes."""

import re
from pathlib import Path

import pytest
from jps_site_tooling.cli import main
from jps_site_tooling.errors import ToolError
from jps_site_tooling.resolve import (
    Resolved,
    parse_release_branch,
    parse_release_tag,
    resolve_dispatch,
    resolve_release,
)
from tests.conftest import run_git, write


def commit(repo: Path, name: str) -> None:
    """Add a file and commit it.

    Args:
        repo: The working tree.
        name: File name and commit subject.
    """
    write(repo / name, f"{name}\n")
    run_git(["add", "-A"], repo)
    run_git(["commit", "-q", "-m", name], repo)


def sha(repo: Path, rev: str) -> str:
    """Resolve a revision to its SHA.

    Args:
        repo: The working tree.
        rev: A revision expression.

    Returns:
        The full SHA.
    """
    return run_git(["rev-parse", "--verify", rev], repo).strip()


@pytest.fixture
def source_repo(tmp_path: Path, git_env: None) -> Path:
    """A clone of a source repository with release lines and tags.

    Layout (every branch and tag is also on ``origin``)::

        master:  c0 - c1 - c2 (v2.0.2, lightweight) - c3 (v5.0.0)
        rls-2.0: from c1: r1 (v2.0.0) - r2 (v2.0.1) - r3 - r4 - r5
        rls-2.1: from r2 (no own tag)
        rls-1.4: from c0: o1 (v1.4.2) - o2
        rls-9.9: from c0 (no tag reachable)
        rls-3.0: from c0: p1 (v3.0.0-rc1)
        rls-2.2: from c1: q1 (annotated v2.2.0-rc1 AND lightweight v2.2.0) - q2
        rls-2.3: from c1: s1 (v2.3.0) - merge of rls-2.0

    Args:
        tmp_path: Pytest's scratch directory.
        git_env: Git isolation (dependency only).

    Returns:
        The fresh clone, on ``master``.
    """
    bare = tmp_path / "source.git"
    run_git(["init", "-q", "--bare", str(bare)], tmp_path)
    seed = tmp_path / "seed"
    run_git(["clone", "-q", str(bare), str(seed)], tmp_path)
    run_git(["checkout", "-q", "-b", "master"], seed)
    commit(seed, "c0")
    commit(seed, "c1")
    run_git(["branch", "rls-2.0"], seed)
    commit(seed, "c2")
    run_git(["tag", "v2.0.2"], seed)
    commit(seed, "c3")
    run_git(["tag", "-a", "-m", "v5.0.0", "v5.0.0"], seed)
    run_git(["checkout", "-q", "rls-2.0"], seed)
    commit(seed, "r1")
    run_git(["tag", "-a", "-m", "v2.0.0", "v2.0.0"], seed)
    commit(seed, "r2")
    run_git(["tag", "-a", "-m", "v2.0.1", "v2.0.1"], seed)
    run_git(["branch", "rls-2.1"], seed)
    for name in ("r3", "r4", "r5"):
        commit(seed, name)
    c0 = sha(seed, "master~3")
    run_git(["checkout", "-q", "-b", "rls-1.4", c0], seed)
    commit(seed, "o1")
    run_git(["tag", "v1.4.2"], seed)
    commit(seed, "o2")
    run_git(["branch", "rls-9.9", c0], seed)
    run_git(["checkout", "-q", "-b", "rls-3.0", c0], seed)
    commit(seed, "p1")
    run_git(["tag", "v3.0.0-rc1"], seed)
    c1 = sha(seed, "master~2")
    run_git(["checkout", "-q", "-b", "rls-2.2", c1], seed)
    commit(seed, "q1")
    run_git(["tag", "-a", "-m", "v2.2.0-rc1", "v2.2.0-rc1"], seed)
    run_git(["tag", "v2.2.0"], seed)
    commit(seed, "q2")
    run_git(["checkout", "-q", "-b", "rls-2.3", c1], seed)
    commit(seed, "s1")
    run_git(["tag", "v2.3.0"], seed)
    run_git(["merge", "-q", "--no-edit", "rls-2.0"], seed)
    run_git(["push", "-q", "--all", "origin"], seed)
    run_git(["push", "-q", "--tags", "origin"], seed)
    clone = tmp_path / "clone"
    run_git(["clone", "-q", "-b", "master", str(bare), str(clone)], tmp_path)
    return clone


def test_dispatch_on_release_branch(source_repo: Path) -> None:
    run_git(["checkout", "-q", "rls-2.0"], source_repo)
    r = resolve_dispatch(source_repo, "refs/heads/rls-2.0")
    assert r == Resolved("v2.0.1", 3, sha(source_repo, "rls-2.0"))


def test_dispatch_at_tag_has_n_zero(source_repo: Path) -> None:
    run_git(["checkout", "-q", "v2.0.1"], source_repo)
    assert resolve_dispatch(source_repo, "refs/heads/rls-2.0").n == 0


def test_dispatch_rejects_non_release_branch(source_repo: Path) -> None:
    with pytest.raises(
        ToolError, match=r"refs/heads/master is not a release branch"
    ):
        resolve_dispatch(source_repo, "refs/heads/master")
    with pytest.raises(ToolError, match="refs/tags/v2.0.0"):
        resolve_dispatch(source_repo, "refs/tags/v2.0.0")


def test_dispatch_rejects_no_reachable_tag(source_repo: Path) -> None:
    run_git(["checkout", "-q", "rls-9.9"], source_repo)
    with pytest.raises(ToolError, match="no release tag of line 9.9 reachable"):
        resolve_dispatch(source_repo, "refs/heads/rls-9.9")


def test_dispatch_rejects_tag_of_another_line(source_repo: Path) -> None:
    # rls-2.1 only reaches v2.0.x tags: not a release of its own line.
    run_git(["checkout", "-q", "rls-2.1"], source_repo)
    with pytest.raises(
        ToolError, match=r"no release tag of line 2.1 reachable"
    ):
        resolve_dispatch(source_repo, "refs/heads/rls-2.1")


def test_dispatch_ignores_non_release_tags(source_repo: Path) -> None:
    run_git(["checkout", "-q", "rls-3.0"], source_repo)
    with pytest.raises(
        ToolError, match=r"no release tag of line 3.0 reachable"
    ):
        resolve_dispatch(source_repo, "refs/heads/rls-3.0")


def test_dispatch_rc_and_release_on_one_commit(source_repo: Path) -> None:
    # An annotated rc tag next to the lightweight release tag (GitHub UI
    # release) must not hide the release.
    run_git(["checkout", "-q", "rls-2.2"], source_repo)
    r = resolve_dispatch(source_repo, "refs/heads/rls-2.2")
    assert (r.version, r.n) == ("v2.2.0", 1)


def test_dispatch_after_merging_another_line(source_repo: Path) -> None:
    # rls-2.3 merged rls-2.0 (whose v2.0.1 is nearer by walk depth); the
    # line's own tag wins and n counts everything since it, merge included.
    run_git(["checkout", "-q", "rls-2.3"], source_repo)
    r = resolve_dispatch(source_repo, "refs/heads/rls-2.3")
    assert (r.version, r.n) == ("v2.3.0", 6)


def test_dispatch_picks_highest_patch(source_repo: Path) -> None:
    run_git(["checkout", "-q", "rls-2.0"], source_repo)
    run_git(["tag", "v2.0.10", "rls-2.0~1"], source_repo)
    run_git(["tag", "v2.0.9", "rls-2.0"], source_repo)
    assert (
        resolve_dispatch(source_repo, "refs/heads/rls-2.0").version == "v2.0.10"
    )


def test_dispatch_rejects_1x(source_repo: Path) -> None:
    run_git(["checkout", "-q", "rls-1.4"], source_repo)
    with pytest.raises(ToolError, match="before 2.0 are closed"):
        resolve_dispatch(source_repo, "refs/heads/rls-1.4")
    with pytest.raises(ToolError, match="refusing refs/heads/rls-0.9"):
        resolve_dispatch(source_repo, "refs/heads/rls-0.9")


def test_shallow_checkout_is_diagnosed(
    source_repo: Path, tmp_path: Path
) -> None:
    # actions/checkout without fetch-depth: 0 gives a shallow clone; the
    # message must say so instead of blaming the tags.
    shallow = tmp_path / "shallow"
    origin = f"file://{source_repo.parent / 'source.git'}"
    run_git(
        [
            "clone",
            "-q",
            "--depth",
            "1",
            "--branch",
            "rls-2.0",
            origin,
            str(shallow),
        ],
        tmp_path,
    )
    with pytest.raises(ToolError, match="shallow.*fetch-depth: 0"):
        resolve_dispatch(shallow, "refs/heads/rls-2.0")
    with pytest.raises(ToolError, match="shallow.*fetch-depth: 0"):
        resolve_release(shallow, "v2.0.1", draft=False, prerelease=False)


def test_non_repository_is_diagnosed(tmp_path: Path, git_env: None) -> None:
    with pytest.raises(ToolError, match="not a git checkout"):
        resolve_dispatch(tmp_path, "refs/heads/rls-2.0")
    with pytest.raises(ToolError, match="not a git checkout"):
        resolve_release(tmp_path, "v2.0.1", draft=False, prerelease=False)


def test_line_match_is_numeric() -> None:
    assert parse_release_branch("refs/heads/rls-1.0") == (1, 0)
    assert parse_release_tag("v10.0.0") == (10, 0, 0)
    assert parse_release_tag("v10.0.0")[:2] != parse_release_branch(
        "refs/heads/rls-1.0"
    )


def test_release_on_branch(source_repo: Path) -> None:
    r = resolve_release(source_repo, "v2.0.1", draft=False, prerelease=False)
    assert r == Resolved("v2.0.1", 0, sha(source_repo, "v2.0.1^{commit}"))
    # Annotated: the tag object is not the commit.
    assert r.source_sha != sha(source_repo, "v2.0.1")


def test_release_tag_not_on_branch(source_repo: Path) -> None:
    with pytest.raises(ToolError, match=r"v2.0.2 .* is not on rls-2.0"):
        resolve_release(source_repo, "v2.0.2", draft=False, prerelease=False)


def test_release_refuses_draft_and_prerelease(source_repo: Path) -> None:
    with pytest.raises(ToolError, match="is a draft"):
        resolve_release(source_repo, "v2.0.1", draft=True, prerelease=False)
    with pytest.raises(ToolError, match="is a pre-release"):
        resolve_release(source_repo, "v2.0.1", draft=False, prerelease=True)


def test_release_refuses_1x_and_bad_tags(source_repo: Path) -> None:
    with pytest.raises(ToolError, match="before 2.0 are closed"):
        resolve_release(source_repo, "v1.4.2", draft=False, prerelease=False)
    # ASCII digits only, no leading zeros; the message shows the raw value.
    for bad in (
        "2.0.1",
        "v2.0",
        "v2.0.0-rc1",
        "v2.0.1\n",
        "v02.01.0",
        "v\u0662.0.1",
    ):
        with pytest.raises(ToolError, match=re.escape(repr(bad))):
            resolve_release(source_repo, bad, draft=False, prerelease=False)
    assert parse_release_tag("v2.0.10") == (2, 0, 10)
    with pytest.raises(ToolError):
        parse_release_branch("refs/heads/rls-02.1")


def test_release_tag_missing_in_checkout(source_repo: Path) -> None:
    with pytest.raises(ToolError, match="v2.0.3 is not in the checkout"):
        resolve_release(source_repo, "v2.0.3", draft=False, prerelease=False)


def test_release_branch_missing(source_repo: Path) -> None:
    with pytest.raises(
        ToolError, match="origin/rls-5.0 is not in the checkout"
    ):
        resolve_release(source_repo, "v5.0.0", draft=False, prerelease=False)


def test_cli_resolve_dispatch(
    source_repo: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    run_git(["checkout", "-q", "rls-2.0"], source_repo)
    args = ["resolve", "dispatch", "--checkout", str(source_repo), "--ref"]
    assert main([*args, "refs/heads/rls-2.0"]) == 0
    out = capsys.readouterr().out
    expected = (
        f"version=v2.0.1\nn=3\nsource_sha={sha(source_repo, 'rls-2.0')}\n"
    )
    assert expected in out
    assert main([*args, "refs/heads/master"]) == 1
    assert "error" in capsys.readouterr().out
    assert main(["resolve"]) == 2


def test_cli_resolve_release(
    source_repo: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    args = [
        "resolve",
        "release",
        "--tag",
        "v2.0.1",
        "--checkout",
        str(source_repo),
    ]
    assert main([*args, "--draft", "false", "--prerelease", "false"]) == 0
    assert "version=v2.0.1\nn=0\nsource_sha=" in capsys.readouterr().out
    assert main([*args, "--draft", "true", "--prerelease", "false"]) == 1
    assert main([*args, "--draft", "maybe", "--prerelease", "false"]) == 2
    missing = [
        "resolve", "release", "--tag", "v2.0.1",
        "--checkout", str(source_repo / "missing"),
        "--draft", "false", "--prerelease", "false",
    ]  # fmt: skip
    assert main(missing) == 2
