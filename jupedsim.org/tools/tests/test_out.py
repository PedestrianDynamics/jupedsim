# SPDX-License-Identifier: LGPL-3.0-or-later
"""Message output: GitHub annotations inside Actions, plain text elsewhere."""

import io
from pathlib import Path

import pytest
from jps_site_tooling import out
from jps_site_tooling.errors import ToolError


def test_running_in_github_action(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    assert out.running_in_github_action() is False
    out.running_in_github_action.cache_clear()
    monkeypatch.setenv("GITHUB_ACTIONS", "true")
    assert out.running_in_github_action() is True
    out.running_in_github_action.cache_clear()
    monkeypatch.setenv("GITHUB_ACTIONS", "1")
    assert out.running_in_github_action() is False


def test_escape_annotation() -> None:
    assert out.escape_annotation("a%b\r\nc\nd") == "a%25b%0D%0Ac%0Ad"
    # % first, so an already escaped sequence is not double-decoded.
    assert out.escape_annotation("%0A") == "%250A"


@pytest.mark.parametrize(
    ("writer", "command", "prefix"),
    [
        (out.write_error, "error", "error: "),
        (out.write_warning, "warning", "warning: "),
        (out.write_info, "notice", ""),
    ],
)
def test_writers_in_github_actions(
    monkeypatch: pytest.MonkeyPatch, writer, command: str, prefix: str
) -> None:
    monkeypatch.setenv("GITHUB_ACTIONS", "true")
    buf = io.StringIO()
    writer("line one\nline two 100%", file=buf)
    assert buf.getvalue() == f"::{command}::line one%0Aline two 100%25\n"


@pytest.mark.parametrize(
    ("writer", "command", "prefix"),
    [
        (out.write_error, "error", "error: "),
        (out.write_warning, "warning", "warning: "),
        (out.write_info, "notice", ""),
    ],
)
def test_writers_outside_github_actions(
    monkeypatch: pytest.MonkeyPatch, writer, command: str, prefix: str
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    buf = io.StringIO()
    writer("line one\nline two 100%", file=buf)
    assert buf.getvalue() == f"{prefix}line one\nline two 100%\n"


def test_default_stream_is_stdout(
    monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    monkeypatch.delenv("GITHUB_ACTIONS", raising=False)
    out.write_error("boom")
    out.write_info("fyi")
    captured = capsys.readouterr()
    assert captured.out == "error: boom\nfyi\n"
    assert captured.err == ""


def test_write_output_stdout_and_github_output(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.delenv("GITHUB_OUTPUT", raising=False)
    out.write_output("pages_sha", "abc")
    assert capsys.readouterr().out == "pages_sha=abc\n"
    target = tmp_path / "out.txt"
    target.write_text("earlier=1\n")
    monkeypatch.setenv("GITHUB_OUTPUT", str(target))
    out.write_output("changed", "true")
    assert capsys.readouterr().out == "changed=true\n"
    assert target.read_text() == "earlier=1\nchanged=true\n"


def test_write_output_rejects_line_breaks(
    monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    monkeypatch.delenv("GITHUB_OUTPUT", raising=False)
    for bad in ("a\nb", "a\rb"):
        with pytest.raises(ValueError, match="line break"):
            out.write_output("k", bad)
    assert capsys.readouterr().out == ""


def test_write_output_unwritable_github_output_is_tool_error(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setenv("GITHUB_OUTPUT", str(tmp_path / "missing" / "out"))
    with pytest.raises(ToolError, match="GITHUB_OUTPUT"):
        out.write_output("k", "v")
