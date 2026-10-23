# SPDX-License-Identifier: LGPL-3.0-or-later
"""docs/versions.json and the docs/stable symlink."""

import json
import os
from pathlib import Path

import pytest
from jps_site_tooling.cli import main
from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.versions import (
    PREFIX,
    VersionsResult,
    collect_versions,
    dev_entries,
    entries,
    write_dev_versions,
    write_versions,
)

ROOT_JSON = '[{"name": "v1.4.2", "url": "https://www.jupedsim.org/v1.4.2/"}]\n'


def make_docs(
    tmp_path: Path,
    versions: tuple[str, ...] = ("v2.0.0", "v2.1.0"),
    v1x: tuple[str, ...] | None = ("v1.4.1", "v1.4.2"),
    stub_stable: bool = False,
) -> Path:
    """Create a gh-pages-like tree for the tests.

    Args:
        tmp_path: Root of the tree; gets a frozen root ``versions.json``.
        versions: Version directories to create under ``docs/``.
        v1x: Versions inside ``docs/v1x/``; None for no ``v1x`` directory.
        stub_stable: Whether to create a plain ``docs/stable`` directory
            holding a stub page.

    Returns:
        The ``docs/`` directory.
    """
    (tmp_path / "versions.json").write_text(ROOT_JSON)
    docs = tmp_path / "docs"
    for v in versions:
        (docs / v).mkdir(parents=True)
        (docs / v / "index.html").write_text(f"<h1>{v}</h1>\n")
    if v1x is not None:
        (docs / "v1x").mkdir(parents=True, exist_ok=True)
        (docs / "v1x" / "versions.json").write_text(ROOT_JSON)
        for v in v1x:
            (docs / "v1x" / v).mkdir()
            (docs / "v1x" / v / "index.html").write_text("old\n")
    if stub_stable:
        (docs / "stable").mkdir()
        (docs / "stable" / "index.html").write_text("stub\n")
    return docs


def expected_entries(stable: str, versions: list[str]) -> list[dict]:
    """Build the expected ``versions.json`` content.

    Args:
        stable: The version ``stable`` points at.
        versions: All versions, newest first.

    Returns:
        The list of entries in file order.
    """
    out: list[dict] = [
        {
            "name": "stable",
            "version": stable,
            "url": f"{PREFIX}stable/",
            "preferred": True,
        }
    ]
    out += [{"name": v, "version": v, "url": f"{PREFIX}{v}/"} for v in versions]
    return out


def test_order_is_numeric_desc(tmp_path: Path) -> None:
    docs = make_docs(tmp_path, ("v2.9.0", "v2.10.0", "v2.0.0"), v1x=None)
    assert collect_versions(docs) == ["v2.10.0", "v2.9.0", "v2.0.0"]


def test_only_release_directories_count(tmp_path: Path) -> None:
    docs = make_docs(tmp_path, ("v2.0.0",), stub_stable=True)
    for name in ("dev", "v2.0.0-rc1"):
        (docs / name).mkdir()
    (docs / "v3.0.0").write_text("a file, not a directory\n")
    assert collect_versions(docs) == ["v2.0.0"]


def test_entries_release_layout(tmp_path: Path) -> None:
    docs = make_docs(tmp_path)
    assert entries(docs) == expected_entries("v2.1.0", ["v2.1.0", "v2.0.0"])
    assert "preferred" not in entries(docs)[-1]


def test_v1x_never_appears(tmp_path: Path) -> None:
    # Whatever docs/v1x holds (versions, an own versions.json) is ignored.
    for v1x in (None, (), ("v1.4.1", "v1.4.2"), ("v3.0.0",)):
        d = tmp_path / str(len(str(v1x)))
        d.mkdir()
        docs = make_docs(d, v1x=v1x)
        assert entries(docs) == expected_entries("v2.1.0", ["v2.1.0", "v2.0.0"])
        assert "v1x" not in json.dumps(entries(docs))


