# SPDX-License-Identifier: LGPL-3.0-or-later
"""The Pagefind search index over the site root plus docs/stable."""

import gzip
import importlib.metadata
import json
import shutil
import tempfile
from pathlib import Path

import pytest
from jps_site_tooling import __version__, pagefind
from jps_site_tooling.cli import main
from jps_site_tooling.errors import ToolError, UsageError

WORDS = {
    "index.html": "sitehome",
    "notes/n.html": "sitenote",
    "docs/v2.0.0/index.html": "twozero",
    "docs/v2.1.0/index.html": "twoone",
    "docs/v1x/v1.4.2/index.html": "archiveword",
}


def _page(word: str) -> str:
    """Build a minimal indexable page carrying one unique word.

    Args:
        word: The word Pagefind will find in the page body.

    Returns:
        The HTML document.
    """
    return (
        '<!doctype html><html lang="en"><head><title>'
        f"{word}</title></head><body><main data-pagefind-body>"
        f"<h1>{word}</h1><p>{word} page</p></main></body></html>\n"
    )


def make_tree(tmp_path: Path, *, stable: str | None = "v2.1.0") -> Path:
    """Create a Pages-shaped tree with site pages, docs and an old index.

    Args:
        tmp_path: Parent directory.
        stable: Target of the ``docs/stable`` symlink; ``"stub"`` creates a
            plain directory with a stub page instead; None creates nothing.

    Returns:
        The tree root.
    """
    root = tmp_path / "root"
    for rel, word in WORDS.items():
        path = root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(_page(word))
    (root / ".git").mkdir()
    (root / ".git" / "HEAD").write_text("ref: refs/heads/gh-pages\n")
    (root / "pagefind").mkdir()
    (root / "pagefind" / "old.js").write_text("old index\n")
    if stable == "stub":
        (root / "docs" / "stable").mkdir()
        (root / "docs" / "stable" / "index.html").write_text(_page("stubpage"))
    elif stable is not None:
        (root / "docs" / "stable").symlink_to(stable, target_is_directory=True)
    return root


def relative_files(directory: Path) -> set[str]:
    """List all files below a directory as POSIX paths relative to it.

    Args:
        directory: The directory to walk.

    Returns:
        The relative paths of every regular file.
    """
    return {
        str(p.relative_to(directory))
        for p in directory.rglob("*")
        if p.is_file()
    }


def fragments(index: Path) -> list[tuple[str, str]]:
    """Read the indexed pages back out of a Pagefind index.

    Args:
        index: The ``pagefind/`` directory.

    Returns:
        ``(url, content)`` per indexed page, sorted by URL.
    """
    out = []
    for f in sorted((index / "fragment").glob("*.pf_fragment")):
        data = gzip.decompress(f.read_bytes())
        frag = json.loads(data[data.index(b"{") :])
        out.append((frag["url"], frag["content"]))
    return sorted(out)


def fake_pagefind(site: Path, output: Path) -> int:
    """Stand in for Pagefind: record the staged files, write pagefind.js.

    Args:
        site: The staged tree Pagefind would index.
        output: Where the index goes.

    Returns:
        The number of staged HTML pages, as the real indexer would report.
    """
    output.mkdir(parents=True, exist_ok=True)
    (output / "pagefind.js").write_text("fake\n")
    staged = sorted(relative_files(site))
    (output / "staged.txt").write_text("\n".join(staged) + "\n")
    return sum(1 for f in staged if f.endswith(".html"))


def test_stamp_format() -> None:
    lines = pagefind.stamp().splitlines()
    assert lines[0] == f"jps_site_tooling {__version__}"
    assert lines[1] == f"pagefind {importlib.metadata.version('pagefind_bin')}"
    assert pagefind.stamp().endswith("\n")


