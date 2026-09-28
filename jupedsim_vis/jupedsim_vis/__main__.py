# SPDX-License-Identifier: LGPL-3.0-or-later
"""Entry point: ``jupedsim-vis`` (console script) and
``python -m jupedsim_vis``.

Besides opening the files named on the command line this module implements
the headless smoke test CI runs: it starts the application, loads those
files *without* creating render tabs (a render tab needs a real OpenGL
context), checks the resulting state and exits instead of showing a window.
"""

import argparse
import os
import sys
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:  # pragma: no cover - imported for type checkers only
    from PySide6.QtCore import QTimer

    from jupedsim_vis.main_window import MainWindow

#: Exit code of the smoke test watchdog.
EXIT_TIMEOUT = 2

#: Every routing failure message starts with this (see ``routing.py``).
_ROUTING_PREFIX = "Cannot calculate shortest path"


def build_parser() -> argparse.ArgumentParser:
    """The command line of ``jupedsim-vis``."""
    p = argparse.ArgumentParser(
        prog="jupedsim-vis",
        description=(
            "Viewer for JuPedSim geometries and trajectory recordings."
        ),
    )
    p.add_argument(
        "--wkt",
        type=Path,
        action="append",
        default=[],
        metavar="FILE",
        help="open a WKT geometry file on startup (repeatable)",
    )
    p.add_argument(
        "--recording",
        type=Path,
        action="append",
        default=[],
        metavar="FILE",
        help="open a sqlite recording on startup (repeatable)",
    )
    testing = p.add_argument_group(
        "testing",
        "Flags used to test the application, e.g. in CI. They load the given "
        "files, run self-checks and exit instead of showing the window. "
        "--smoke-test does that headless (QT_QPA_PLATFORM=offscreen unless "
        "already set); --smoke-test-render leaves the Qt platform alone, it "
        "needs a real display or a working OpenGL context.",
    )
    testing.add_argument(
        "--smoke-test",
        action="store_true",
        help=(
            "start, load the given files without rendering, run self-checks "
            "and exit (0 ok, 1 failure, 2 timeout)"
        ),
    )
    testing.add_argument(
        "--smoke-test-render",
        action="store_true",
        help=(
            "like --smoke-test but also creates the render tabs; keeps the "
            "configured Qt platform, which must provide a working OpenGL "
            "context (e.g. QT_QPA_PLATFORM=xcb under Xvfb)"
        ),
    )
    testing.add_argument(
        "--smoke-test-timeout",
        type=float,
        default=60.0,
        metavar="SECONDS",
        help="abort the smoke test after this many seconds (default: 60)",
    )
    return p


def _forces_offscreen(args: argparse.Namespace) -> bool:
    """Whether this run has to pick the offscreen Qt platform itself.

    Only ``--smoke-test`` does: it never creates a render tab, so it can run
    anywhere. ``--smoke-test-render`` does create them, and a render tab
    without a real window handle crashes the process, so its platform is
    left to whoever starts it (``QT_QPA_PLATFORM=xcb`` under Xvfb in CI, a
    real display for a developer).
    """
    return bool(args.smoke_test and not args.smoke_test_render)


def _setup_qt_platform(args: argparse.Namespace) -> None:
    """Select the Qt platform, if this run gets to choose.

    Must be called before the first PySide6 import.
    """
    if _forces_offscreen(args):
        os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


