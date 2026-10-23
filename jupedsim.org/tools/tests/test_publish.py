# SPDX-License-Identifier: LGPL-3.0-or-later
"""Publishing: docs and site staging, the push-retry loop, the guards."""

import json
import shutil
from pathlib import Path

import pytest
from jps_site_tooling import pagefind
from jps_site_tooling.checkout import check_checkout
from jps_site_tooling.cli import main
from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.git import check_pages_checkout
from jps_site_tooling.publish import (
    SITE_EXCLUDES,
    PublishResult,
    publish,
    publish_docs,
    publish_site,
    rsync_tree,
    stage_docs,
)
from tests.conftest import (
    PAGES_BRANCH,
    SOURCE_SHA,
    PagesRepo,
    page,
    run_git,
    write,
)


@pytest.fixture
def stub_pagefind(monkeypatch: pytest.MonkeyPatch) -> None:
    """Replace the indexer: the old index is replaced by a stub file.

    Args:
        monkeypatch: Pytest's attribute patcher.
    """

    def fake_build_index(root: Path) -> Path:
        shutil.rmtree(root / "pagefind", ignore_errors=True)
        write(root / "pagefind" / "stub.txt", "pagefind stub\n")
        return root / "pagefind"

    monkeypatch.setattr(pagefind, "build_index", fake_build_index)


def docs_build(tmp: Path, version: str, files: dict[str, str]) -> Path:
    """A build-site output with ``docs/<version>/`` holding ``files``.

    Args:
        tmp: Parent directory.
        version: The docs version directory name.
        files: Relative path to content, under ``docs/<version>/``.

    Returns:
        The build directory.
    """
    build = tmp / "docs-build"
    write(build / "index.html", page("buildsite"))
    for rel, content in files.items():
        write(build / "docs" / version / rel, content)
    return build


def site_build(tmp: Path, files: dict[str, str]) -> Path:
    """A ``build-site --only site`` output holding ``files``.

    Args:
        tmp: Parent directory.
        files: Relative path to content.

    Returns:
        The build directory.
    """
    build = tmp / "site-build"
    for rel, content in files.items():
        write(build / rel, content)
    return build


def snapshot(root: Path) -> dict[str, bytes]:
    """Map every file under ``root`` to its content.

    Args:
        root: The directory to read.

    Returns:
        Relative POSIX path to file bytes, for comparing two trees.
    """
    return {
        str(p.relative_to(root).as_posix()): p.read_bytes()
        for p in sorted(root.rglob("*"))
        if p.is_file()
    }


def docs(
    repo: PagesRepo, build: Path, version: str = "v2.0.0", sha: str = SOURCE_SHA
) -> PublishResult:
    """Publish docs with the fixture defaults.

    Args:
        repo: The Pages repository fixture.
        build: The build-site output.
        version: The docs version.
        sha: The source commit.

    Returns:
        What the publish did.
    """
    return publish_docs(
        build, version, repo.checkout, "origin", PAGES_BRANCH, sha
    )