def test_staging_excludes_root_entries(tmp_path: Path) -> None:
    # .git, docs/ and the previous index are skipped; every other root
    # entry is site content and indexed.
    root = make_tree(tmp_path, stable=None)
    (root / "articles").mkdir()
    (root / "articles" / "a.html").write_text(_page("article"))
    into = tmp_path / "staged"
    pagefind.stage(root, into)
    assert relative_files(into) == {
        "index.html",
        "notes/n.html",
        "articles/a.html",
    }


def test_staging_follows_stable_symlink_only(tmp_path: Path) -> None:
    root = make_tree(tmp_path)
    into = tmp_path / "staged"
    pagefind.stage(root, into)
    files = relative_files(into)
    assert files == {"index.html", "notes/n.html", "docs/stable/index.html"}
    assert "twoone" in (into / "docs" / "stable" / "index.html").read_text()
    assert not (into / "docs" / "stable").is_symlink()


def test_staging_ignores_stable_stub_dir(tmp_path: Path) -> None:
    root = make_tree(tmp_path, stable="stub")
    into = tmp_path / "staged"
    pagefind.stage(root, into)
    assert not any(f.startswith("docs/") for f in relative_files(into))


def test_build_index_swaps_in_new_index(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(pagefind, "run_pagefind", fake_pagefind)
    root = make_tree(tmp_path)
    index = pagefind.build_index(root)
    assert index == root / "pagefind"
    assert (index / "pagefind.js").is_file()
    assert not (index / "old.js").exists()
    assert (index / "build-stamp.txt").read_text() == pagefind.stamp()
    staged = set((index / "staged.txt").read_text().split())
    assert staged == {"index.html", "notes/n.html", "docs/stable/index.html"}


def test_failed_build_keeps_previous_index(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = make_tree(tmp_path)

    def boom(site: Path, output: Path) -> None:
        """Fail like a crashing binary."""
        raise ToolError("pagefind exploded")

    monkeypatch.setattr(pagefind, "run_pagefind", boom)
    with pytest.raises(ToolError, match="exploded"):
        pagefind.build_index(root)
    assert (root / "pagefind" / "old.js").read_text() == "old index\n"
    assert relative_files(root / "pagefind") == {"old.js"}
    # An indexer run that writes no pagefind.js is a failure too.
    monkeypatch.undo()
    monkeypatch.setattr(pagefind, "_index_directory", lambda site, out, cfg: 0)
    with pytest.raises(ToolError, match="pagefind.js"):
        pagefind.build_index(root)
    assert relative_files(root / "pagefind") == {"old.js"}


def test_indexer_failure_is_tool_error(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = make_tree(tmp_path)

    def broken(site: Path, out: Path, cfg: object) -> int:
        """Fail like the Pagefind service raising mid-run."""
        raise RuntimeError("service died")

    monkeypatch.setattr(pagefind, "_index_directory", broken)
    with pytest.raises(ToolError, match="service died"):
        pagefind.build_index(root)
    assert relative_files(root / "pagefind") == {"old.js"}


def test_build_index_logs_page_count(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    monkeypatch.setattr(pagefind, "run_pagefind", fake_pagefind)
    pagefind.build_index(make_tree(tmp_path))
    assert "indexed 3 pages" in capsys.readouterr().out


def test_old_index_symlink_is_replaced_by_a_directory(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(pagefind, "run_pagefind", fake_pagefind)
    root = make_tree(tmp_path)
    elsewhere = tmp_path / "elsewhere"
    elsewhere.mkdir()
    (elsewhere / "keep.txt").write_text("keep\n")
    shutil.rmtree(root / "pagefind")
    (root / "pagefind").symlink_to(elsewhere, target_is_directory=True)
    index = pagefind.build_index(root)
    assert not index.is_symlink() and index.is_dir()
    assert (index / "pagefind.js").is_file()
    assert relative_files(elsewhere) == {"keep.txt"}


def test_unremovable_old_index_is_tool_error(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(pagefind, "run_pagefind", fake_pagefind)
    root = make_tree(tmp_path)
    old = root / "pagefind"
    (old / "fragment").mkdir()
    (old / "fragment" / "f.pf_fragment").write_bytes(b"x")
    (old / "fragment").chmod(0o500)
    try:
        with pytest.raises(ToolError, match="previous index"):
            pagefind.build_index(root)
        # Nothing nested into the old index, no misplaced pagefind.js.
        assert not (old / "pagefind").exists()
        assert not (old / "pagefind.js").exists()
    finally:
        (old / "fragment").chmod(0o700)


def test_dangling_stable_is_tool_error(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(pagefind, "run_pagefind", fake_pagefind)
    root = make_tree(tmp_path, stable="v9.9.9")
    with pytest.raises(ToolError, match="dangling"):
        pagefind.build_index(root)
    assert relative_files(root / "pagefind") == {"old.js"}


def test_versions_works_without_pagefind(tmp_path: Path) -> None:
    # The pagefind imports must be lazy: other subcommands run in
    # environments without Pagefind (build-site --only docs).
    import os
    import subprocess
    import sys

    code = (
        "import sys; sys.modules['pagefind'] = None; "
        "sys.modules['pagefind_bin'] = None; "
        "from jps_site_tooling.cli import main; "
        "raise SystemExit(main(['versions', '--help']))"
    )
    env = dict(os.environ, PYTHONPATH=str(Path(__file__).resolve().parents[1]))
    result = subprocess.run(
        [sys.executable, "-c", code], env=env, capture_output=True, text=True
    )
    assert result.returncode == 0, result.stderr


def test_missing_pagefind_distribution_is_tool_error(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def gone(name: str) -> str:
        """Fail like a venv without pagefind_bin."""
        raise importlib.metadata.PackageNotFoundError(name)

    monkeypatch.setattr(pagefind.importlib.metadata, "version", gone)
    with pytest.raises(ToolError, match="pagefind_bin"):
        pagefind.stamp()


def test_root_not_a_directory(tmp_path: Path) -> None:
    with pytest.raises(UsageError):
        pagefind.build_index(tmp_path / "nope")


def test_temp_dirs_are_cleaned(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    created: list[str] = []
    real = tempfile.TemporaryDirectory

    class Recording(real):  # type: ignore[type-arg,misc]
        """TemporaryDirectory that remembers its path."""

        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            created.append(self.name)

    monkeypatch.setattr(pagefind.tempfile, "TemporaryDirectory", Recording)
    monkeypatch.setattr(pagefind, "run_pagefind", fake_pagefind)
    pagefind.build_index(make_tree(tmp_path))
    assert created
    assert not any(Path(name).exists() for name in created)


def test_cli_pagefind(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    monkeypatch.setattr(pagefind, "run_pagefind", fake_pagefind)
    root = make_tree(tmp_path)
    assert main(["pagefind", str(root)]) == 0
    assert (root / "pagefind" / "pagefind.js").is_file()
    assert "indexed 3 pages" in capsys.readouterr().out
    assert main(["pagefind"]) == 2
    assert main(["pagefind", str(tmp_path / "nope")]) == 2


def test_real_pagefind_build(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    # Run from an unrelated cwd: the index must land only where asked, not
    # in the API's default ./pagefind of the current directory.
    cwd = tmp_path / "cwd"
    cwd.mkdir()
    monkeypatch.chdir(cwd)
    root = make_tree(tmp_path)
    staged = tmp_path / "staged"
    pagefind.stage(root, staged)
    assert pagefind.run_pagefind(staged, tmp_path / "direct") == 3
    assert (tmp_path / "direct" / "pagefind.js").is_file()
    assert not (cwd / "pagefind").exists()
    index = pagefind.build_index(root)
    assert not (cwd / "pagefind").exists()
    found = fragments(index)
    urls = {url for url, _ in found}
    text = " ".join(content for _, content in found)
    assert {"/", "/notes/n.html", "/docs/stable/"} <= urls
    for word in ("sitehome", "sitenote", "twoone"):
        assert word in text
    for word in (
        "twozero",
        "archiveword",
        "old index",
    ):
        assert word not in text
    assert (index / "build-stamp.txt").read_text() == pagefind.stamp()
