# SPDX-License-Identifier: LGPL-3.0-or-later
"""Site assembly: the two Sphinx builds, versions, Pagefind, build state."""

import json
import sys
from pathlib import Path

import pytest
from jps_site_tooling import pagefind, site
from jps_site_tooling.cli import main
from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.site import BuildOptions
from jps_site_tooling.versions import dev_entries


def make_source(tmp_path: Path) -> Path:
    """Create a jupedsim.org-like source folder with both conf.py files.

    Args:
        tmp_path: Parent directory.

    Returns:
        The source folder.
    """
    source = tmp_path / "jupedsim.org"
    for project in ("site", "docs"):
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


def fake_build_index(root: Path) -> Path:
    """Stand in for ``pagefind.build_index``."""
    (root / "pagefind").mkdir(exist_ok=True)
    (root / "pagefind" / "stub.txt").write_text("stub\n")
    return root / "pagefind"


@pytest.fixture
def stubs(monkeypatch: pytest.MonkeyPatch) -> FakeSphinx:
    """Stub Sphinx and Pagefind; return the Sphinx recorder."""
    sphinx = FakeSphinx()
    monkeypatch.setattr(site, "run_sphinx", sphinx)
    monkeypatch.setattr(pagefind, "build_index", fake_build_index)
    monkeypatch.setattr(site, "pagefind_available", lambda: True)
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
    plain = site.sphinx_command(
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
    full = site.sphinx_command(
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
        site.build_site(options(tmp_path))
    (source / "docs" / "conf.py").write_text("")
    with pytest.raises(UsageError):
        site.build_site(options(tmp_path, version=""))
    assert stubs.calls == []


def test_only_site(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, only="site")
    assert site.build_site(opts) == opts.out
    assert len(stubs.calls) == 1
    call = stubs.calls[0]
    assert call["source"] == opts.source / "site"
    assert call["out"] == opts.out
    assert call["doctrees"] == tmp_path / "out-cache" / "doctrees-site"
    assert call["parallel"] is False
    assert not (opts.out / "docs").exists()
    assert not (opts.out / "pagefind").exists()


def test_only_site_leaves_docs_alone(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, only="site")
    for rel, text in (
        ("docs/v1.0.0/index.html", "old docs\n"),
        ("docs/versions.json", "[]\n"),
        ("pagefind/old.js", "old index\n"),
    ):
        (opts.out / rel).parent.mkdir(parents=True, exist_ok=True)
        (opts.out / rel).write_text(text)
    site.build_site(opts)
    assert (opts.out / "docs/v1.0.0/index.html").read_text() == "old docs\n"
    assert (opts.out / "docs/versions.json").read_text() == "[]\n"
    assert (opts.out / "pagefind/old.js").read_text() == "old index\n"


def test_versions_release_vs_dev(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, only="docs")
    site.build_site(opts)
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
        tmp_path, only="docs", version="dev", out=tmp_path / "dev-out"
    )
    site.build_site(dev)
    assert json.loads(
        (dev.out / "docs/versions.json").read_text()
    ) == dev_entries("dev")
    assert not (dev.out / "docs" / "stable").exists()


def test_all_builds_pagefind(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path)
    site.build_site(opts)
    assert [c["what"] for c in stubs.calls] == [
        "site build (site/)",
        "docs build (docs/)",
    ]
    assert (opts.out / "pagefind" / "stub.txt").is_file()


def test_pagefind_unavailable_warns(
    tmp_path: Path,
    stubs: FakeSphinx,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.setattr(site, "pagefind_available", lambda: False)
    opts = options(tmp_path)
    site.build_site(opts)
    assert not (opts.out / "pagefind").exists()
    out = capsys.readouterr().out
    assert "warning: " in out and "pagefind" in out


def test_jupyter_execute_parked_even_when_docs_build_fails(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    sphinx = FakeSphinx(fail_docs=True)
    monkeypatch.setattr(site, "run_sphinx", sphinx)
    monkeypatch.setattr(site, "pagefind_available", lambda: False)
    opts = options(tmp_path, only="docs")
    with pytest.raises(ToolError, match="docs build"):
        site.build_site(opts)
    assert not (opts.out / "docs" / "jupyter_execute").exists()
    assert (tmp_path / "out-cache" / "jupyter_execute-docs" / "x.png").is_file()


def test_jupyter_execute_restored_before_docs_build(
    tmp_path: Path, stubs: FakeSphinx
) -> None:
    cache = tmp_path / "out-cache" / "jupyter_execute-docs"
    cache.mkdir(parents=True)
    (cache / "old.png").write_bytes(b"old")
    opts = options(tmp_path, only="docs")
    site.build_site(opts)
    assert [p.name for p in stubs.seen_during_docs] == ["old.png"]
    assert not (opts.out / "docs" / "jupyter_execute").exists()
    assert sorted(p.name for p in cache.iterdir()) == ["old.png", "x.png"]


def test_doctrees_removed_from_out(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path)
    for rel in (".doctrees/env.pickle", "docs/v2.0.0/.doctrees/env.pickle"):
        (opts.out / rel).parent.mkdir(parents=True, exist_ok=True)
        (opts.out / rel).write_text("stale")
    site.build_site(opts)
    assert not (opts.out / ".doctrees").exists()
    assert not (opts.out / "docs/v2.0.0/.doctrees").exists()


def test_clean_removes_out_and_cache(tmp_path: Path, stubs: FakeSphinx) -> None:
    opts = options(tmp_path, only="site")
    (opts.out / "stale").mkdir(parents=True)
    (opts.out / "stale" / "f.html").write_text("stale")
    cache = tmp_path / "out-cache"
    cache.mkdir()
    (cache / "stale.txt").write_text("stale")
    site.build_site(opts)
    assert (opts.out / "stale" / "f.html").exists() and (
        cache / "stale.txt"
    ).exists()
    site.build_site(options(tmp_path, only="site", clean=True))
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
    from jps_site_tooling import out as out_module

    site.build_site(options(tmp_path, only="site"))
    assert "serve with: python -m http.server" in capsys.readouterr().out
    monkeypatch.setenv("GITHUB_ACTIONS", "true")
    out_module.running_in_github_action.cache_clear()
    try:
        site.build_site(options(tmp_path, only="site"))
        assert "serve with" not in capsys.readouterr().out
    finally:
        out_module.running_in_github_action.cache_clear()


def test_real_sphinx_build(tmp_path: Path) -> None:
    project = tmp_path / "proj"
    project.mkdir()
    (project / "conf.py").write_text("project = 't'\nextensions = []\n")
    (project / "index.rst").write_text("Title\n=====\n\nHello.\n")
    out = tmp_path / "html"
    site.run_sphinx(
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
        site.run_sphinx(
            project,
            tmp_path / "html2",
            tmp_path / "dt2",
            notebooks=True,
            parallel=False,
            strict=True,
            what="test build",
        )


def test_cli_build_site(tmp_path: Path, stubs: FakeSphinx) -> None:
    source = make_source(tmp_path)
    out = tmp_path / "out"
    rc = main(
        [
            "build-site",
            "--source",
            str(source),
            "--out",
            str(out),
            "--version",
            "v1.0.0",
            "--no-notebooks",
            "--strict",
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
                "build-site",
                "--source",
                str(source),
                "--out",
                str(out),
                "--version",
                "dev",
                "--only",
                "nope",
            ]
        )
        == 2
    )


def test_version_optional_for_site_only(
    tmp_path: Path, stubs: FakeSphinx
) -> None:
    # A site-only build involves no docs version (site.yml passes none).
    source = make_source(tmp_path)
    out = tmp_path / "out"
    base = ["build-site", "--source", str(source), "--out", str(out)]
    assert main([*base, "--only", "site"]) == 0
    assert (out / "index.html").is_file()
    assert main([*base, "--only", "docs"]) == 2
    assert main(base) == 2
    with pytest.raises(UsageError, match="docs version"):
        site.build_site(options(tmp_path, version="", only="all"))
    site.build_site(options(tmp_path, version="", only="site"))
