# SPDX-License-Identifier: LGPL-3.0-or-later
"""Reusable native viewer stub for a running JuPedSim ``Simulation``.

Hand it a prepared ``Simulation`` and call :meth:`SimulationViewer.run`. It opens a native
desktop window (pywebview / WebKitGTK) with a single Play/Pause button. While playing it calls
``sim.iterate()`` continuously; every ``render_every`` steps it redraws the agents as simple
cylinders (height 1.0, radius 0.2). A per-step callback runs on *every* iterate step -- that is
the hook where a caller can add agents, retarget, log, etc.

The trame server is rendered offscreen and hosted in a background thread (in-process, so it
shares the live ``sim``); pywebview owns the main thread and shows the frames. No browser, no
URL to paste.
"""

from __future__ import annotations

import os
import socket
import threading
import time
import urllib.request
from pathlib import Path
from typing import Callable, Optional

# pyvista/VTK must go offscreen *before* they initialise: frames render to an FBO and stream to
# the embedded webview, never into an on-screen native GL window.
os.environ.setdefault("PYVISTA_OFF_SCREEN", "true")

import numpy as np  # noqa: E402
import pyvista as pv  # noqa: E402

pv.OFF_SCREEN = True

from pyvista.trame.ui import plotter_ui  # noqa: E402
from trame.app import get_server  # noqa: E402
from trame.ui.vuetify3 import SinglePageLayout  # noqa: E402
from trame.widgets import vuetify3 as v3  # noqa: E402

TITLE = "JuPedSim - Simulation Viewer"

# One low-poly cylinder template, placed (not scaled/oriented) at every agent position. Built
# with its base at z=0 and centre at z=0.5 so a glyph anchored at (x, y, z_floor) stands *on*
# the floor and reaches up 1.0 m. Low resolution keeps the per-frame glyph rebuild cheap.
_AGENT_CYLINDER = pv.Cylinder(
    center=(0.0, 0.0, 0.5),
    direction=(0.0, 0.0, 1.0),
    radius=0.2,
    height=1.0,
    resolution=12,
)

# Callback invoked once per iterate step, before the (throttled) redraw. Receives the sim.
StepCallback = Callable[["object"], None]


def _find_free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def _wait_until_serving(port: int, timeout: float = 20.0) -> bool:
    deadline = time.time() + timeout
    url = f"http://localhost:{port}/index.html"
    while time.time() < deadline:
        try:
            urllib.request.urlopen(url, timeout=0.5)
            return True
        except Exception:
            time.sleep(0.2)
    return False


