# SPDX-License-Identifier: LGPL-3.0-or-later
"""3D viewer for a multi-level navigation mesh, via pyvista + trame.

The 3D scene is rendered *offscreen* by VTK (server side) and shown either in the browser or
in a native desktop window (``--native``, via pywebview's WebKitGTK).
"""

import argparse
import asyncio
import atexit
import math
import os
import socket
import tempfile
import time
import urllib.request
from collections.abc import Callable
from dataclasses import dataclass

# Force pyvista offscreen before it (and VTK) initialise: the frame is rendered to an FBO and
# streamed to the client, never into an on-screen native window.
os.environ.setdefault("PYVISTA_OFF_SCREEN", "true")

import numpy as np  # noqa: E402
import pyvista as pv  # noqa: E402
from vtkmodules.vtkRenderingCore import vtkCellPicker  # noqa: E402

pv.OFF_SCREEN = True

import jupedsim as jps  # noqa: E402
from pyvista.trame.ui import plotter_ui  # noqa: E402
from trame.app import asynchronous, get_server  # noqa: E402
from trame.app.file_upload import ClientFile  # noqa: E402
from trame.ui.vuetify3 import SinglePageWithDrawerLayout  # noqa: E402
from trame.widgets import vuetify3 as v3  # noqa: E402

TITLE = "JuPedSim - Multi-Level Viewer"

# BoundaryIndex implementations selectable in Boundary Highlight mode:
# (key stored in state, dropdown title, factory building the index from a Geometry).
BOUNDARY_INDEX_VARIANTS = [
    ("naive", "Naive", jps.pyjupedsim.make_naive_boundary_index),
    ("portal", "Portal", jps.make_portal_boundary_index),
]


@dataclass(frozen=True)
class ViewerMode:
    """One mutually exclusive user interaction mode.

    Registered once in build_app(): the title feeds the mode dropdown and the
    drawer header, build_drawer() emits the mode's drawer rows (called once at
    layout build), reset() tears down all of the mode's state when the user
    switches away from it, and on_right_click() (if any) receives fractional
    canvas coords whenever the user right-clicks while the mode is active.
    """

    key: str  # value stored in state.interaction_mode
    title: str  # dropdown entry and drawer header
    reset: Callable[[], None]
    build_drawer: Callable[[], None]
    on_right_click: Callable[[float, float], None] | None = None


@dataclass(frozen=True)
class Hdf5Scene:
    """What the viewer reads from a file written by jupedsim's Hdf5TrajectoryWriter."""

    vertices: np.ndarray  # [N, 3]
    triangles: np.ndarray  # [M, 3]
    face_regions: np.ndarray  # [M] region id per triangle
    n_regions: int
    # Travel times by stage id, each the grid of every region by region id; None if the
    # file was written without write_travel_times=True.
    travel_times: (
        dict[int, dict[int, jps.Hdf5TrajectoryLoader.TravelTimeGrid]] | None
    )


def read_hdf5(path: str) -> Hdf5Scene:
    import h5py
    import hdf5plugin  # noqa: F401 -- registers the zstd filter the datasets use

    with h5py.File(path, "r") as hf:
        vertices = hf["mesh/vertices"][:].astype(float)
        triangles = hf["mesh/triangles"][:].astype(np.int64)
        face_regions = hf["mesh/regions"][:].astype(np.int64)
        n_regions = int(hf["regions"].shape[0])
    return Hdf5Scene(
        vertices=vertices,
        triangles=triangles,
        face_regions=face_regions,
        n_regions=n_regions,
        travel_times=read_travel_times(path),
    )


def read_travel_times(
    path: str,
) -> dict[int, dict[int, jps.Hdf5TrajectoryLoader.TravelTimeGrid]] | None:
    """All travel times in @p path by stage id, or None if it holds none.

    A file the loader rejects still shows its mesh and agents, just without travel
    times: the reason is printed.
    """
    try:
        with jps.Hdf5TrajectoryLoader(path) as loader:
            if not loader.has_travel_times():
                return None
            return {
                stage: loader.load_travel_times(stage)
                for stage in loader.load_travel_time_stage_ids()
            }
    except jps.Hdf5TrajectoryLoader.Error as e:
        print(f"{path}: no travel times shown: {e}")
        return None


class Recording:
    """The recorded agent positions of an HDF5 file, read one frame at a time.

    Keeps the file open until close(). Frame i holds the rows
    trajectory[frame_offsets[i]:frame_offsets[i + 1]]; a file whose run ended before the
    writer was closed may lack the end of /frame_offsets, which is then rebuilt from the
    rows' frame numbers.

    With @p delete_on_close the recording owns the file, e.g. a temporary copy of an
    upload, and deletes it in close(): not before, as Windows cannot delete a file that is
    still open. If the constructor raises, the file stays the caller's.
    """

    def __init__(self, path: str, delete_on_close: bool = False):
        import h5py
        import hdf5plugin  # noqa: F401 -- registers the zstd filter the datasets use

        self.path = path
        self._delete_on_close = False
        self._file = h5py.File(path, "r")
        try:
            self._trajectory = self._file["trajectory"]
            offsets = self._file["frame_offsets"][:]
            n_rows = self._trajectory.shape[0]
            if len(offsets) == 0 or offsets[-1] != n_rows:
                frames = self._trajectory.fields("frame")[:]
                n = int(frames[-1]) + 1 if n_rows else 0
                offsets = np.searchsorted(frames, np.arange(n + 1))
            self._offsets = offsets.astype(np.int64)
            # Seconds between two recorded frames.
            self.frame_dt = float(self._file.attrs["dt"]) * int(
                self._file.attrs["every_nth_frame"]
            )
        except BaseException:
            self._file.close()
            raise
        self._delete_on_close = delete_on_close

    @property
    def n_frames(self) -> int:
        return len(self._offsets) - 1

    def positions(self, frame: int) -> np.ndarray:
        """[N, 3] positions of the agents in @p frame."""
        begin, end = self._offsets[frame], self._offsets[frame + 1]
        rows = self._trajectory.fields(["x", "y", "z"])[begin:end]
        return np.column_stack([rows["x"], rows["y"], rows["z"]]).astype(float)

    def close(self) -> None:
        """Close the file and, if the recording owns it, delete it. Safe to call more
        than once."""
        self._file.close()
        if self._delete_on_close:
            self._delete_on_close = False
            os.unlink(self.path)


