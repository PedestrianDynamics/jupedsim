# SPDX-License-Identifier: LGPL-3.0-or-later
"""Website assembly: the two Sphinx builds, versions, Pagefind, build state."""

import json
import sys
from pathlib import Path

import pytest
from jps_website_tooling import pagefind, website
from jps_website_tooling.cli import main
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.versions import dev_entries
from jps_website_tooling.website import BuildOptions


def make_source(tmp_path: Path) -> Path:
    """Create a jupedsim.org-like source folder with both conf.py files.

    Args:
        tmp_path: Parent directory.

    Returns:
        The source folder.
    """
    source = tmp_path / "jupedsim.org"
    for project in ("www", "docs"):
        (source / project).mkdir(parents=True)
        (source / project / "conf.py").write_text(f"project = '{project}'\n")
    return source


class FakeSphinx:
    """Stand in for ``run_sphinx``: record calls, write a page like Sphinx."""

    def __init__(self, *, fail_docs: bool = False) -> None:
        self.calls: list[dict[str, object]] = []
        self.fail_docs = fail_docs
        self.seen_during_docs: list[Path] = []

    def __call__(
        self,
        source: Path,
        out: Path,
        doctrees: Path,
        *,
        notebooks: bool,
        parallel: bool,
        strict: bool,
        what: str,
    ) -> None:
        """Record the call; emulate myst-nb's jupyter_execute for docs."""
        self.calls.append(
            dict(
                source=source,
                out=out,
                doctrees=doctrees,
                notebooks=notebooks,
                parallel=parallel,
                strict=strict,
                what=what,
            )
        )
        out.mkdir(parents=True, exist_ok=True)
        (out / "index.html").write_text(f"<h1>{source.name}</h1>\n")
        if source.name == "docs":
            executed = out.parent / "jupyter_execute"
            self.seen_during_docs = (
                sorted(executed.glob("*")) if executed.exists() else []
            )
            executed.mkdir(exist_ok=True)
            (executed / "x.png").write_bytes(b"png")
            if self.fail_docs:
                raise ToolError("docs build failed with exit code 2")


INDEXED: list[tuple[Path, Path | None]] = []


def fake_build_index(root: Path, *, docs: Path | None = None) -> Path:
    """Stand in for ``pagefind.build_index``; records the docs argument."""
    INDEXED.append((root, docs))
    (root / "pagefind").mkdir(exist_ok=True)
    (root / "pagefind" / "stub.txt").write_text("stub\n")
    return root / "pagefind"


@pytest.fixture
def stubs(monkeypatch: pytest.MonkeyPatch) -> FakeSphinx:
    """Stub Sphinx and Pagefind; return the Sphinx recorder."""
    sphinx = FakeSphinx()
    monkeypatch.setattr(website, "run_sphinx", sphinx)
    monkeypatch.setattr(pagefind, "build_index", fake_build_index)
    monkeypatch.setattr(website, "pagefind_available", lambda: True)
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    return sphinx


def options(tmp_path: Path, **kw: object) -> BuildOptions:
    """Build options for the fixture source and ``<tmp>/out``."""
    base: dict[str, object] = dict(
        source=make_source(tmp_path)
        if not (tmp_path / "jupedsim.org").exists()
        else tmp_path / "jupedsim.org",
        out=tmp_path / "out",
        version="v2.0.0",
    )
    base.update(kw)
    return BuildOptions(**base)  # type: ignore[arg-type]


def test_sphinx_command_flags(tmp_path: Path) -> None:
    src, out, dt = Path("src"), Path("out"), Path("dt")
    plain = website.sphinx_command(
        src, out, dt, notebooks=True, parallel=False, strict=False
    )
    assert plain == [
        sys.executable,
        "-m",
        "sphinx",
        "-T",
        "-b",
        "html",
        "-d",
        "dt",
        "src",
        "out",
    ]
    full = website.sphinx_command(
        src, out, dt, notebooks=False, parallel=True, strict=True
    )
    assert full == [
        sys.executable, "-m", "sphinx", "-T", "-j", "auto", "-W", "--keep-going",
        "-b", "html", "-d", "dt", "src", "out", "-D", "nb_execution_mode=off",
    ]  # fmt: skip


def test_source_validation(tmp_path: Path, stubs: FakeSphinx) -> None:
    source = make_source(tmp_path)
    (source / "docs" / "conf.py").unlink()
    with pytest.raises(UsageError):
        website.build_website(options(tmp_path))
    (source / "docs" / "conf.py").write_text("")
    with pytest.raises(UsageError):
        website.build_website(options(tmp_path, version=""))
    assert stubs.calls == []


def test_only_site(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, target="www")
    assert website.build_website(opts) == opts.out
    assert len(stubs.calls) == 1
    call = stubs.calls[0]
    assert call["source"] == opts.source / "www"
    assert call["out"] == opts.out
    assert call["doctrees"] == tmp_path / "out-cache" / "doctrees-www"
    assert call["parallel"] is False
    assert not (opts.out / "docs").exists()
    assert not (opts.out / "pagefind").exists()


