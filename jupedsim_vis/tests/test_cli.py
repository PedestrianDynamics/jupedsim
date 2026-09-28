# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for the command line interface and the headless smoke test.

The smoke test is what CI runs to check that the application starts, so it
is exercised here both in process (fast, gives readable failures) and once
through ``python -m jupedsim_vis`` (checks the entry point itself).

None of these tests may create a render tab: ``RenderWidget`` segfaults
without a real OpenGL context, which is why ``--smoke-test`` loads documents
without views and ``--smoke-test-render`` is left to the Xvfb job.
"""

import os
import subprocess
import sys
from pathlib import Path

import pytest
from make_fixture_recording import write_recording

from jupedsim_vis.__main__ import (
    _setup_qt_platform,
    _start_watchdog,
    build_parser,
    main,
)

DATA = Path(__file__).parent / "data"
SIMPLE_WKT = DATA / "simple.wkt"

NO_JUPEDSIM = "Cannot calculate shortest path: jupedsim is not installed."


@pytest.fixture
def no_jupedsim(monkeypatch):
    """``None`` in ``sys.modules`` makes ``import jupedsim`` raise."""
    monkeypatch.setitem(sys.modules, "jupedsim", None)


@pytest.fixture
def recording_v3(tmp_path) -> Path:
    path = tmp_path / "recording_v3.sqlite"
    write_recording(path, version=3, frames=5, agents=4)
    return path


def test_help_shows_the_testing_group() -> None:
    help_text = build_parser().format_help()

    assert "testing:" in help_text
    assert "test the application" in help_text
    assert "--smoke-test" in help_text
    assert "--smoke-test-render" in help_text
    assert "--smoke-test-timeout" in help_text


def test_help_mentions_the_file_options() -> None:
    help_text = build_parser().format_help()

    assert "--wkt" in help_text
    assert "--recording" in help_text


def test_help_exits_successfully(capsys) -> None:
    with pytest.raises(SystemExit) as excinfo:
        main(["--help"])

    assert excinfo.value.code == 0
    assert "--smoke-test-timeout" in capsys.readouterr().out


def test_parser_defaults() -> None:
    args = build_parser().parse_args([])

    assert args.wkt == []
    assert args.recording == []
    assert args.smoke_test is False
    assert args.smoke_test_render is False
    assert args.smoke_test_timeout == 60.0


def test_file_options_are_repeatable_paths() -> None:
    args = build_parser().parse_args(
        ["--wkt", "a.wkt", "--recording", "b.sqlite", "--wkt", "c.wkt"]
    )

    assert args.wkt == [Path("a.wkt"), Path("c.wkt")]
    assert args.recording == [Path("b.sqlite")]


def test_smoke_test_without_files(no_jupedsim, capsys) -> None:
    assert main(["--smoke-test"]) == 0

    out = capsys.readouterr().out
    assert "documents=0 tabs=0" in out
    assert f"platform={os.environ['QT_QPA_PLATFORM']}" in out
    assert "title=jupedsim_vis" in out
    assert out.strip().endswith("smoke-test: OK")


def test_smoke_test_with_wkt(no_jupedsim, capsys) -> None:
    assert main(["--smoke-test", "--wkt", str(SIMPLE_WKT)]) == 0

    out = capsys.readouterr().out
    assert "documents=1 tabs=0" in out
    assert out.strip().endswith("smoke-test: OK")


def test_smoke_test_with_recording(no_jupedsim, recording_v3, capsys) -> None:
    assert main(["--smoke-test", "--recording", str(recording_v3)]) == 0

    out = capsys.readouterr().out
    assert "documents=1 tabs=0" in out
    assert out.strip().endswith("smoke-test: OK")


def test_smoke_test_with_wkt_and_recording(
    no_jupedsim, recording_v3, capsys
) -> None:
    exit_code = main(
        [
            "--smoke-test",
            "--wkt",
            str(SIMPLE_WKT),
            "--recording",
            str(recording_v3),
        ]
    )

    assert exit_code == 0
    out = capsys.readouterr().out
    assert "documents=2 tabs=0" in out


def test_smoke_test_reports_missing_jupedsim(no_jupedsim, capsys) -> None:
    assert main(["--smoke-test", "--wkt", str(SIMPLE_WKT)]) == 0

    out = capsys.readouterr().out
    assert "routing=unavailable" in out
    assert f'reason="{NO_JUPEDSIM}"' in out


def test_smoke_test_fails_on_missing_wkt(no_jupedsim, tmp_path, capsys) -> None:
    missing = tmp_path / "does_not_exist.wkt"

    assert main(["--smoke-test", "--wkt", str(missing)]) == 1

    err = capsys.readouterr().err
    assert "FAILED loading" in err
    # The message may render the path via repr (doubled backslashes on
    # Windows), so only the file name is compared.
    assert missing.name in err


def test_smoke_test_fails_on_missing_recording(
    no_jupedsim, tmp_path, capsys
) -> None:
    missing = tmp_path / "does_not_exist.sqlite"

    assert main(["--smoke-test", "--recording", str(missing)]) == 1

    err = capsys.readouterr().err
    assert "FAILED loading" in err
    # The message may render the path via repr (doubled backslashes on
    # Windows), so only the file name is compared.
    assert missing.name in err


def test_smoke_test_fails_on_garbage_wkt(no_jupedsim, tmp_path, capsys) -> None:
    broken = tmp_path / "broken.wkt"
    broken.write_text("this is not wkt at all", encoding="UTF-8")

    assert main(["--smoke-test", "--wkt", str(broken)]) == 1

    assert "FAILED loading" in capsys.readouterr().err


@pytest.fixture
def dialogs(monkeypatch) -> list[tuple[str, str]]:
    """Record every ``QMessageBox.critical`` instead of showing it."""
    from PySide6.QtWidgets import QMessageBox

    calls: list[tuple[str, str]] = []

    def critical(parent, title, text, *args, **kwargs):
        calls.append((title, text))
        return QMessageBox.StandardButton.Ok

    monkeypatch.setattr(QMessageBox, "critical", critical)
    return calls


@pytest.fixture
def no_event_loop(monkeypatch) -> None:
    """Make ``app.exec()`` return at once, so ``main`` can be called."""
    from PySide6.QtWidgets import QApplication

    monkeypatch.setattr(QApplication, "exec", lambda self: 0)


def test_a_normal_run_reports_a_load_failure_in_a_dialog(
    no_jupedsim, no_event_loop, dialogs, tmp_path
) -> None:
    """Without --smoke-test a broken file must not raise: it is reported
    and the window still comes up (``app.exec()`` is reached)."""
    missing = tmp_path / "missing.wkt"

    assert main(["--wkt", str(missing)]) == 0

    assert len(dialogs) == 1
    title, text = dialogs[0]
    assert str(missing) in title
    assert text


def test_a_normal_run_reports_a_broken_recording(
    no_jupedsim, no_event_loop, dialogs, tmp_path
) -> None:
    """Same for a recording, and the dialog title names the file.

    Only failing files are passed here on purpose: a file that loads would
    get a render tab, which segfaults without an OpenGL context.
    """
    missing = tmp_path / "missing.sqlite"

    assert main(["--recording", str(missing)]) == 0

    assert [title for title, _ in dialogs] == [f"Error opening {missing}"]


def _platform_after_setup(argv: list[str], monkeypatch) -> str | None:
    """Parse ``argv``, run the platform setup and report ``QT_QPA_PLATFORM``.

    This only ever touches the environment: no Qt object is created, in
    particular no render tab, which is the very thing ``--smoke-test-render``
    would need a real display for.
    """
    monkeypatch.delenv("QT_QPA_PLATFORM", raising=False)
    args = build_parser().parse_args(argv)

    _setup_qt_platform(args)

    return os.environ.get("QT_QPA_PLATFORM")


def test_smoke_test_runs_offscreen(monkeypatch) -> None:
    assert _platform_after_setup(["--smoke-test"], monkeypatch) == "offscreen"


def test_smoke_test_keeps_a_configured_platform(monkeypatch) -> None:
    monkeypatch.setenv("QT_QPA_PLATFORM", "xcb")
    args = build_parser().parse_args(["--smoke-test"])

    _setup_qt_platform(args)

    assert os.environ["QT_QPA_PLATFORM"] == "xcb"


def test_smoke_test_render_keeps_the_real_platform(monkeypatch) -> None:
    """Rendering needs a real display; forcing offscreen would segfault."""
    assert _platform_after_setup(["--smoke-test-render"], monkeypatch) is None
    assert (
        _platform_after_setup(
            ["--smoke-test", "--smoke-test-render"], monkeypatch
        )
        is None
    )


def test_normal_start_keeps_the_real_platform(monkeypatch) -> None:
    assert _platform_after_setup([], monkeypatch) is None


def test_help_describes_the_two_smoke_modes() -> None:
    # argparse wraps the group description and happily breaks words at their
    # hyphens, so compare without line breaks and steer clear of the flag
    # names when checking for a phrase.
    help_text = " ".join(build_parser().format_help().split())

    assert "does that headless" in help_text
    assert "QT_QPA_PLATFORM=offscreen unless already set" in help_text
    assert "leaves the Qt platform alone" in help_text
    assert "needs a real display or a working OpenGL context" in help_text


def test_qsettings_are_isolated_from_the_user(tmp_path) -> None:
    """The smoke test builds a ``MainWindow``, which reads and writes
    ``QSettings``. The autouse fixture in ``conftest.py`` has to keep that
    away from the settings of whoever runs the suite."""
    from jupedsim_vis import main_window

    settings = main_window.QSettings("jupedsim", "jupedsim_vis")

    # QSettings reports the file with forward slashes on every platform.
    settings_file = Path(settings.fileName()).resolve()
    assert settings_file.is_relative_to(tmp_path.resolve())


def test_watchdog_is_a_single_shot_timer(qapplication) -> None:
    watchdog = _start_watchdog(0.5)
    try:
        assert watchdog.isSingleShot()
        assert watchdog.interval() == 500
        assert watchdog.isActive()
    finally:
        watchdog.stop()


@pytest.mark.skip(
    reason="the watchdog calls os._exit(2), which cannot be observed in "
    "process, and it only fires when the event loop hangs, which cannot be "
    "provoked deterministically from the outside"
)
def test_smoke_test_times_out() -> None:  # pragma: no cover
    pass


def test_smoke_test_runs_as_a_module(tmp_path) -> None:
    proc = subprocess.run(
        [
            sys.executable,
            "-m",
            "jupedsim_vis",
            "--smoke-test",
            "--wkt",
            str(SIMPLE_WKT),
        ],
        env={**os.environ, "QT_QPA_PLATFORM": "offscreen"},
        capture_output=True,
        text=True,
        timeout=120,
    )

    assert proc.returncode == 0, proc.stdout + proc.stderr
    assert "smoke-test: OK" in proc.stdout, proc.stdout + proc.stderr
    assert "documents=1 tabs=0" in proc.stdout, proc.stdout + proc.stderr