def travel_time_surface(
    grids: dict,
    vertices: np.ndarray,
    triangles: np.ndarray,
    face_regions: np.ndarray,
    lift: float,
) -> pv.PolyData | None:
    """One quad per walkable cell of @p grids, lying on its region's surface.

    @p grids holds the grid of every region by region id, for one stage: as
    Hdf5TrajectoryLoader.load_travel_times or Simulation.travel_times return them. Every
    region of a WalkableSurface is planar - floors are flat, stairs and ramps are planar
    quads - so a cell takes its region's plane and cells on stairs and ramps follow the
    slope; @p lift raises the quads above the mesh to keep them from z-fighting.
    Carries the cell scalar "travel_time" and the point scalar "elevation" (for clipping),
    the cells' position on the lattice all grids share as "lattice_row" / "lattice_col",
    and "direction": the walking direction tilted onto the cell's plane, zero inside the
    destination.
    """
    points, travel_times, cell_directions, lattice = [], [], [], []
    for region_id, grid in grids.items():
        cell_size = float(grid.cell_size)
        origin_x, origin_y = (float(v) for v in grid.origin)
        tt = grid.values
        rows, cols = np.nonzero(np.isfinite(tt))
        region_tris = triangles[face_regions == region_id]
        if len(rows) == 0 or len(region_tris) == 0:
            continue
        cx = origin_x + (cols + 0.5) * cell_size
        cy = origin_y + (rows + 0.5) * cell_size

        # The region's plane z = a * x + b * y + c, fitted to its vertices.
        p = vertices[np.unique(region_tris)]
        a, b, c = np.linalg.lstsq(
            np.column_stack([p[:, 0], p[:, 1], np.ones(len(p))]),
            p[:, 2],
            rcond=None,
        )[0]

        # The 4 corners of every cell, counterclockwise, as consecutive points.
        half = cell_size / 2
        corners = []
        for dx, dy in (
            (-half, -half),
            (half, -half),
            (half, half),
            (-half, half),
        ):
            x, y = cx + dx, cy + dy
            z = a * x + b * y + c + lift
            corners.append(np.column_stack([x, y, z]))
        points.append(np.stack(corners, axis=1).reshape(-1, 3))
        travel_times.append(tt[rows, cols])
        # The grids share one lattice, so the origin is a whole number of cells away
        # from the lattice's own: the same rounding holds for every grid.
        lattice.append(
            np.column_stack(
                [
                    rows + int(np.rint(origin_y / cell_size)),
                    cols + int(np.rint(origin_x / cell_size)),
                ]
            )
        )
        d = grid.directions
        dx, dy = (
            d[rows, cols, 0].astype(float),
            d[rows, cols, 1].astype(float),
        )
        # Along the plane z = a x + b y + c, so arrows follow stairs and ramps.
        dz = a * dx + b * dy
        cell_directions.append(np.column_stack([dx, dy, dz]))

    if not travel_times:
        return None
    pts = np.vstack(points)
    n = len(pts) // 4
    quads = np.arange(4 * n, dtype=np.int64).reshape(n, 4)
    faces = np.hstack([np.full((n, 1), 4, dtype=np.int64), quads]).ravel()
    surface = pv.PolyData(pts, faces)
    surface.cell_data["travel_time"] = np.concatenate(travel_times)
    lattice_rc = np.vstack(lattice)
    surface.cell_data["lattice_row"] = lattice_rc[:, 0]
    surface.cell_data["lattice_col"] = lattice_rc[:, 1]
    surface.cell_data["direction"] = np.vstack(cell_directions)
    surface["elevation"] = pts[:, 2] - lift
    return surface


def direction_arrows(
    surface: pv.PolyData, stride: int, length: float, lift: float
):
    """Arrows along the "direction" of every @p stride-th cell of @p surface in both
    lattice directions, centred on the cells and @p lift above them; None if no cell
    qualifies. Cells inside the destination (zero direction) get no arrow."""
    if "direction" not in surface.cell_data or surface.n_cells == 0:
        return None
    directions = surface.cell_data["direction"]
    keep = (
        (surface.cell_data["lattice_row"] % stride == 0)
        & (surface.cell_data["lattice_col"] % stride == 0)
        & (np.linalg.norm(directions[:, :2], axis=1) > 0)
    )
    if not keep.any():
        return None
    centres = surface.cell_centers().points[keep]
    centres[:, 2] += lift
    anchors = pv.PolyData(centres)
    anchors["direction"] = directions[keep]
    arrow = pv.Arrow(
        start=(-0.5, 0.0, 0.0),
        direction=(1.0, 0.0, 0.0),
        tip_length=0.35,
        tip_radius=0.15,
        shaft_radius=0.05,
        tip_resolution=8,
        shaft_resolution=8,
    )
    return anchors.glyph(
        orient="direction", scale=False, factor=length, geom=arrow
    )