def test_write_versions_creates_json_and_symlink(tmp_path: Path) -> None:
    docs = make_docs(tmp_path)
    result = write_versions(docs)
    assert result == VersionsResult("v2.1.0", True, True)
    assert result.changed
    assert (docs / "stable").is_symlink()
    assert str((docs / "stable").readlink()) == "v2.1.0"
    assert (docs / "versions.json").read_text() == (
        json.dumps(entries(docs), indent=2) + "\n"
    )
    assert (tmp_path / "versions.json").read_text() == ROOT_JSON
    assert (docs / "v1x" / "versions.json").read_text() == ROOT_JSON
    assert not (docs / "v1x" / "stable").exists()


def test_stable_directory_replaced_by_symlink(tmp_path: Path) -> None:
    docs = make_docs(tmp_path, stub_stable=True)
    result = write_versions(docs)
    assert result.link_changed
    assert (docs / "stable").is_symlink()
    assert str((docs / "stable").readlink()) == "v2.1.0"
    assert not (docs / "v2.1.0" / "stub").exists()


def test_stable_file_replaced_by_symlink(tmp_path: Path) -> None:
    docs = make_docs(tmp_path)
    (docs / "stable").write_text("not a link\n")
    write_versions(docs)
    assert (docs / "stable").is_symlink()


def test_stable_repointed_only_when_target_differs(tmp_path: Path) -> None:
    docs = make_docs(tmp_path)
    (docs / "stable").symlink_to("v2.0.0", target_is_directory=True)
    assert write_versions(docs).link_changed is True
    assert str((docs / "stable").readlink()) == "v2.1.0"
    assert write_versions(docs).link_changed is False


def test_second_run_changes_nothing(tmp_path: Path) -> None:
    docs = make_docs(tmp_path)
    write_versions(docs)
    before = os.stat(docs / "versions.json").st_mtime_ns
    result = write_versions(docs)
    assert result == VersionsResult("v2.1.0", False, False)
    assert not result.changed
    assert os.stat(docs / "versions.json").st_mtime_ns == before


def test_no_versions_is_usage_error(tmp_path: Path) -> None:
    docs = make_docs(tmp_path, versions=(), v1x=("v1.4.2",))
    with pytest.raises(UsageError):
        write_versions(docs)
    with pytest.raises(UsageError):
        write_versions(tmp_path / "missing")


def test_versions_json_obstacle_is_tool_error(tmp_path: Path) -> None:
    docs = make_docs(tmp_path)
    (docs / "versions.json").mkdir()
    with pytest.raises(ToolError):
        write_versions(docs)
    assert not (docs / "stable").exists()
    (docs / "versions.json").rmdir()
    (docs / "versions.json").symlink_to("v2.0.0/index.html")
    with pytest.raises(ToolError):
        write_versions(docs)
    assert not (docs / "stable").exists()


def test_dev_entries_and_writer(tmp_path: Path) -> None:
    docs = make_docs(tmp_path, versions=("dev",), v1x=None)
    assert dev_entries("dev") == [
        {
            "name": "dev",
            "version": "dev",
            "url": "/docs/dev/",
            "preferred": True,
        }
    ]
    result = write_dev_versions(docs, "dev")
    assert result == VersionsResult(None, True, False)
    assert json.loads((docs / "versions.json").read_text()) == dev_entries(
        "dev"
    )
    assert not (docs / "stable").exists()
    assert write_dev_versions(docs, "dev") == VersionsResult(None, False, False)


def test_cli_versions(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    docs = make_docs(tmp_path)
    assert main(["versions", str(docs)]) == 0
    assert str((docs / "stable").readlink()) == "v2.1.0"
    assert capsys.readouterr().out == ""
    assert main(["versions", str(docs), "--dev", "dev"]) == 0
    assert json.loads((docs / "versions.json").read_text()) == dev_entries(
        "dev"
    )
    assert main(["versions", str(tmp_path / "nope")]) == 2
    (docs / "versions.json").unlink()
    (docs / "versions.json").mkdir()
    assert main(["versions", str(docs)]) == 1