def test_version_rejected(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = docs_build(tmp_path, "v2.0.0", {"index.html": page("x")})
    tip = pages_repo.tip()
    with pytest.raises(UsageError, match="vX.Y.Z"):
        docs(pages_repo, build, version="2.0.0")
    assert pages_repo.tip() == tip


def test_source_sha_rejected(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = docs_build(tmp_path, "v2.0.0", {"index.html": page("x")})
    tip = pages_repo.tip()
    with pytest.raises(UsageError, match="source sha"):
        docs(pages_repo, build, sha="refs/heads/master")
    site = site_build(tmp_path, {"index.html": page("home")})
    with pytest.raises(UsageError, match="source sha"):
        publish_site(site, pages_repo.checkout, "origin", PAGES_BRANCH, "ABC")
    with pytest.raises(UsageError, match="source sha"):
        docs(pages_repo, build, sha=SOURCE_SHA + "\n")
    assert pages_repo.tip() == tip


def test_docs_refuses_bad_build(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = docs_build(tmp_path, "v2.0.1", {"index.html": page("x")})
    with pytest.raises(UsageError, match="docs/v2.0.0/index.html"):
        docs(pages_repo, build)


def test_site_refuses_bad_build(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = site_build(tmp_path, {"about.html": page("x")})
    with pytest.raises(UsageError, match="index.html"):
        publish_site(
            build, pages_repo.checkout, "origin", PAGES_BRANCH, SOURCE_SHA
        )


def test_refuses_nested_layout(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = docs_build(
        pages_repo.checkout / "artifact", "v2.0.0", {"index.html": page("x")}
    )
    tip = pages_repo.tip()
    with pytest.raises(UsageError, match="inside the Pages checkout"):
        docs(pages_repo, build)
    assert (build / "docs/v2.0.0/index.html").is_file()
    assert pages_repo.tip() == tip


def test_publish_refuses_source_checkout(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    # A clone of the same remote on an unrelated branch, with work in it.
    src = tmp_path / "src"
    run_git(["clone", "-q", str(pages_repo.bare), str(src)], tmp_path)
    run_git(["checkout", "-q", "--orphan", "master"], src)
    write(src / "README", "source\n")
    run_git(["add", "-A"], src)
    run_git(["commit", "-q", "-m", "src"], src)
    write(src / "notes.txt", "untracked work\n")
    build = docs_build(tmp_path, "v2.0.0", {"index.html": page("x")})
    tip = pages_repo.tip()
    with pytest.raises(
        ToolError, match="not a checkout of origin/gh-pages-new"
    ):
        publish_docs(build, "v2.0.0", src, "origin", PAGES_BRANCH, SOURCE_SHA)
    assert (src / "notes.txt").read_text() == "untracked work\n"
    assert (src / "README").is_file()
    assert pages_repo.tip() == tip


def test_publish_refuses_dirty_pages_checkout(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    write(pages_repo.checkout / "wip.html", page("wip"))
    build = docs_build(tmp_path, "v2.0.0", {"index.html": page("x")})
    tip = pages_repo.tip()
    with pytest.raises(ToolError, match="wip.html"):
        docs(pages_repo, build)
    assert (pages_repo.checkout / "wip.html").is_file()
    assert pages_repo.tip() == tip


def test_docs_publish(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = docs_build(
        tmp_path,
        "v2.0.0",
        {"index.html": page("two"), "api/index.html": page("api")},
    )
    result = publish_docs(
        build, "v2.0.0", pages_repo.checkout, "origin", PAGES_BRANCH, SOURCE_SHA
    )
    assert result.changed and result.pages_sha == pages_repo.tip()
    assert pages_repo.message() == "docs: publish v2.0.0 (abcdef123)"
    tree = pages_repo.tree()
    assert (tree / "docs/v2.0.0/api/index.html").read_text() == page("api")
    assert (tree / "docs/stable").readlink() == Path("v2.0.0")
    names = [
        e["name"] for e in json.loads((tree / "docs/versions.json").read_text())
    ]
    assert names == ["stable", "v2.0.0"]
    redirect = (tree / "docs/index.html").read_text()
    # The /docs/ stub forwards query and hash.
    assert (
        'location.replace("/docs/stable/" + location.search + location.hash)'
        in redirect
    )
    assert 'content="0; url=/docs/stable/"' in redirect
    assert (tree / "pagefind/stub.txt").is_file()
    for kept in (
        "CNAME",
        ".nojekyll",
        "old.html",
        "docs/v1x/v1.4.2/index.html",
        "docs/v1x/versions.json",
    ):
        assert (tree / kept).exists()
    # The build's site root is not published by docs.
    assert not (tree / "index.html").exists()


def test_docs_replaces_existing_version_dir(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    first = docs_build(
        tmp_path / "a",
        "v2.0.0",
        {"index.html": page("x"), "stale.html": page("stale")},
    )
    docs(pages_repo, first)
    assert (pages_repo.tree() / "docs/v2.0.0/stale.html").is_file()
    second = docs_build(tmp_path / "b", "v2.0.0", {"index.html": page("y")})
    docs(pages_repo, second)
    tree = pages_repo.tree()
    assert not (tree / "docs/v2.0.0/stale.html").exists()
    assert (tree / "docs/v2.0.0/index.html").read_text() == page("y")


def test_docs_push_retried_after_rejection(
    pages_repo: PagesRepo,
    tmp_path: Path,
    stub_pagefind: None,
    capsys: pytest.CaptureFixture[str],
) -> None:
    build = docs_build(tmp_path, "v2.0.0", {"index.html": page("x")})
    slept: list[float] = []
    calls = {"n": 0}

    def stage(root: Path) -> list[str]:
        calls["n"] += 1
        if calls["n"] == 1:
            pages_repo.push_competitor("competitor.html")
        return stage_docs(root, build, "v2.0.0")

    result = publish(
        pages_repo.checkout,
        "origin",
        PAGES_BRANCH,
        stage,
        "docs: publish v2.0.0 (abcdef123)",
        sleep=slept.append,
    )
    assert result.changed and calls["n"] == 2 and slept == [5]
    tree = pages_repo.tree()
    assert (tree / "competitor.html").is_file()
    assert (tree / "docs/v2.0.0/index.html").is_file()
    out = capsys.readouterr().out
    assert "Push rejected (attempt 1/5), retrying" in out
    # git's own reason follows the headline.
    assert "[rejected]" in out and "fetch first" in out


def test_docs_gives_up_after_five_rejections(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = docs_build(tmp_path, "v2.0.0", {"index.html": page("x")})
    slept: list[float] = []
    calls = {"n": 0}

    def stage(root: Path) -> list[str]:
        calls["n"] += 1
        pages_repo.push_competitor(f"competitor{calls['n']}.html")
        return stage_docs(root, build, "v2.0.0")

    with pytest.raises(ToolError, match=r"(?s)after 5 attempts.*\[rejected\]"):
        publish(
            pages_repo.checkout,
            "origin",
            PAGES_BRANCH,
            stage,
            "docs: publish v2.0.0 (abcdef123)",
            sleep=slept.append,
        )
    assert slept == [5, 10, 15, 20]
    assert pages_repo.message() == "competitor competitor5.html"
    # No unpushed commit is left behind (HEAD is inside the remote history,
    # at the last fetched tip), so a re-run passes the guard.
    run_git(["fetch", "-q", "origin", PAGES_BRANCH], pages_repo.checkout)
    run_git(
        ["merge-base", "--is-ancestor", "HEAD", pages_repo.tip()],
        pages_repo.checkout,
    )
    assert run_git(["status", "--porcelain"], pages_repo.checkout) == ""
    check_pages_checkout(pages_repo.checkout, "origin", PAGES_BRANCH)


def test_site_publish_replaces_root_only(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    docs(pages_repo, docs_build(tmp_path, "v2.0.0", {"index.html": page("d")}))
    before = pages_repo.tree()
    build = site_build(
        tmp_path,
        {"index.html": page("home"), "articles/index.html": page("a")},
    )
    result = publish_site(
        build, pages_repo.checkout, "origin", PAGES_BRANCH, SOURCE_SHA
    )
    assert result.changed and result.pages_sha == pages_repo.tip()
    assert pages_repo.message() == "site: publish abcdef123"
    tree = pages_repo.tree()
    assert not (tree / "old.html").exists()
    assert (tree / "articles/index.html").is_file()
    for kept in ("CNAME", ".nojekyll"):
        assert (tree / kept).read_text() == (before / kept).read_text()
    # docs/ is untouched by a site publish: same files, same contents.
    for sub in ("docs/v2.0.0", "docs/v1x"):
        assert snapshot(tree / sub) == snapshot(before / sub)
    assert (tree / "docs/stable").readlink() == Path("v2.0.0")
    assert (tree / "pagefind/stub.txt").is_file()


def test_site_unchanged_is_not_an_error(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    build = site_build(tmp_path, {"index.html": page("home")})
    first = publish_site(
        build, pages_repo.checkout, "origin", PAGES_BRANCH, SOURCE_SHA
    )
    second = publish_site(
        build, pages_repo.checkout, "origin", PAGES_BRANCH, SOURCE_SHA
    )
    assert first.changed and not second.changed
    assert second.pages_sha == first.pages_sha == pages_repo.tip()


def test_check_checkout_allows_package_inside(tmp_path: Path) -> None:
    # `tag` runs against the source checkout, which holds this package when
    # it is used uninstalled (CMake, PYTHONPATH); only the build matters.
    tools = Path(__file__).resolve().parents[1]
    assert check_checkout(tools) == tools
    with pytest.raises(UsageError, match="does not exist"):
        check_checkout(tmp_path / "missing")
    (tmp_path / "pages").mkdir()
    with pytest.raises(UsageError, match="inside the Pages checkout"):
        check_checkout(tmp_path / "pages", tmp_path / "pages" / "build")


def test_rsync_tree_excludes_and_deletes(tmp_path: Path) -> None:
    src, dst = tmp_path / "src", tmp_path / "dst"
    write(src / "a", "new\n")
    write(src / "sub" / "b", "b\n")
    write(dst / "a", "old\n")
    write(dst / "stale", "stale\n")
    write(dst / "docs" / "x", "x\n")
    write(dst / "pagefind" / "y", "y\n")
    rsync_tree(src, dst, SITE_EXCLUDES)
    assert (dst / "a").read_text() == "new\n"
    assert (dst / "sub" / "b").is_file()
    assert not (dst / "stale").exists()
    assert (dst / "docs" / "x").is_file() and (dst / "pagefind" / "y").is_file()


def test_cli_publish_outputs(
    pages_repo: PagesRepo,
    tmp_path: Path,
    stub_pagefind: None,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    target = tmp_path / "gh-out"
    monkeypatch.setenv("GITHUB_OUTPUT", str(target))
    build = site_build(tmp_path, {"index.html": page("home")})
    args = [
        "publish",
        "site",
        "--build",
        str(build),
        "--checkout",
        str(pages_repo.checkout),
        "--branch",
        PAGES_BRANCH,
        "--source-sha",
        SOURCE_SHA,
    ]
    assert main(args) == 0
    tip = pages_repo.tip()
    assert f"pages_sha={tip}\nchanged=true\n" in capsys.readouterr().out
    assert main(args) == 0  # the same site again
    assert f"pages_sha={tip}\nchanged=false\n" in capsys.readouterr().out
    assert target.read_text().count("pages_sha=") == 2
    assert main(["publish"]) == 2
    bad_version = [
        "publish",
        "docs",
        "--build",
        str(build),
        "--version",
        "nope",
        "--checkout",
        str(pages_repo.checkout),
        "--branch",
        PAGES_BRANCH,
        "--source-sha",
        SOURCE_SHA,
    ]
    assert main(bad_version) == 2


def test_publish_real_pagefind(pages_repo: PagesRepo, tmp_path: Path) -> None:
    # The real indexer over a published tree: site pages and docs/stable
    # (the published version) are indexed, docs/v1x is not.
    from tests.test_pagefind import fragments

    docs(
        pages_repo,
        docs_build(tmp_path, "v2.0.0", {"index.html": page("pptwozero")}),
    )
    tree = pages_repo.tree()
    assert (tree / "pagefind" / "pagefind.js").is_file()
    found = fragments(tree / "pagefind")
    text = " ".join(content for _, content in found)
    # docs/stable is indexed as its own URL with the published version's text.
    assert [c for u, c in found if u == "/docs/stable/"] == ["pptwozero."]
    assert "oldpage" in text
    assert "archive" not in text

    publish_site(
        site_build(
            tmp_path,
            {"index.html": page("home"), "articles/a.html": page("pparticle")},
        ),
        pages_repo.checkout,
        "origin",
        PAGES_BRANCH,
        SOURCE_SHA,
    )
    text = " ".join(c for _, c in fragments(pages_repo.tree() / "pagefind"))
    assert "pparticle" in text and "pptwozero" in text
    assert "oldpage" not in text


def test_docs_publish_creates_docs_dir(
    pages_repo: PagesRepo, tmp_path: Path, stub_pagefind: None
) -> None:
    # A Pages branch without any docs/ yet (seeded with the site only).
    seed = pages_repo.tree()
    run_git(["rm", "-rq", "docs"], seed)
    run_git(["commit", "-q", "-m", "no docs yet"], seed)
    run_git(["push", "-q", "origin", f"HEAD:{PAGES_BRANCH}"], seed)
    build = docs_build(tmp_path, "v2.0.0", {"index.html": page("x")})
    result = docs(pages_repo, build)
    assert result.changed
    tree = pages_repo.tree()
    assert (tree / "docs/v2.0.0/index.html").is_file()
    assert (tree / "docs/stable").readlink() == Path("v2.0.0")