def _open_files(window: "MainWindow", args: argparse.Namespace) -> None:
    """Open the files named on the command line of a normal run.

    A file that cannot be opened is reported in a dialog and skipped: the
    remaining files still open and the window comes up either way. Only the
    smoke test turns a load failure into a non-zero exit code.
    """
    from PySide6.QtWidgets import QMessageBox

    openers = [(path, window.open_wkt_file) for path in args.wkt]
    openers += [(path, window.open_recording_file) for path in args.recording]
    for path, open_file in openers:
        try:
            open_file(path, with_view=True)
        except Exception as e:
            QMessageBox.critical(window, f"Error opening {path}", str(e))


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    smoke = args.smoke_test or args.smoke_test_render

    # Has to happen before the first PySide6 import, which is why Qt is
    # imported inside this function and not at module level.
    _setup_qt_platform(args)

    from PySide6.QtCore import QTimer
    from PySide6.QtWidgets import QApplication

    from jupedsim_vis.main_window import MainWindow

    app = QApplication.instance() or QApplication(sys.argv[:1])
    window = MainWindow()

    if not smoke:
        _open_files(window, args)
        return app.exec()

    with_view = args.smoke_test_render
    try:
        for path in args.wkt:
            window.open_wkt_file(path, with_view=with_view)
        for path in args.recording:
            window.open_recording_file(path, with_view=with_view)
    except Exception as e:
        print(f"smoke-test: FAILED loading: {e}", file=sys.stderr)
        return 1

    watchdog = _start_watchdog(args.smoke_test_timeout)
    QTimer.singleShot(0, app.quit)
    app.exec()
    # One process may run several smoke tests (the test suite does); a
    # watchdog left running would kill it later on.
    watchdog.stop()
    return _smoke_report(window, args, with_view)


def _start_watchdog(timeout: float) -> "QTimer":
    """Arm the timer that kills a smoke test which never finishes."""
    from PySide6.QtCore import QTimer

    watchdog = QTimer()
    watchdog.setSingleShot(True)
    watchdog.setInterval(int(timeout * 1000))
    watchdog.timeout.connect(_abort_on_timeout)
    watchdog.start()
    return watchdog


def _abort_on_timeout() -> None:
    """Leave the process the hard way.

    The point of the watchdog is a Qt event loop that does not return, so
    unwinding is not an option.
    """
    print("smoke-test: FAILED: timeout", file=sys.stderr, flush=True)
    os._exit(EXIT_TIMEOUT)


def _smoke_report(
    window: "MainWindow", args: argparse.Namespace, with_view: bool
) -> int:
    """Check the state the smoke test ended up in and report it.

    Returns ``0`` after printing a summary and ``smoke-test: OK``, or ``1``
    after printing the first failed check to stderr.
    """
    from jupedsim_vis.main_window import (
        GeometryDocument,
        RecordingDocument,
    )
    from jupedsim_vis.routing import check_available

    documents = len(window.documents)
    tabs = window.tabs.count()
    expected_documents = len(args.wkt) + len(args.recording)
    expected_tabs = expected_documents if with_view else 0
    title = window.windowTitle()
    message = window.statusBar().currentMessage()
    reason = check_available()

    checks: list[tuple[bool, str]] = [
        (
            title == "jupedsim_vis",
            f"unexpected window title {title!r}",
        ),
        (
            documents == expected_documents,
            f"expected {expected_documents} documents, got {documents}",
        ),
        (
            tabs == expected_tabs,
            f"expected {expected_tabs} tabs, got {tabs}",
        ),
    ]
    if reason is None:
        checks.append(
            (
                message == "",
                f"expected an empty status bar, got {message!r}",
            )
        )
    else:
        checks.append(
            (
                message.startswith(_ROUTING_PREFIX),
                f"expected the status bar to report {reason!r}, "
                f"got {message!r}",
            )
        )
    for doc in window.documents:
        if isinstance(doc, GeometryDocument):
            checks.append(
                (
                    doc.geo.num_triangles > 0,
                    f"{doc.path} was triangulated into nothing",
                )
            )
        elif isinstance(doc, RecordingDocument):
            checks.append(
                (doc.recording.num_frames > 0, f"{doc.path} has no frames")
            )

    for ok, what in checks:
        if not ok:
            print(f"smoke-test: FAILED: {what}", file=sys.stderr)
            return 1

    summary = (
        f"smoke-test: platform={os.environ.get('QT_QPA_PLATFORM') or 'default'}"
        f" title={title} documents={documents} tabs={tabs}"
        f" routing={'unavailable' if reason else 'available'}"
    )
    if reason:
        summary += f' reason="{reason}"'
    print(summary)
    print("smoke-test: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