def test_only_site_leaves_docs_alone(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, target="www")
    for rel, text in (
        ("docs/v1.0.0/index.html", "old docs\n"),
        ("docs/versions.json", "[]\n"),
        ("pagefind/old.js", "old index\n"),
    ):
        (opts.out / rel).parent.mkdir(parents=True, exist_ok=True)
        (opts.out / rel).write_text(text)
    website.build_website(opts)
    assert (opts.out / "docs/v1.0.0/index.html").read_text() == "old docs\n"
    assert (opts.out / "docs/versions.json").read_text() == "[]\n"
    assert (opts.out / "pagefind/old.js").read_text() == "old index\n"


def test_versions_release_vs_dev(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, target="docs")
    website.build_website(opts)
    call = stubs.calls[0]
    assert call["source"] == opts.source / "docs"
    assert call["out"] == opts.out / "docs" / "v2.0.0"
    assert call["doctrees"] == tmp_path / "out-cache" / "doctrees-docs"
    assert call["parallel"] is True
    assert str((opts.out / "docs" / "stable").readlink()) == "v2.0.0"
    names = [
        e["name"]
        for e in json.loads((opts.out / "docs/versions.json").read_text())
    ]
    assert names == ["stable", "v2.0.0"]

    dev = options(
        tmp_path, target="docs", version="dev", out=tmp_path / "dev-out"
    )
    website.build_website(dev)
    assert json.loads(
        (dev.out / "docs/versions.json").read_text()
    ) == dev_entries("dev")
    assert not (dev.out / "docs" / "stable").exists()


def test_all_builds_pagefind(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path)
    website.build_website(opts)
    assert [c["what"] for c in stubs.calls] == [
        "www build (www/)",
        "docs build (docs/)",
    ]
    assert (opts.out / "pagefind" / "stub.txt").is_file()


