#! /usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""3D viewer for a multi-level navigation mesh, via pyvista + trame.

The 3D scene is rendered *offscreen* by VTK (server side) and shown either in the browser or
in a native desktop window (``--native``, via pywebview's WebKitGTK).
"""

import argparse
import asyncio
import os
import socket
import tempfile
import time
import urllib.request

# Force pyvista offscreen before it (and VTK) initialise: the frame is rendered to an FBO and
# streamed to the client, never into an on-screen native window.
os.environ.setdefault("PYVISTA_OFF_SCREEN", "true")

import numpy as np  # noqa: E402
import pyvista as pv  # noqa: E402
from vtkmodules.vtkRenderingCore import vtkCellPicker  # noqa: E402

pv.OFF_SCREEN = True

from pyvista.trame.ui import plotter_ui  # noqa: E402
from trame.app import get_server  # noqa: E402
from trame.app.file_upload import ClientFile  # noqa: E402
from trame.ui.vuetify3 import SinglePageWithDrawerLayout  # noqa: E402
from trame.widgets import vuetify3 as v3  # noqa: E402

from jupedsim.internal.routing_3d import (  # noqa: E402
    SurfaceMeshShortestPathRoutingEngine,
)

TITLE = "JuPedSim - Multi-Level Viewer"


def find_free_port() -> int:
    """Ask the OS for an unused TCP port (bind to :0, read it back, release it)."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def build_app(obj_path: str | None):
    """Build the trame server hosting a pyvista Plotter"""
    plotter = pv.Plotter()
    plotter.set_background("white")
    plotter.enable_parallel_projection()

    server = get_server(client_type="vue3")
    state, ctrl = server.state, server.controller
    state.z_min = 0.0
    state.z_max = 1.0
    state.clip_range = [
        0.0,
        1.0,
    ]  # [low, high] band -- slider (reassigns the whole array)
    state.clip_low = (
        0.0  # mirror for the editable number fields (scalars sync reliably;
    )
    state.clip_high = (
        1.0  # an in-place clip_range[i] mutation would NOT sync to the server)
    )
    state.wireframe = True
    state.mesh_name = ""
    state.obj_file = None
    state.n_points = 0
    state.n_cells = 0
    # Routing: a right-click sets the start, then the hovered point is the live target, and the
    # next right-click freezes the path. Hover only tracks while no button is held, so the left
    # button and all modifier / wheel gestures stay pure camera (rotate / pan / spin / zoom).
    state.route_status = "idle"  # idle | tracking | frozen
    state.cursor_pos = "—"  # live, unsnapped surface position under the cursor
    state.route_start = "—"
    state.route_target = "—"
    state.route_cost = 0.0
    state.route_corners = 0
    state.route_setup_us = 0  # set_target() precompute time
    state.route_query_us = 0  # get_shortest_path() query time

    # Holds the currently loaded mesh + its height range; swapped out by load_mesh().
    current: dict = {"mesh": None, "marker_r": 0.1}
    # Routing interaction state. Start/target are exact surface world positions (xyz), not mesh
    # vertices -- the geodesic is computed by the engine, not snapped to the triangulation.
    route: dict = {
        "start_pos": None,
        "target_pos": None,
        "tracking": False,
        "pending": None,  # latest un-processed hover (fx, fy); coalesced, not queued
        "busy": False,  # a hover compute/render is currently in flight
        "line": None,  # persistent polyline dataset, updated in place per hover
    }
    # Exact any-angle geodesic engine (CGAL MMP), fed the same OBJ as the displayed mesh.
    engine = SurfaceMeshShortestPathRoutingEngine()
    picker = vtkCellPicker()
    picker.SetTolerance(0.0005)

    def draw() -> None:
        mesh = current["mesh"]
        if mesh is None:
            return
        lo, hi = (float(v) for v in state.clip_range)
        z_min, z_max = current["z_min"], current["z_max"]
        if lo <= z_min and hi >= z_max:
            sub = mesh
        else:
            # keep only the cells whose elevation lies inside the band -> isolates floors,
            # including middle ones when there are more than two levels.
            sub = mesh.threshold((lo, hi), scalars="elevation")
            if sub.n_cells == 0:  # empty band
                sub = mesh.threshold((lo, lo + 1e-6), scalars="elevation")
        # Replace only the "mesh" actor by name (not plotter.clear()) so the route actors
        # survive a clip/wireframe change; drop the old scalar bar to avoid stacking copies.
        try:
            plotter.remove_scalar_bar(title="Height z [m]")
        except (KeyError, StopIteration):
            pass
        plotter.add_mesh(
            sub,
            name="mesh",
            scalars="elevation",
            cmap="viridis",
            show_edges=bool(state.wireframe),
            edge_color="black",
            scalar_bar_args=dict(title="Height z [m]"),
        )
        plotter.add_axes()

    def load_mesh(mesh, name: str, obj_path: str) -> None:
        # Feed the exact-geodesic engine the same OBJ (it builds its own triangulation + AABB tree).
        engine.set_geometry_from_obj(obj_path)
        mesh["elevation"] = mesh.points[:, 2]  # colour + clip by height
        z_min = float(mesh.points[:, 2].min())
        z_max = float(mesh.points[:, 2].max())
        if z_max <= z_min:  # flat mesh: keep the slider non-degenerate
            z_max = z_min + 1.0
        diag = float(
            np.linalg.norm(
                np.asarray(mesh.bounds[1::2]) - np.asarray(mesh.bounds[::2])
            )
        )
        current.update(
            mesh=mesh, z_min=z_min, z_max=z_max, marker_r=max(0.01 * diag, 0.05)
        )
        clear_route()
        with state:
            state.z_min, state.z_max = z_min, z_max
            state.clip_range = [z_min, z_max]  # start showing the whole mesh
            state.clip_low, state.clip_high = z_min, z_max
            state.mesh_name = name
            state.n_points = int(mesh.n_points)
            state.n_cells = int(mesh.n_cells)
        draw()
        plotter.reset_camera()
        plotter.view_isometric()
        if ctrl.view_update:
            ctrl.view_update()

    def _redraw() -> None:
        draw()
        if ctrl.view_update:
            ctrl.view_update()

    @ctrl.add("on_client_connected")
    def _on_connect(*_a, **_k):
        # The very first streamed frame can miss the edge overlay; re-render once the client is
        # actually attached so the triangulation shows without the user toggling it off/on.
        _redraw()

    @state.change("clip_range", "wireframe")
    def _on_range(clip_range, **_):
        # Mirror the slider into the number fields (skip if already equal -> no feedback loop).
        lo, hi = float(clip_range[0]), float(clip_range[1])
        if [lo, hi] != [state.clip_low, state.clip_high]:
            with state:
                state.clip_low, state.clip_high = lo, hi
        _redraw()

    @state.change("clip_low", "clip_high")
    def _on_fields(clip_low, clip_high, **_):
        try:
            band = [float(clip_low), float(clip_high)]
        except (TypeError, ValueError):
            return  # mid-typing / empty field
        if band != [float(v) for v in state.clip_range]:
            state.clip_range = band  # reassign -> _on_range redraws

    @state.change("obj_file")
    def _on_file(obj_file, **_):
        if not obj_file:
            return
        entry = obj_file[0] if isinstance(obj_file, (list, tuple)) else obj_file
        client_file = ClientFile(entry)
        if client_file.is_empty:
            return
        # pyvista picks the reader by extension, so the bytes need a real .obj file.
        with tempfile.NamedTemporaryFile(suffix=".obj", delete=False) as tmp:
            tmp.write(client_file.content)
            tmp_path = tmp.name
        try:
            load_mesh(pv.read(tmp_path), client_file.name, tmp_path)
        finally:
            os.unlink(tmp_path)

    @ctrl.set("reset_view")
    def _reset_view():
        plotter.view_isometric()
        if ctrl.view_update:
            ctrl.view_update()

    # -- routing via JuPedSim routing engine----------------------------

    def clear_route() -> None:
        route.update(start_pos=None, target_pos=None, tracking=False, line=None)
        for actor_name in ("route_path", "route_start", "route_target"):
            plotter.remove_actor(actor_name, render=False)
        with state:
            state.route_status = "idle"
            state.route_start = state.route_target = "—"
            state.route_cost, state.route_corners = 0.0, 0
            state.route_setup_us, state.route_query_us = 0, 0

    def _fmt(p) -> str:
        return f"{p[0]:.3f}, {p[1]:.3f}, {p[2]:.3f}"

    def _place_marker(actor_name: str, center, color: str) -> None:
        # Create the marker sphere once, then just move it. add_mesh() rebuilds
        # the actor/mapper (~1 ms); SetPosition on the existing actor is ~µs.
        actor = plotter.actors.get(actor_name)
        if actor is None:
            actor = plotter.add_mesh(
                pv.Sphere(
                    radius=current["marker_r"],
                    theta_resolution=12,
                    phi_resolution=12,
                ),
                name=actor_name,
                color=color,
                pickable=False,  # never intercept the pick ray meant for the surface
            )
        actor.SetPosition(float(center[0]), float(center[1]), float(center[2]))

    def _draw_path(pts) -> None:
        # Update the polyline dataset in place (copy_from ~0.1 ms) rather than
        # re-adding the actor every hover (add_mesh ~1 ms + mapper rebuild).
        new = pv.lines_from_points(pts)
        if route["line"] is None:
            route["line"] = new
            plotter.add_mesh(
                new, name="route_path", color="red", line_width=6, pickable=False
            )
        else:
            route["line"].copy_from(new)

    def _reset_path() -> None:
        plotter.remove_actor("route_path", render=False)
        route["line"] = None

    def _pick_world(x, y):
        """Ray-pick at display coords (x, y) -> exact surface world position, or None if off-mesh."""
        picker.Pick(x, y, 0, plotter.renderer)
        if picker.GetCellId() < 0:
            return None
        return np.asarray(picker.GetPickPosition())

    def update_route(target_pos) -> None:
        start_pos = route["start_pos"]
        if start_pos is None:
            return
        prev = route["target_pos"]
        # Check whether "close" to previous target to make live-updates not feel too sloppy.
        if (
            prev is not None
            and float(np.linalg.norm(target_pos - prev))
            < 0.25 * current["marker_r"]
        ):
            return
        # The fixed start is CGAL's source (set via set_target); the live hovered
        # point is the query source. The returned path runs hover -> start.
        query_t = tuple(float(v) for v in target_pos)
        t0 = time.perf_counter()
        try:
            path, cost = engine.get_shortest_path(query_t)
        except Exception:
            return  # off-surface pick or no path
        elapsed_us = (time.perf_counter() - t0) * 1e6
        if len(path) < 2:
            return
        pts = np.asarray(path, dtype=float)
        route["target_pos"] = np.asarray(target_pos, dtype=float)
        _draw_path(pts)
        _place_marker("route_target", target_pos, "red")
        with state:
            state.route_target = _fmt(target_pos)
            state.route_cost = round(cost, 3)
            state.route_corners = int(len(pts))
            state.route_query_us = int(elapsed_us)
        if ctrl.view_update:
            ctrl.view_update()

    def set_start(start_pos) -> None:
        # The fixed start becomes the engine's target: set_target() runs the
        # (expensive) per-target precompute here, once, so live hovers are cheap.
        t0 = time.perf_counter()
        try:
            engine.set_target(tuple(float(v) for v in start_pos))
        except Exception:
            return  # off-surface pick
        setup_us = (time.perf_counter() - t0) * 1e6
        route.update(
            start_pos=np.asarray(start_pos, dtype=float),
            tracking=True,
            target_pos=None,
        )
        _reset_path()
        plotter.remove_actor(
            "route_target", render=False
        )  # drop the previous run's endpoint
        _place_marker("route_start", start_pos, "limegreen")
        with state:
            state.route_status = "tracking"
            state.route_start = _fmt(start_pos)
            state.route_target = "—"
            state.route_cost, state.route_corners, state.route_query_us = (
                0.0,
                0,
                0,
            )
            state.route_setup_us = int(setup_us)
        if ctrl.view_update:
            ctrl.view_update()

    # Routing is fully client-driven and decoupled from the camera: RIGHT-click sets start /
    # freezes, plain hover (no button) moves the live target. The left button and all modifier /
    # wheel gestures stay pure camera (rotate / pan / spin / zoom) -- we touch none of them.
    def _pick_world_fraction(fx, fy):
        """Pick the exact surface position at fractional cursor coords (origin bottom-left)."""
        w, h = plotter.window_size
        return _pick_world(int(fx * w), int(fy * h))

    def _do_hover(fx, fy) -> None:
        pos = _pick_world_fraction(fx, fy)
        # Live, unsnapped cursor readout -- handy to sanity-check the pick (esp. the z / floor).
        state.cursor_pos = _fmt(pos) if pos is not None else "—"
        if pos is None or not route["tracking"]:
            return
        update_route(pos)

    @ctrl.trigger("route_hover")
    async def _route_hover(fx, fy):
        # Backpressure by coalescing: record only the *latest* hover and, while a
        # compute/render is in flight, let further hovers collapse onto it instead
        # of queuing. Otherwise, on slow/large geometry, a backlog of stale
        # positions replays for seconds after the mouse has stopped.
        if current["mesh"] is None:
            return
        route["pending"] = (fx, fy)
        if route["busy"]:
            return
        route["busy"] = True
        try:
            while route["pending"] is not None:
                hx, hy = route["pending"]
                route["pending"] = None
                _do_hover(hx, hy)
                # Yield so hover triggers that arrived during the (blocking) render
                # get dispatched now -- they just overwrite `pending` with the latest.
                await asyncio.sleep(0)
        finally:
            route["busy"] = False

    @ctrl.trigger("route_click")
    def _route_click(fx, fy):
        if current["mesh"] is None:
            return
        if route["tracking"]:
            route["tracking"] = False  # freeze: keep the last path on screen
            state.route_status = "frozen"
            if ctrl.view_update:
                ctrl.view_update()
            return
        pos = _pick_world_fraction(fx, fy)
        if pos is not None:
            set_start(pos)

    with SinglePageWithDrawerLayout(server) as layout:
        layout.title.set_text(TITLE)
        with layout.drawer:
            with v3.VList(density="compact"):
                v3.VListSubheader("Mesh")
                v3.VListItem(
                    "{{ mesh_name || '—' }}",
                    subtitle="File",
                    prepend_icon="mdi-file-outline",
                )
                v3.VListItem("{{ n_points }}", subtitle="Vertices")
                v3.VListItem("{{ n_cells }}", subtitle="Faces")
                v3.VListItem(
                    "{{ z_min.toFixed(2) }} … {{ z_max.toFixed(2) }} m",
                    subtitle="Height range z",
                )
                v3.VDivider(classes="my-2")
                v3.VListSubheader(
                    "Routing  ·  right-click to set start / freeze"
                )
                v3.VListItem("{{ cursor_pos }}", subtitle="Cursor (x, y, z)")
                v3.VListItem("{{ route_status }}", subtitle="State")
                v3.VListItem("{{ route_start }}", subtitle="Start (x, y, z)")
                v3.VListItem("{{ route_target }}", subtitle="Target (x, y, z)")
                v3.VListItem(
                    "{{ route_cost.toFixed(2) }}", subtitle="Path cost"
                )
                v3.VListItem("{{ route_corners }}", subtitle="Waypoints")
                v3.VListItem(
                    "{{ route_setup_us }} µs", subtitle="set_target time"
                )
                v3.VListItem(
                    "{{ route_query_us }} µs", subtitle="get_shortest_path time"
                )
        with layout.toolbar:
            with v3.VMenu(close_on_content_click=False):
                with v3.Template(v_slot_activator="{ props }"):
                    v3.VBtn("File", v_bind="props", variant="text")
                with v3.VList(density="compact", style="min-width: 240px"):
                    # The uploader lives directly in the menu: clicking it opens the native
                    # file dialog. (A JS handler touching `document` fails -- Vue evaluates
                    # event expressions in a sandbox without the global `document`.)
                    v3.VFileInput(
                        v_model=("obj_file", None),
                        accept=".obj",
                        label="Load OBJ…",
                        prepend_icon="mdi-folder-open",
                        density="comfortable",
                        hide_details=True,
                        classes="px-3 pt-2",
                    )
            v3.VSpacer()
            # Height band: range slider with editable low/high fields on each side.
            v3.VTextField(
                v_model_number=("clip_low",),
                type="number",
                density="compact",
                variant="outlined",
                hide_details=True,
                style="max-width: 90px",
            )
            v3.VRangeSlider(
                v_model=("clip_range",),
                min=("z_min",),
                max=("z_max",),
                step=0.1,
                hide_details=True,
                classes="mx-2",
                style="max-width: 280px; min-width: 160px",
            )
            v3.VTextField(
                v_model_number=("clip_high",),
                type="number",
                density="compact",
                variant="outlined",
                hide_details=True,
                style="max-width: 90px",
            )
            v3.VSwitch(
                v_model=("wireframe", True),
                label="Triangulation",
                hide_details=True,
                classes="ml-4",
            )
            with v3.VBtn(icon=True, click=ctrl.reset_view, classes="ml-2"):
                v3.VIcon("mdi-crop-free")
        # Routing input is captured here on the client and forwarded as fractional cursor coords
        # (origin bottom-left) to the route_* triggers. Coords are relative to $event.target -- the
        # actual rendered canvas/img -- via offsetX/offsetY, so the mapping is correct even if the
        # render element is smaller than or offset within its container (else picks land off / on
        # the wrong floor). Hover only tracks when no button is held, so camera drags never move
        # the target; right-click sets start / freezes (and we suppress its menu).
        coords = (
            "[$event.offsetX / $event.target.clientWidth, "
            "1 - $event.offsetY / $event.target.clientHeight]"
        )
        hover_js = f"$event.buttons === 0 && trigger('route_hover', {coords})"
        click_js = f"$event.preventDefault(); trigger('route_click', {coords})"
        with layout.content:
            with v3.VContainer(
                fluid=True,
                classes="fill-height pa-0",
                mousemove=hover_js,
                contextmenu=click_js,
            ):
                # mode="server": pure remote rendering so the camera (left button, modifiers,
                # wheel) is handled server-side. The default "trame" mode interacts client-side.
                view = plotter_ui(plotter, mode="server")
                ctrl.view_update = view.update

    # No initial mesh: start with an empty scene and let the user pick one via the
    # toolbar's "File -> Load OBJ…" dialog.
    if obj_path:
        load_mesh(pv.read(obj_path), os.path.basename(obj_path), obj_path)
    return server


def _wait_until_serving(port: int, timeout: float = 20.0) -> bool:
    """Block until the trame server answers on ``port`` (or give up after ``timeout``)."""
    deadline = time.time() + timeout
    url = f"http://localhost:{port}/index.html"
    while time.time() < deadline:
        try:
            urllib.request.urlopen(url, timeout=0.5)
            return True
        except Exception:
            time.sleep(0.2)
    return False


def run_native(obj_path: str | None, port: int) -> None:
    """Show the trame app in a native pywebview (WebKitGTK) window.

    The trame/aiohttp server installs SIGINT handlers that only work on the main thread, and
    pywebview's GTK loop also wants the main thread -- so the server runs in a *subprocess*
    (its own main thread) while pywebview owns this process's main thread.
    """
    import subprocess
    import sys

    import webview

    child = subprocess.Popen(
        [
            sys.executable,
            __file__,
            *(["--obj", obj_path] if obj_path else []),
            "--port",
            str(port),
            "--no-browser",
        ]
    )
    try:
        if not _wait_until_serving(port):
            raise RuntimeError(f"trame server did not come up on port {port}")
        webview.create_window(TITLE, f"http://localhost:{port}")
        webview.start()
    finally:
        child.terminate()
        try:
            child.wait(timeout=5)
        except subprocess.TimeoutExpired:
            child.kill()


def main() -> None:
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument(
        "--obj",
        default=None,
        help="OBJ file to load on startup",
    )
    parser.add_argument(
        "--native",
        action="store_true",
        help="open a native desktop window (pywebview) instead of a browser tab",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=None,
        help="server port (default: a free port chosen automatically)",
    )
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="serve without opening a browser (used internally by --native)",
    )
    args = parser.parse_known_args()[0]

    port = args.port or find_free_port()

    if args.native:
        run_native(args.obj, port)
        return

    server = build_app(args.obj)
    if not args.no_browser:
        print(f"Serving on http://localhost:{port}  (Ctrl-C to stop)")
    server.start(
        port=port, open_browser=not args.no_browser, show_connection_info=False
    )


if __name__ == "__main__":
    main()
