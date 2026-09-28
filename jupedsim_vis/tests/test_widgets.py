# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for the GUI modules that do not need an OpenGL context.

Constructing ``RenderWidget`` (and therefore ``ViewGeometryWidget`` /
``ReplayWidget``) segfaults under every headless Qt platform, so the widgets
themselves are only covered by the optional Xvfb job. What is covered here is
everything around them: the modules must import without jupedsim, and
``Trajectory`` works on plain VTK data objects.
"""

import importlib
import sys

from make_fixture_recording import DEFAULT_BOUNDS, write_recording

from jupedsim_vis.recording import Recording
from jupedsim_vis.trajectory import Trajectory

#: Every module of the package that pulls in Qt or VTK. None of them may
#: import jupedsim; routing is reached through ``routing.create_router``.
GUI_MODULES = (
    "jupedsim_vis.geometry",
    "jupedsim_vis.geometry_widget",
    "jupedsim_vis.grid",
    "jupedsim_vis.main_window",
    "jupedsim_vis.move_controller",
    "jupedsim_vis.replay_widget",
    "jupedsim_vis.trajectory",
    "jupedsim_vis.view_geometry_widget",
    "jupedsim_vis.__main__",
)


def test_gui_modules_import_without_jupedsim(monkeypatch):
    """``None`` in ``sys.modules`` makes ``import jupedsim`` raise, so this
    fails if any module imports jupedsim at import time -- even in an
    environment where jupedsim happens to be installed."""
    monkeypatch.setitem(sys.modules, "jupedsim", None)
    for name in list(sys.modules):
        if name == "jupedsim_vis" or name.startswith("jupedsim_vis."):
            monkeypatch.delitem(sys.modules, name, raising=False)

    for name in GUI_MODULES:
        importlib.import_module(name)


#: Asymmetric on purpose: a box of 0 0 -> 30 10 catches a reader that
#: swaps the x and y bounds, which a square never would.
WIDE_BOUNDS = (0.0, 30.0, 0.0, 10.0)


def _trajectory(
    tmp_path,
    *,
    frames: int,
    agents: int,
    bounds: tuple[float, float, float, float] = DEFAULT_BOUNDS,
) -> Trajectory:
    path = tmp_path / "recording_v3.sqlite"
    write_recording(
        path, version=3, frames=frames, agents=agents, bounds=bounds
    )
    return Trajectory(Recording(path))


def test_trajectory_reports_frames_and_bounds(tmp_path):
    trajectory = _trajectory(tmp_path, frames=5, agents=4, bounds=WIDE_BOUNDS)

    assert trajectory.num_frames == 5
    bounds = trajectory.get_bounds()
    assert (
        bounds.xmin,
        bounds.xmax,
        bounds.ymin,
        bounds.ymax,
    ) == WIDE_BOUNDS


def test_trajectory_starts_on_the_first_frame(tmp_path):
    trajectory = _trajectory(tmp_path, frames=5, agents=4)

    assert trajectory.current_index == 0
    assert trajectory.polydata.GetNumberOfPoints() == 4


def test_trajectory_goto_frame_clamps(tmp_path):
    trajectory = _trajectory(tmp_path, frames=5, agents=4)

    trajectory.goto_frame(2)
    assert trajectory.current_index == 2
    assert trajectory.polydata.GetNumberOfPoints() == 4

    trajectory.goto_frame(99)
    assert trajectory.current_index == 4

    trajectory.goto_frame(-99)
    assert trajectory.current_index == 0


def test_trajectory_advance_frame_clamps(tmp_path):
    trajectory = _trajectory(tmp_path, frames=5, agents=4)

    trajectory.advance_frame(3)
    assert trajectory.current_index == 3
    assert trajectory.polydata.GetNumberOfPoints() == 4

    trajectory.advance_frame(10)
    assert trajectory.current_index == 4

    trajectory.advance_frame(-10)
    assert trajectory.current_index == 0
    assert trajectory.polydata.GetNumberOfPoints() == 4