class SimulationViewer:
    """Native Play/Pause viewer around a live JuPedSim ``Simulation``.

    Arguments:
        sim: A prepared ``jupedsim.Simulation`` (geometry, journeys, agents already set up).
        on_step: Optional callback run on *every* ``sim.iterate()`` step, receiving ``sim``.
            Use it to add/retarget agents or record state. It is registered up front and stays
            for the life of the viewer.
        steps_per_frame: Sim steps advanced per rendered frame (default 10). Every frame is
            shown and the sim never runs ahead of the display; raise it to make the sim run
            faster. Editable live in the toolbar.
        target_fps: Frame-pacing rate (default 30). The loop sleeps out the rest of each frame
            period, so playback speed is roughly ``steps_per_frame * target_fps`` steps/second.
        geometry_obj: Optional path to the OBJ the sim was built from; when given, its surface
            is drawn once as static context under the agents.
    """

    def __init__(
        self,
        sim,
        on_step: Optional[StepCallback] = None,
        *,
        steps_per_frame: int = 10,
        target_fps: float = 30.0,
        geometry_obj: Optional[os.PathLike | str] = None,
    ) -> None:
        self._sim = sim
        self._on_step = on_step
        # Sim steps advanced per rendered frame -- the on-screen speed knob (editable live in the
        # toolbar). Every frame is shown; the sim never runs ahead of the display.
        self._steps_per_frame = max(1, int(steps_per_frame))
        # Frame pacing period: we sleep out the rest of it after each frame, so the sim advances
        # steps_per_frame per ~1/target_fps and the event loop stays free for camera/input.
        self._min_frame_dt = 1.0 / max(1.0, float(target_fps))
        self._geometry_obj = Path(geometry_obj) if geometry_obj else None

        self._plotter = pv.Plotter()
        self._plotter.set_background("white")
        # Terrain-style interaction: keeps +z fixed as "up" (no roll / flipping), matching a
        # floor-plan / multi-level building better than the default trackball. Same as trame_viewer.
        self._plotter.enable_terrain_style(
            mouse_wheel_zooms=True, shift_pans=True
        )
        self._server = None  # built lazily in run()
        self._agents_mesh = (
            None  # glyph dataset, updated in place per frame (see _draw_agents)
        )
        self._floor_mesh = (
            None  # static surface mesh, loaded once (see _load_floor_mesh)
        )
        self._n_regions = 0

    # -- rendering ------------------------------------------------------

    def _load_floor_mesh(self) -> None:
        """Build the walkable-surface mesh once. Holds both a height scalar and a per-cell
        region id so the floor can be recoloured (see _draw_floor) without reloading."""
        if self._geometry_obj is None:
            return
        from jupedsim.internal.routing import Geometry

        # Read a second time rather than borrowed from the simulation, which owns its geometry.
        geo = Geometry.from_obj(str(self._geometry_obj))
        verts = np.asarray(geo.vertices(), dtype=float)
        tris = np.asarray(geo.triangles(), dtype=np.int64)
        faces = np.hstack(
            [np.full((len(tris), 1), 3, dtype=np.int64), tris]
        ).ravel()
        mesh = pv.PolyData(verts, faces)
        mesh["elevation"] = verts[
            :, 2
        ]  # colour by height (point scalar), like trame_viewer
        region_ids = np.asarray(geo.region_id_per_face(), dtype=float)
        n_regions = int(geo.region_count())
        if len(region_ids) == mesh.n_cells:
            mesh.cell_data["region"] = region_ids  # per-face region id
        else:  # face-count mismatch: disable region colouring rather than mislabel
            mesh.cell_data["region"] = np.zeros(mesh.n_cells, dtype=float)
            n_regions = 0
        self._floor_mesh = mesh
        self._n_regions = n_regions

    def _draw_floor(self, wireframe: bool, color_by_region: bool) -> None:
        """(Re)draw the floor actor. Cheap: the mesh is static, only the mapper's scalars/edges
        change, so this can run on every toolbar toggle."""
        if self._floor_mesh is None:
            return
        # Drop old scalar bars first so toggling colour mode doesn't stack copies.
        for title in ("Height z [m]", "Region"):
            try:
                self._plotter.remove_scalar_bar(title=title)
            except (KeyError, StopIteration):
                pass
        if color_by_region and self._n_regions > 0:
            n = self._n_regions
            self._plotter.add_mesh(
                self._floor_mesh,
                name="floor",
                scalars="region",
                cmap="tab20",
                n_colors=max(n, 1),
                clim=(0, max(n - 1, 1)),
                show_edges=wireframe,
                edge_color="darkgray",
                scalar_bar_args=dict(title="Region"),
                pickable=False,
            )
        else:
            self._plotter.add_mesh(
                self._floor_mesh,
                name="floor",
                scalars="elevation",
                cmap="viridis",
                show_edges=wireframe,
                edge_color="darkgray",
                scalar_bar_args=dict(title="Height z [m]"),
                pickable=False,
            )

    def _agent_points(self) -> np.ndarray:
        pts = [
            (ag.location.x, ag.location.y, ag.location.z)
            for ag in self._sim.agents()
        ]
        return (
            np.asarray(pts, dtype=float)
            if pts
            else np.empty((0, 3), dtype=float)
        )

    def _draw_agents(self) -> None:
        pts = self._agent_points()
        if len(pts) == 0:
            self._plotter.remove_actor("agents", render=False)
            self._agents_mesh = None
            return
        # orient/scale off: every agent gets the same upright unit cylinder, just translated.
        glyphs = pv.PolyData(pts).glyph(
            geom=_AGENT_CYLINDER, orient=False, scale=False
        )
        # In place after the first frame: copy_from updates the dataset the actor already holds
        # (~0.1 ms), avoiding the add_mesh actor/mapper rebuild (~1 ms). It does NOT save the
        # glyph() geometry rebuild above -- that's the part that grows with agent count.
        if self._agents_mesh is None:
            self._agents_mesh = glyphs
            self._plotter.add_mesh(glyphs, name="agents", color="royalblue")
        else:
            self._agents_mesh.copy_from(glyphs)

    # -- trame app ------------------------------------------------------

    def _build_server(self):
        server = get_server(client_type="vue3")
        state, ctrl = server.state, server.controller
        state.playing = False
        state.iteration = int(self._sim.iteration_count())
        state.n_agents = int(self._sim.agent_count())
        state.steps_per_frame = (
            self._steps_per_frame
        )  # live speed knob (toolbar)
        state.wireframe = True  # show the triangulation edges
        state.color_by_region = (
            False  # False -> colour by height, True -> by region id
        )

        self._load_floor_mesh()
        self._draw_floor(state.wireframe, state.color_by_region)
        self._draw_agents()
        self._plotter.add_axes()
        self._plotter.reset_camera()
        self._plotter.view_isometric()

        @state.change("wireframe", "color_by_region")
        def _on_display(wireframe, color_by_region, **_):
            self._draw_floor(bool(wireframe), bool(color_by_region))
            if ctrl.view_update:
                ctrl.view_update()

        def _sync_state() -> None:
            state.iteration = int(self._sim.iteration_count())
            state.n_agents = int(self._sim.agent_count())

        def _render_frame() -> None:
            self._draw_agents()
            with state:
                _sync_state()
            if ctrl.view_update:
                ctrl.view_update()

        async def _run_loop() -> None:
            import asyncio

            # Frame-locked: advance exactly steps_per_frame steps, draw that frame, then sleep
            # out the rest of the frame period. Every frame is shown and the sim never runs ahead
            # of the display; the sleep is also what keeps the event loop free for camera/input.
            while state.playing:
                frame_start = time.perf_counter()
                try:
                    n = max(1, int(state.steps_per_frame))
                except (TypeError, ValueError):
                    n = (
                        self._steps_per_frame
                    )  # mid-typing in the field: keep the last good value
                for _ in range(n):
                    self._sim.iterate()
                    if self._on_step is not None:
                        self._on_step(self._sim)
                    if self._sim.agent_count() == 0:
                        break
                _render_frame()
                if self._sim.agent_count() == 0:
                    with state:
                        state.playing = False
                    break
                elapsed = time.perf_counter() - frame_start
                await asyncio.sleep(max(0.0, self._min_frame_dt - elapsed))

        @ctrl.set("toggle_play")
        def _toggle_play():
            import asyncio

            state.playing = not state.playing
            if state.playing:
                asyncio.create_task(_run_loop())

        @ctrl.add("on_client_connected")
        def _on_connect(*_a, **_k):
            # The very first streamed frame can miss the edge overlay, so the triangulation stays
            # invisible until the user toggles it. Re-add the floor actor once the client is
            # actually attached to force a fresh render with edges (same fix as trame_viewer).
            self._draw_floor(bool(state.wireframe), bool(state.color_by_region))
            if ctrl.view_update:
                ctrl.view_update()

        with SinglePageLayout(server) as layout:
            layout.title.set_text(TITLE)
            with layout.toolbar:
                v3.VBtn(
                    "{{ playing ? 'Pause' : 'Play' }}",
                    click=ctrl.toggle_play,
                    prepend_icon=("playing ? 'mdi-pause' : 'mdi-play'",),
                    variant="tonal",
                )
                v3.VTextField(
                    v_model_number=("steps_per_frame",),
                    type="number",
                    min=1,
                    label="Steps / frame",
                    density="compact",
                    variant="outlined",
                    hide_details=True,
                    classes="ml-4",
                    style="max-width: 130px",
                )
                v3.VSwitch(
                    v_model=("wireframe", True),
                    label="Triangulation",
                    hide_details=True,
                    classes="ml-4",
                )
                # off = colour by height (viridis), on = colour by region id (tab20)
                v3.VSwitch(
                    v_model=("color_by_region", False),
                    label="Regions",
                    hide_details=True,
                    classes="ml-4",
                )
                v3.VSpacer()
                v3.VChip("Step {{ iteration }}", classes="mr-2")
                v3.VChip("{{ n_agents }} agents")
            with layout.content:
                with v3.VContainer(fluid=True, classes="fill-height pa-0"):
                    view = plotter_ui(self._plotter, mode="server")
                    ctrl.view_update = view.update

        return server

    # -- run ------------------------------------------------------------

    def run(
        self, *, port: Optional[int] = None, window_size=(1200, 800)
    ) -> None:
        """Open the native window and block until it is closed.

        The trame server runs in a background daemon thread (sharing this process's ``sim``);
        pywebview owns the main thread and displays the offscreen-rendered frames.
        """
        import asyncio

        import webview

        port = port or _find_free_port()
        self._server = self._build_server()

        def _serve() -> None:
            # Fresh event loop for this thread; trame/wslink picks it up via get_event_loop().
            # aiohttp's runner installs SIGINT/SIGTERM handlers via loop.add_signal_handler,
            # which only works on the main thread -- neutralise them here (we don't need the
            # graceful-signal shutdown: it's a daemon thread that dies with the process when the
            # webview window closes).
            loop = asyncio.new_event_loop()
            loop.add_signal_handler = lambda *a, **k: None
            loop.remove_signal_handler = lambda *a, **k: False
            asyncio.set_event_loop(loop)
            self._server.start(
                port=port,
                open_browser=False,
                show_connection_info=False,
            )

        thread = threading.Thread(target=_serve, daemon=True)
        thread.start()
        if not _wait_until_serving(port):
            raise RuntimeError(f"trame server did not come up on port {port}")

        webview.create_window(
            TITLE,
            f"http://localhost:{port}",
            width=window_size[0],
            height=window_size[1],
        )
        webview.start()