def test_pagefind_unavailable_warns(
    tmp_path: Path,
    stubs: FakeSphinx,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.setattr(website, "pagefind_available", lambda: False)
    opts = options(tmp_path)
    website.build_website(opts)
    assert not (opts.out / "pagefind").exists()
    out = capsys.readouterr().out
    assert "warning: " in out and "pagefind" in out


def test_jupyter_execute_parked_even_when_docs_build_fails(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    sphinx = FakeSphinx(fail_docs=True)
    monkeypatch.setattr(website, "run_sphinx", sphinx)
    monkeypatch.setattr(website, "pagefind_available", lambda: False)
    opts = options(tmp_path, target="docs")
    with pytest.raises(ToolError, match="docs build"):
        website.build_website(opts)
    assert not (opts.out / "docs" / "jupyter_execute").exists()
    assert (tmp_path / "out-cache" / "jupyter_execute-docs" / "x.png").is_file()


def test_jupyter_execute_restored_before_docs_build(
    tmp_path: Path, stubs: FakeSphinx
) -> None:
    cache = tmp_path / "out-cache" / "jupyter_execute-docs"
    cache.mkdir(parents=True)
    (cache / "old.png").write_bytes(b"old")
    opts = options(tmp_path, target="docs")
    website.build_website(opts)
    assert [p.name for p in stubs.seen_during_docs] == ["old.png"]
    assert not (opts.out / "docs" / "jupyter_execute").exists()
    assert sorted(p.name for p in cache.iterdir()) == ["old.png", "x.png"]


def test_doctrees_removed_from_out(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path)
    for rel in (".doctrees/env.pickle", "docs/v2.0.0/.doctrees/env.pickle"):
        (opts.out / rel).parent.mkdir(parents=True, exist_ok=True)
        (opts.out / rel).write_text("stale")
    website.build_website(opts)
    assert not (opts.out / ".doctrees").exists()
    assert not (opts.out / "docs/v2.0.0/.doctrees").exists()


def test_clean_removes_out_and_cache(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, target="www")
    (opts.out / "stale").mkdir(parents=True)
    (opts.out / "stale" / "f.html").write_text("stale")
    cache = tmp_path / "out-cache"
    cache.mkdir()
    (cache / "stale.txt").write_text("stale")
    website.build_website(opts)
    assert (opts.out / "stale" / "f.html").exists() and (
        cache / "stale.txt"
    ).exists()
    website.build_website(options(tmp_path, target="www", clean=True))
    assert (
        not (opts.out / "stale").exists() and not (cache / "stale.txt").exists()
    )
    assert (opts.out / "index.html").is_file()


def test_serve_hint_only_outside_actions(
    tmp_path: Path,
    stubs: FakeSphinx,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    from jps_website_tooling import out as out_module

    website.build_website(options(tmp_path, target="www"))
    assert "serve with: python -m http.server" in capsys.readouterr().out
    monkeypatch.setenv("GITHUB_ACTIONS", "true")
    out_module.running_in_github_action.cache_clear()
    try:
        website.build_website(options(tmp_path, target="www"))
        assert "serve with" not in capsys.readouterr().out
    finally:
        out_module.running_in_github_action.cache_clear()


def test_real_sphinx_build(tmp_path: Path) -> None:
    project = tmp_path / "proj"
    project.mkdir()
    (project / "conf.py").write_text("project = 't'\nextensions = []\n")
    (project / "index.rst").write_text("Title\n=====\n\nHello.\n")
    out = tmp_path / "html"
    website.run_sphinx(
        project,
        out,
        tmp_path / "dt",
        notebooks=True,
        parallel=False,
        strict=True,
        what="test build",
    )
    assert (out / "index.html").is_file()
    # Real Sphinx honours -d: no .doctrees in the output, doctrees in dt.
    assert not (out / ".doctrees").exists()
    assert (tmp_path / "dt" / "environment.pickle").is_file()
    (project / "index.rst").write_text("Broken\n==\n\n:unknownrole:`x`\n")
    with pytest.raises(ToolError, match="test build"):
        website.run_sphinx(
            project,
            tmp_path / "html2",
            tmp_path / "dt2",
            notebooks=True,
            parallel=False,
            strict=True,
            what="test build",
        )


def test_cli_build_website(
    tmp_path: Path, stubs: FakeSphinx, monkeypatch: pytest.MonkeyPatch
) -> None:
    make_source(tmp_path)
    monkeypatch.chdir(tmp_path)
    out = tmp_path / "out"
    rc = main(
        [
            "build-website",
            "--out",
            str(out),
            "--version",
            "v1.0.0",
            "--no-notebooks",
            "--clean",
        ]
    )
    assert rc == 0
    assert all(
        c["strict"] is True and c["notebooks"] is False for c in stubs.calls
    )
    assert (out / "docs" / "v1.0.0" / "index.html").is_file()
    assert (
        main(
            [
                "build-website",
                "--out",
                str(out),
                "--version",
                "dev",
                "--target",
                "nope",
            ]
        )
        == 2
    )


def test_version_optional_for_site_only(
    tmp_path: Path, stubs: FakeSphinx, monkeypatch: pytest.MonkeyPatch
) -> None:
    # A www-only build involves no docs version (www.yml passes none).
    make_source(tmp_path)
    monkeypatch.chdir(tmp_path)
    out = tmp_path / "out"
    base = ["build-website", "--out", str(out)]
    assert main([*base, "--target", "www"]) == 0
    assert (out / "index.html").is_file()
    assert main([*base, "--target", "docs"]) == 2
    assert main(base) == 2
    with pytest.raises(UsageError, match="docs version"):
        website.build_website(options(tmp_path, version="", target="all"))
    website.build_website(options(tmp_path, version="", target="www"))


def test_stable_version_builds_a_directory(
    tmp_path: Path, stubs: FakeSphinx
) -> None:
    # PR previews build with version "stable": the docs land in docs/stable/
    # as a plain directory (no symlink to prune, indexed by Pagefind).
    opts = options(tmp_path, target="docs", version="stable")
    website.build_website(opts)
    stable = opts.out / "docs" / "stable"
    assert stable.is_dir() and not stable.is_symlink()
    assert (stable / "index.html").is_file()
    names = [
        e["name"]
        for e in json.loads((opts.out / "docs/versions.json").read_text())
    ]
    assert names == ["stable"]


def test_index_covers_the_built_docs(tmp_path: Path, stubs: FakeSphinx) -> None:
    # The index of a full build must include the docs it just built, also
    # when docs/stable is a directory (version "stable") and not a symlink.
    INDEXED.clear()
    opts = options(tmp_path, version="stable")
    website.build_website(opts)
    assert INDEXED == [(opts.out, opts.out / "docs" / "stable")]
    INDEXED.clear()
    release = options(tmp_path, version="v2.0.0", out=tmp_path / "rel")
    website.build_website(release)
    assert INDEXED == [(release.out, release.out / "docs" / "v2.0.0")]


def test_strict_by_default_and_lenient(
    tmp_path: Path, stubs: FakeSphinx, monkeypatch: pytest.MonkeyPatch
) -> None:
    assert BuildOptions(source=tmp_path, out=tmp_path / "o").strict is True
    source = make_source(tmp_path)
    monkeypatch.chdir(tmp_path)  # the source is <cwd>/jupedsim.org
    out = tmp_path / "out"
    assert main(["build-website", "--out", str(out), "--target", "www"]) == 0
    assert stubs.calls[-1]["strict"] is True
    assert stubs.calls[-1]["source"] == source / "www"
    assert (
        main(
            ["build-website", "--out", str(out), "--target", "www", "--lenient"]
        )
        == 0
    )
    assert stubs.calls[-1]["strict"] is False
    assert (
        main(["build-website", "--source", str(source), "--out", str(out)]) == 2
    )
    assert main(["build-website", "--out", str(out), "--strict"]) == 2


def test_www_never_executes_notebooks(
    tmp_path: Path, stubs: FakeSphinx
) -> None:
    # www is not tied to a jupedsim version: its notebooks are rendered with
    # their stored outputs, never executed. The docs still execute theirs.
    website.build_website(options(tmp_path, notebooks=True))
    by_what = {c["what"]: c["notebooks"] for c in stubs.calls}
    assert by_what == {"www build (www/)": False, "docs build (docs/)": True}