def find_free_port() -> int:
    """Ask the OS for an unused TCP port (bind to :0, read it back, release it)."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def build_app(obj_path: str | None, h5_path: str | None = None):
    """Build the trame server hosting a pyvista Plotter"""
    plotter = pv.Plotter()
    plotter.set_background("white")
    plotter.enable_parallel_projection()
    # Terrain-style interaction: keeps +z as a fixed "up" (no roll / flipping upside down), which
    # is far easier to handle with our geometries than the default trackball.
    plotter.enable_terrain_style(mouse_wheel_zooms=True, shift_pans=True)

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
    state.color_by_region = (
        False  # False -> colour by height, True -> by region id
    )
    state.mesh_name = ""
    state.obj_file = None
    state.n_points = 0
    state.n_cells = 0
    state.n_regions = 0
    # Mutually exclusive interaction mode, selected in the toolbar dropdown.
    # Keys come from the ViewerMode registry (`modes`) below.
    state.interaction_mode = "none"
    # Routing (Pathfinding mode): a right-click sets the start, then the hovered point is the live
    # target, and the next right-click freezes the path. Hover only tracks while no button is held,
    # so the left button and all modifier / wheel gestures stay pure camera (rotate / pan / spin /
    # zoom).
    state.route_status = "idle"  # idle | tracking | frozen
    state.cursor_pos = "—"  # live, unsnapped surface position under the cursor
    state.route_start = "—"
    state.route_target = "—"
    state.route_cost = 0.0
    state.route_corners = 0
    state.route_query_us = 0  # get_shortest_path() full solve time
    # Boundary Highlight mode: a right-click sets the query point; all boundary line segments
    # within the search distance are highlighted.
    state.boundary_distance = 2.0  # BoundaryIndex.query() search radius [m]
    state.boundary_index_variant = "naive"  # key into BOUNDARY_INDEX_VARIANTS
    state.boundary_query = "—"  # query point of the highlighted result
    state.boundary_count = 0  # number of highlighted segments
    state.boundary_query_us = 0  # BoundaryIndex.query() solve time
    # Pathfinding and Boundary Highlight need a jps.Geometry, which only an OBJ provides; an
    # HDF5 file brings just its mesh.
    state.has_geometry = False
    # Travel Times mode: floor field travel times from an HDF5 file's /travel_times.
    state.travel_available = False
    state.travel_stages = []  # dropdown items, one per stage in the file
    state.travel_stage = None  # selected stage id
    state.travel_max = (
        0.0  # largest finite travel time of the selected stage [s]
    )
    state.travel_pick = "—"  # travel time at the last right-clicked point
    state.travel_arrows = True  # draw the walking directions as arrows
    state.travel_arrow_stride = (
        0  # an arrow every n-th cell; 0 picks n by cell count
    )
    # Replay of the agents recorded in an HDF5 file. Independent of the interaction mode.
    state.replay_available = False
    state.replay_n_frames = 0
    state.replay_frame = 0
    state.replay_playing = False
    state.replay_speed = 1.0  # playback speed relative to simulated time
    state.replay_time = "—"
    state.replay_agents = 0  # agents in the shown frame, including clipped ones

    # Holds the currently loaded geometry + mesh + its height range; swapped out by load_mesh().
    current: dict = {
        "mesh": None,
        "geometry": None,
        "n_regions": 0,
        "marker_r": 0.1,
        "vertices": None,
        "triangles": None,
        "face_regions": None,
    }
    # Travel times of the loaded HDF5 file (see Hdf5Scene.travel_times), and the overlay
    # surface per stage, built on first display.
    travel: dict = {
        "grids": None,
        "surfaces": {},
    }
    # The open Recording of the loaded HDF5 file, the point set the agents actor draws
    # (updated in place per frame), and the running playback task.
    replay: dict = {"recording": None, "points": None, "task": None}

    @atexit.register
    def _close_recording() -> None:
        # The recording of an upload deletes its temporary copy when closed: close the
        # last one on exit so the copy does not outlive the viewer.
        if replay["recording"] is not None:
            replay["recording"].close()

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
    # Boundary Highlight interaction state. Each index variant is built lazily
    # on its first query (users may never enter the mode / switch variants) and
    # cached per variant; all are dropped on mesh load.
    boundary: dict = {"indices": {}, "loc": None}
    engine = None  # Routing engine - populated in load_mesh()
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
        # survive a clip/wireframe change; drop old scalar bars to avoid stacking copies.
        for title in ("Height z [m]", "Region", "Travel time [s]"):
            try:
                plotter.remove_scalar_bar(title=title)
            except (KeyError, StopIteration):
                pass
        by_region = bool(state.color_by_region) and current["n_regions"] > 0
        if (
            state.interaction_mode == "travel_times"
            and state.travel_stage is not None
        ):
            # The travel time overlay carries the colour; the mesh is just the floor under it.
            plotter.add_mesh(
                sub,
                name="mesh",
                color="lightgray",
                show_edges=bool(state.wireframe),
                edge_color="black",
            )
        elif by_region:
            n = current["n_regions"]
            # region id is a per-cell scalar; a qualitative map + discrete colours
            # so adjacent regions stay visually distinct.
            plotter.add_mesh(
                sub,
                name="mesh",
                scalars="region",
                cmap="tab20",
                n_colors=max(n, 1),
                clim=(0, max(n - 1, 1)),
                show_edges=bool(state.wireframe),
                edge_color="black",
                scalar_bar_args=dict(title="Region"),
            )
        else:
            plotter.add_mesh(
                sub,
                name="mesh",
                scalars="elevation",
                cmap="viridis",
                show_edges=bool(state.wireframe),
                edge_color="black",
                scalar_bar_args=dict(title="Height z [m]"),
            )
        _draw_travel_times(lo, hi)
        _draw_agents()
        plotter.add_axes()

    def _draw_travel_times(lo: float, hi: float) -> None:
        plotter.remove_actor("travel_times", render=False)
        plotter.remove_actor("travel_arrows", render=False)
        stage = state.travel_stage
        if state.interaction_mode != "travel_times" or stage is None:
            return
        surface = travel["surfaces"].get(stage)
        if surface is None:
            surface = travel["surfaces"][stage] = travel_time_surface(
                travel["grids"][stage],
                current["vertices"],
                current["triangles"],
                current["face_regions"],
                lift=0.5 * current["marker_r"],
            )
        if surface is None:
            return
        if lo > current["z_min"] or hi < current["z_max"]:
            surface = surface.threshold((lo, hi), scalars="elevation")
            if surface.n_cells == 0:
                return
        times = surface.cell_data["travel_time"]
        # A cell whose centre lies a hair inside a wall has a speed near zero and a
        # travel time orders of magnitude above the rest; scaling to the true maximum
        # would squeeze every other cell into the lowest colour. Cells above the
        # percentile take the top colour; the drawer shows the true maximum.
        state.travel_max = float(times.max())
        plotter.add_mesh(
            surface,
            name="travel_times",
            scalars="travel_time",
            cmap="plasma",
            clim=(0.0, max(float(np.percentile(times, 99.9)), 1e-6)),
            show_edges=False,
            scalar_bar_args=dict(title="Travel time [s]"),
        )
        if state.travel_arrows:
            _draw_travel_arrows(surface)

    def _draw_travel_arrows(surface) -> None:
        # All grids share one lattice, so any of them has the cell size.
        stage_grids = travel["grids"][state.travel_stage]
        cell_size = float(next(iter(stage_grids.values())).cell_size)
        stride = int(state.travel_arrow_stride or 0)
        if stride <= 0:
            # About 3000 arrows at most: enough to read the flow over the whole
            # geometry without the arrows merging into a hatching.
            stride = max(1, math.ceil(math.sqrt(surface.n_cells / 3000)))
        arrows = direction_arrows(
            surface,
            stride,
            length=0.8 * stride * cell_size,
            lift=0.25 * current["marker_r"],
        )
        if arrows is not None:
            plotter.add_mesh(
                arrows, name="travel_arrows", color="white", pickable=False
            )

    def _draw_agents() -> None:
        recording = replay["recording"]
        if recording is None or recording.n_frames == 0:
            plotter.remove_actor("agents", render=False)
            replay["points"] = None
            return
        frame = min(max(int(state.replay_frame), 0), recording.n_frames - 1)
        pts = recording.positions(frame)
        state.replay_agents = len(pts)
        state.replay_time = (
            f"{frame * recording.frame_dt:.2f} s"
            f" / {(recording.n_frames - 1) * recording.frame_dt:.2f} s"
        )
        lo, hi = (float(v) for v in state.clip_range)
        pts = pts[(pts[:, 2] >= lo) & (pts[:, 2] <= hi)]
        if len(pts) == 0:
            plotter.remove_actor("agents", render=False)
            replay["points"] = None
            return
        # Lift above the mesh and the travel time overlay.
        pts[:, 2] += current["marker_r"]
        new = pv.PolyData(pts)
        if replay["points"] is None:
            replay["points"] = new
            plotter.add_mesh(
                new,
                name="agents",
                color="black",
                render_points_as_spheres=True,
                point_size=10,
                pickable=False,
            )
        else:
            replay["points"].copy_from(new)

    @state.change("replay_frame")
    def _on_replay_frame(**_):
        _draw_agents()
        if ctrl.view_update:
            ctrl.view_update()

    async def _play() -> None:
        """Advance replay_frame along the wall clock until paused or at the last frame.

        Frames are picked by elapsed time, not counted, so a slow render skips frames
        instead of slowing the replay down. Scrubbing or a new speed re-anchors the clock.
        """
        recording = replay["recording"]
        last = recording.n_frames - 1
        if int(state.replay_frame) >= last:
            state.replay_frame = 0
        anchor = (
            time.perf_counter(),
            int(state.replay_frame),
            float(state.replay_speed),
        )
        shown = anchor[1]
        while state.replay_playing and replay["recording"] is recording:
            if (
                int(state.replay_frame) != shown
                or float(state.replay_speed) != anchor[2]
            ):
                anchor = (
                    time.perf_counter(),
                    int(state.replay_frame),
                    float(state.replay_speed),
                )
            t0, start, speed = anchor
            frame = start + int(
                (time.perf_counter() - t0) * speed / recording.frame_dt
            )
            with state:
                if frame >= last:
                    state.replay_frame = last
                    state.replay_playing = False
                elif frame != shown:
                    state.replay_frame = frame
            shown = int(state.replay_frame)
            await asyncio.sleep(1 / 60)

    @state.change("replay_playing")
    def _on_replay_playing(replay_playing, **_):
        task = replay["task"]
        if (
            replay_playing
            and replay["recording"] is not None
            and (task is None or task.done())
        ):
            replay["task"] = asynchronous.create_task(_play())

    def _set_recording(recording) -> None:
        """Swap in @p recording (or None) before a scene is shown; see _publish_replay."""
        state.replay_playing = False  # a running _play() sees the swap and ends
        if replay["recording"] is not None:
            replay["recording"].close()
        replay.update(recording=recording, points=None)
        plotter.remove_actor("agents", render=False)
        state.replay_frame = 0

    def _publish_replay() -> None:
        recording = replay["recording"]
        with state:
            state.replay_available = (
                recording is not None and recording.n_frames > 0
            )
            state.replay_n_frames = recording.n_frames if recording else 0
            state.replay_frame = 0
            state.replay_time = "—"
            state.replay_agents = 0

    def load_mesh(obj_path: str, name: str) -> None:
        nonlocal engine
        # Geometry is the single source of truth: the rendered vertices/faces
        # and the region colouring are read from it, and the engine borrows the
        # SAME instance, so routing, picking and region colouring all agree.
        geo = jps.Geometry.from_obj(obj_path)
        engine = jps.SurfaceMeshShortestPathRoutingEngine(geo)
        travel.update(grids=None, surfaces={})
        _set_recording(None)
        show_scene(
            np.asarray(geo.vertices(), dtype=float),
            np.asarray(geo.triangles(), dtype=np.int64),
            np.asarray(geo.region_id_per_face(), dtype=np.int64),
            int(geo.region_count()),
            geo,
            name,
        )
        _publish_travel_stages()
        _publish_replay()

    def load_hdf5(h5_path: str, name: str, temporary: bool = False) -> None:
        """Show @p h5_path; with @p temporary its recording deletes it once closed."""
        nonlocal engine
        scene = read_hdf5(h5_path)
        engine = None  # no Geometry to route on
        # Swap the data in before the mesh is drawn, and publish the stages only after, so
        # no redraw ever pairs one file's grids with another file's mesh.
        travel.update(grids=scene.travel_times, surfaces={})
        _set_recording(Recording(h5_path, delete_on_close=temporary))
        show_scene(
            scene.vertices,
            scene.triangles,
            scene.face_regions,
            scene.n_regions,
            None,
            name,
        )
        _publish_travel_stages()
        _publish_replay()

    def _publish_travel_stages() -> None:
        grids = travel["grids"]
        stages = sorted(grids) if grids is not None else []
        with state:
            state.travel_available = grids is not None
            state.travel_stages = [
                {"title": f"Stage {s}", "value": s} for s in stages
            ]
            state.travel_stage = stages[0] if stages else None

    def show_scene(verts, tris, face_region_ids, n_regions, geo, name) -> None:
        """Show a mesh; @p geo is its jps.Geometry, or None if there is none."""
        # pyvista face stream: [3, i, j, k, 3, i, j, k, ...]
        faces = np.hstack(
            [np.full((len(tris), 1), 3, dtype=np.int64), tris]
        ).ravel()
        mesh = pv.PolyData(verts, faces)
        mesh["elevation"] = verts[
            :, 2
        ]  # colour + clip by height (point scalar)
        if len(face_region_ids) == mesh.n_cells:
            # per-face region id
            mesh.cell_data["region"] = np.asarray(face_region_ids, dtype=float)
        else:  # face-count mismatch: disable region colouring rather than mislabel
            mesh.cell_data["region"] = np.zeros(mesh.n_cells, dtype=float)
            n_regions = 0

        z_min = float(verts[:, 2].min())
        z_max = float(verts[:, 2].max())
        if z_max <= z_min:  # flat mesh: keep the slider non-degenerate
            z_max = z_min + 1.0
        diag = float(
            np.linalg.norm(
                np.asarray(mesh.bounds[1::2]) - np.asarray(mesh.bounds[::2])
            )
        )
        current.update(
            mesh=mesh,
            geometry=geo,
            vertices=verts,
            triangles=tris,
            face_regions=np.asarray(face_region_ids, dtype=np.int64),
            n_regions=n_regions,
            z_min=z_min,
            z_max=z_max,
            marker_r=max(0.01 * diag, 0.05),
        )
        boundary["indices"].clear()  # segment grids of the previous mesh
        for m in modes:
            m.reset()  # a new mesh invalidates every mode's picks/overlays
        with state:
            state.z_min, state.z_max = z_min, z_max
            state.clip_range = [z_min, z_max]  # start showing the whole mesh
            state.clip_low, state.clip_high = z_min, z_max
            state.mesh_name = name
            state.n_points = int(mesh.n_points)
            state.n_cells = int(mesh.n_cells)
            state.n_regions = n_regions
            state.has_geometry = geo is not None
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

    @state.change("clip_range", "wireframe", "color_by_region")
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
        # Both readers want a real file with the right extension.
        suffix = os.path.splitext(client_file.name)[1].lower()
        with tempfile.NamedTemporaryFile(suffix=suffix, delete=False) as tmp:
            tmp.write(client_file.content)
            tmp_path = tmp.name
        if suffix not in (".h5", ".hdf5"):
            try:
                load_mesh(tmp_path, client_file.name)
            finally:
                # The mesh is read in full: nothing holds the file open any more.
                os.unlink(tmp_path)
            return
        try:
            # The Recording reads from the file until it is closed, so it deletes the
            # file then: Windows cannot delete a file that is still open.
            load_hdf5(tmp_path, client_file.name, temporary=True)
        except BaseException:
            recording = replay["recording"]
            if recording is None or recording.path != tmp_path:
                os.unlink(tmp_path)  # failed before the Recording took the file
            raise

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
            state.route_query_us = 0

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
                new,
                name="route_path",
                color="red",
                line_width=6,
                pickable=False,
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
        # The fixed start is the geodesic source, the live hovered point the target.
        src = tuple(float(v) for v in start_pos)
        tgt = tuple(float(v) for v in target_pos)
        t0 = time.perf_counter()
        try:
            path = engine.get_shortest_path(src, tgt)
        except Exception:
            return  # off-surface pick or no path
        elapsed_us = (time.perf_counter() - t0) * 1e6
        if len(path) < 2:
            return
        pts = np.asarray(path, dtype=float)
        # The engine hands out the way, not what it costs: measure it along the surface here.
        cost = float(np.linalg.norm(np.diff(pts, axis=0), axis=1).sum())
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
        # Just fix the start; the geodesic runs per hover in update_route()
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
        if ctrl.view_update:
            ctrl.view_update()

    # Routing is only active in Pathfinding mode. It is fully client-driven and decoupled from
    # the camera: RIGHT-click sets start / freezes, plain hover (no button) moves the live
    # target. The left button and all modifier / wheel gestures stay pure camera (rotate / pan /
    # spin / zoom) -- we touch none of them.
    def _pick_world_fraction(fx, fy):
        """Pick the exact surface position at fractional cursor coords (origin bottom-left)."""
        # The client divides by the canvas size; before layout (or mid-detach) that
        # size can be 0, making fx/fy inf/nan -> int() would overflow. Skip those.
        if not (math.isfinite(fx) and math.isfinite(fy)):
            return None
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
        # The client already gates on the mode; re-check here since its state
        # can be stale mid-switch.
        if state.interaction_mode != "pathfinding":
            return
        if current["mesh"] is None or engine is None:
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

    def _pathfinding_click(fx, fy):
        if current["mesh"] is None or engine is None:
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

    # -- boundary highlight via BoundaryIndex --------------------------

    def _draw_boundary_segments(segments, z: float) -> None:
        plotter.remove_actor("boundary_segments", render=False)
        if not segments:
            return
        # The index hands out 2D ((x1, y1), (x2, y2)) segments; draw them at
        # the query point's height, slightly lifted so they never z-fight with
        # the floor. Exact on flat floors, approximate on ramps / stairs.
        lift = z + 0.05
        pts = np.array([(*p, lift) for seg in segments for p in seg])
        pairs = np.arange(len(pts)).reshape(-1, 2)
        cells = np.hstack(
            [np.full((len(pairs), 1), 2, dtype=np.int64), pairs]
        ).ravel()
        plotter.add_mesh(
            pv.PolyData(pts, lines=cells),
            name="boundary_segments",
            color="magenta",
            line_width=6,
            pickable=False,
        )

    def _draw_boundary_radius(center, radius: float, z: float) -> None:
        plotter.remove_actor("boundary_radius", render=False)
        if radius <= 0.0:
            return
        # The search disc as a closed polyline around the query point, at the
        # same lift as the highlighted segments.
        angles = np.linspace(0.0, 2.0 * np.pi, 129)
        pts = np.column_stack(
            [
                center[0] + radius * np.cos(angles),
                center[1] + radius * np.sin(angles),
                np.full(len(angles), z + 0.05),
            ]
        )
        pairs = np.column_stack(
            [np.arange(len(pts) - 1), np.arange(1, len(pts))]
        )
        cells = np.hstack(
            [np.full((len(pairs), 1), 2, dtype=np.int64), pairs]
        ).ravel()
        plotter.add_mesh(
            pv.PolyData(pts, lines=cells),
            name="boundary_radius",
            color="lightgray",
            line_width=1,
            pickable=False,
        )

    def _run_boundary_query() -> None:
        loc = boundary["loc"]
        if loc is None:
            return
        try:
            distance = max(float(state.boundary_distance), 0.0)
        except (TypeError, ValueError):
            return  # mid-typing / empty distance field
        variant = state.boundary_index_variant
        index = boundary["indices"].get(variant)
        if index is None:
            factory = next(
                f for k, _, f in BOUNDARY_INDEX_VARIANTS if k == variant
            )
            index = boundary["indices"][variant] = factory(current["geometry"])
        t0 = time.perf_counter()
        segments = index.query(loc, distance)
        elapsed_us = (time.perf_counter() - t0) * 1e6
        _draw_boundary_segments(segments, loc.z)
        _draw_boundary_radius((loc.x, loc.y), distance, loc.z)
        with state:
            state.boundary_query = _fmt((loc.x, loc.y, loc.z))
            state.boundary_count = len(segments)
            state.boundary_query_us = int(elapsed_us)
        if ctrl.view_update:
            ctrl.view_update()

    def _boundary_click(fx, fy):
        if current["geometry"] is None:
            return
        pos = _pick_world_fraction(fx, fy)
        if pos is None:
            return
        # Read the region from the picked cell's data, not via its index: clipping by
        # height renumbers the cells.
        regions = picker.GetDataSet().GetCellData().GetArray("region")
        region_id = int(regions.GetValue(picker.GetCellId()))
        try:
            loc = current["geometry"].get_location(
                float(pos[0]), float(pos[1]), region_id=region_id
            )
        except jps.SimulationError:
            return  # pick rounds off the region's edge
        boundary["loc"] = loc
        _place_marker("boundary_query", pos, "dodgerblue")
        _run_boundary_query()

    @state.change("boundary_distance", "boundary_index_variant")
    def _on_boundary_inputs(**_):
        # Re-run the query live while a point is set; parsing (and clamping)
        # of the field value happens inside _run_boundary_query().
        if state.interaction_mode == "boundary":
            _run_boundary_query()

    def _reset_boundary() -> None:
        # The query point and highlights are mode state; the search distance
        # is a user setting and survives leaving the mode.
        boundary["loc"] = None
        for actor_name in (
            "boundary_segments",
            "boundary_radius",
            "boundary_query",
        ):
            plotter.remove_actor(actor_name, render=False)
        with state:
            state.boundary_query = "—"
            state.boundary_count = 0
            state.boundary_query_us = 0

    # -- travel times from an HDF5 file ---------------------------------

    def _travel_click(fx, fy):
        if state.travel_stage is None:
            return
        pos = _pick_world_fraction(fx, fy)
        if pos is None:
            return
        # The overlay is lifted above the mesh, so a pick from above hits it first; a pick
        # on the bare mesh (a cell that is not walkable) has no travel time.
        times = picker.GetDataSet().GetCellData().GetArray("travel_time")
        if times is None:
            state.travel_pick = "—"
            return
        _place_marker("travel_query", pos, "dodgerblue")
        cell = picker.GetCellId()
        pick = f"{times.GetValue(cell):.2f} s at {_fmt(pos)}"
        directions = picker.GetDataSet().GetCellData().GetArray("direction")
        if directions is not None:
            dx, dy, _ = directions.GetTuple3(cell)
            pick += (
                f", heading ({dx:.2f}, {dy:.2f})" if dx or dy else ", arrived"
            )
        state.travel_pick = pick
        if ctrl.view_update:
            ctrl.view_update()

    @state.change("travel_stage")
    def _on_travel_stage(**_):
        if state.interaction_mode == "travel_times":
            _reset_travel_pick()
            _redraw()

    @state.change("travel_arrows", "travel_arrow_stride")
    def _on_travel_arrows(**_):
        if state.interaction_mode == "travel_times":
            _redraw()

    def _reset_travel_pick() -> None:
        plotter.remove_actor("travel_query", render=False)
        state.travel_pick = "—"

    def _reset_travel() -> None:
        # The stage selection is a user setting and survives leaving the mode. The overlay
        # itself goes with the next draw(), which only draws it in this mode.
        _reset_travel_pick()

    def _build_travel_drawer() -> None:
        with v3.Template(v_if="!travel_available"):
            v3.VListItem(
                "Load an HDF5 file written with write_travel_times=True",
                subtitle="No travel times",
            )
        with v3.Template(v_else=True):
            v3.VListItem("right-click reads the travel time", subtitle="Usage")
            v3.VSelect(
                v_model=("travel_stage", None),
                items=("travel_stages", []),
                label="Stage",
                density="compact",
                variant="outlined",
                hide_details=True,
                classes="mx-4 mb-2",
            )
            v3.VListItem("{{ travel_pick }}", subtitle="Travel time")
            v3.VSwitch(
                v_model=("travel_arrows", True),
                label="Walking directions",
                hide_details=True,
                density="compact",
                classes="mx-4",
            )
            v3.VSelect(
                v_model=("travel_arrow_stride", 0),
                items=(
                    "travel_arrow_strides",
                    [{"title": "Auto", "value": 0}]
                    + [
                        {
                            "title": f"every {n} cells"
                            if n > 1
                            else "every cell",
                            "value": n,
                        }
                        for n in (1, 2, 4, 8, 16, 32)
                    ],
                ),
                label="Arrow spacing",
                disabled=("!travel_arrows",),
                density="compact",
                variant="outlined",
                hide_details=True,
                classes="mx-4 mb-2",
            )
            v3.VListItem(
                "{{ travel_max.toFixed(1) }} s",
                subtitle="Largest travel time (colours capped at the 99.9th percentile)",
            )

    # -- interaction modes ---------------------------------------------
    # To add a mode: append a ViewerMode entry, gate its input handlers on
    # state.interaction_mode, and make reset() undo everything the mode did.

    def _reset_pathfinding() -> None:
        route["pending"] = None
        clear_route()
        state.cursor_pos = "—"

    def _build_none_drawer() -> None:
        v3.VListItem("No interaction active", subtitle="Pick a mode above")

    def _build_needs_geometry_hint() -> None:
        v3.VListItem(
            "needs an OBJ file; an HDF5 file brings no geometry",
            subtitle="Unavailable",
            v_if="!has_geometry",
        )

    def _build_pathfinding_drawer() -> None:
        _build_needs_geometry_hint()
        v3.VListItem(
            "right-click sets start / freezes the path", subtitle="Usage"
        )
        v3.VListItem("{{ cursor_pos }}", subtitle="Cursor (x, y, z)")
        v3.VListItem("{{ route_status }}", subtitle="State")
        v3.VListItem("{{ route_start }}", subtitle="Start (x, y, z)")
        v3.VListItem("{{ route_target }}", subtitle="Target (x, y, z)")
        v3.VListItem("{{ route_cost.toFixed(2) }}", subtitle="Path cost")
        v3.VListItem("{{ route_corners }}", subtitle="Waypoints")
        v3.VListItem(
            "{{ route_query_us }} µs", subtitle="get_shortest_path time"
        )

    def _build_boundary_drawer() -> None:
        _build_needs_geometry_hint()
        v3.VListItem("right-click sets the query point", subtitle="Usage")
        v3.VSelect(
            v_model=("boundary_index_variant", "naive"),
            items=(
                "boundary_index_variants",
                [
                    {"title": title, "value": key}
                    for key, title, _ in BOUNDARY_INDEX_VARIANTS
                ],
            ),
            label="Index variant",
            density="compact",
            variant="outlined",
            hide_details=True,
            classes="mx-4 mb-2",
        )
        v3.VSlider(
            v_model=("boundary_distance", 2.0),
            min=0.1,
            max=20.0,
            step=0.1,
            hide_details=True,
            classes="mx-4",
        )
        v3.VTextField(
            v_model_number=("boundary_distance",),
            label="Search distance [m]",
            type="number",
            density="compact",
            variant="outlined",
            hide_details=True,
            classes="mx-4 mt-1",
        )
        v3.VListItem("{{ boundary_query }}", subtitle="Query point (x, y, z)")
        v3.VListItem("{{ boundary_count }}", subtitle="Segments")
        v3.VListItem(
            "{{ boundary_query_us }} µs", subtitle="BoundaryIndex.query time"
        )

    modes = [
        ViewerMode("none", "None", lambda: None, _build_none_drawer),
        ViewerMode(
            "pathfinding",
            "Pathfinding",
            _reset_pathfinding,
            _build_pathfinding_drawer,
            on_right_click=_pathfinding_click,
        ),
        ViewerMode(
            "boundary",
            "Boundary Highlight",
            _reset_boundary,
            _build_boundary_drawer,
            on_right_click=_boundary_click,
        ),
        ViewerMode(
            "travel_times",
            "Travel Times",
            _reset_travel,
            _build_travel_drawer,
            on_right_click=_travel_click,
        ),
    ]
    mode_by_key = {m.key: m for m in modes}
    active_mode = {"key": state.interaction_mode}

    @ctrl.trigger("mode_click")
    def _mode_click(fx, fy):
        # Right-clicks arrive here whenever any mode is active (client-side
        # gate) and are dispatched to the mode's handler.
        mode = mode_by_key.get(state.interaction_mode)
        if mode is not None and mode.on_right_click is not None:
            mode.on_right_click(fx, fy)

    @state.change("interaction_mode")
    def _on_mode(interaction_mode, **_):
        prev, active_mode["key"] = active_mode["key"], interaction_mode
        if prev == interaction_mode:
            return
        mode_by_key[prev].reset()  # leaving a mode drops all of its state
        _redraw()  # a mode may draw into the scene (Travel Times)

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
                v3.VListItem("{{ n_regions }}", subtitle="Regions")
                v3.VListItem(
                    "{{ z_min.toFixed(2) }} … {{ z_max.toFixed(2) }} m",
                    subtitle="Height range z",
                )
                v3.VDivider(classes="my-2")
                with v3.Template(v_if="replay_available"):
                    v3.VListSubheader("Replay")
                    with v3.VListItem():
                        with v3.Template(v_slot_prepend=True):
                            with v3.VBtn(
                                icon=True,
                                variant="text",
                                density="comfortable",
                                click="replay_playing = !replay_playing",
                            ):
                                v3.VIcon(
                                    icon=(
                                        "replay_playing ? 'mdi-pause' : 'mdi-play'",
                                    )
                                )
                        v3.VSlider(
                            v_model=("replay_frame", 0),
                            min=0,
                            max=("replay_n_frames - 1",),
                            step=1,
                            hide_details=True,
                        )
                    v3.VListItem("{{ replay_time }}", subtitle="Time")
                    v3.VListItem("{{ replay_agents }}", subtitle="Agents")
                    v3.VSelect(
                        v_model=("replay_speed", 1.0),
                        items=(
                            "replay_speeds",
                            [
                                {"title": f"{v:g}×", "value": v}
                                for v in (0.25, 0.5, 1.0, 2.0, 5.0, 10.0)
                            ],
                        ),
                        label="Speed",
                        density="compact",
                        variant="outlined",
                        hide_details=True,
                        classes="mx-4 mb-2",
                    )
                    v3.VDivider(classes="my-2")
                # One block per mode; only the selected mode's drawer is
                # rendered, always headed by that mode's name.
                for m in modes:
                    with v3.Template(v_if=f"interaction_mode === '{m.key}'"):
                        v3.VListSubheader(m.title)
                        m.build_drawer()
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
                        accept=".obj,.h5,.hdf5",
                        label="Load OBJ or HDF5…",
                        prepend_icon="mdi-folder-open",
                        density="comfortable",
                        hide_details=True,
                        classes="px-3 pt-2",
                    )
            v3.VSelect(
                v_model=("interaction_mode", "none"),
                items=(
                    "interaction_modes",
                    [{"title": m.title, "value": m.key} for m in modes],
                ),
                label="Mode",
                density="compact",
                variant="outlined",
                hide_details=True,
                classes="ml-4",
                style="max-width: 180px",
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
            # Toggle the surface colouring: off = height (viridis), on = region id.
            v3.VSwitch(
                v_model=("color_by_region", False),
                label="Regions",
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
        # the target. Hover is gated client-side on Pathfinding (the only mode using it) -- no
        # websocket traffic per mousemove outside it. Right-clicks are forwarded (and the native
        # context menu suppressed) whenever any mode is active; the server dispatches them to the
        # active mode's handler. In mode None the native context menu stays available.
        coords = (
            "[$event.offsetX / $event.target.clientWidth, "
            "1 - $event.offsetY / $event.target.clientHeight]"
        )
        hover_js = (
            "interaction_mode === 'pathfinding' && "
            f"$event.buttons === 0 && trigger('route_hover', {coords})"
        )
        click_js = (
            "interaction_mode !== 'none' && "
            f"($event.preventDefault(), trigger('mode_click', {coords}))"
        )
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

    # No initial file: start with an empty scene and let the user pick one via the
    # toolbar's "File -> Load OBJ or HDF5…" dialog.
    if obj_path:
        load_mesh(obj_path, os.path.basename(obj_path))
    elif h5_path:
        load_hdf5(h5_path, os.path.basename(h5_path))
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


def run_native(obj_path: str | None, h5_path: str | None, port: int) -> None:
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
            "-m",
            "trame_viewer",
            *(["--obj", obj_path] if obj_path else []),
            *(["--h5", h5_path] if h5_path else []),
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
    sources = parser.add_mutually_exclusive_group()
    sources.add_argument(
        "--obj",
        default=None,
        help="OBJ file to load on startup",
    )
    sources.add_argument(
        "--h5",
        default=None,
        help="HDF5 file written by jupedsim's Hdf5TrajectoryWriter to load on startup",
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
        run_native(args.obj, args.h5, port)
        return

    server = build_app(args.obj, args.h5)
    if not args.no_browser:
        print(f"Serving on http://localhost:{port}  (Ctrl-C to stop)")
    server.start(
        port=port, open_browser=not args.no_browser, show_connection_info=False
    )


if __name__ == "__main__":
    main()
