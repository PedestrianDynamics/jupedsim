# SPDX-License-Identifier: LGPL-3.0-or-later

import heapq
import itertools

import numpy as np

import jupedsim.native as py_jps

# Alias the routing engines.
RoutingEngine = py_jps.RoutingEngine
TAStarRoutingEngine = py_jps.TAStarRoutingEngine


def _tri_area2(a: np.ndarray, b: np.ndarray, c: np.ndarray) -> float:
    """Signed area*2 of triangle (a, b, c) in Mononen's funnel convention.

    > 0 when c is on the clockwise side of a->b. Matches the sign comparisons
    used by the Simple Stupid Funnel Algorithm below.
    """
    return (c[0] - a[0]) * (b[1] - a[1]) - (b[0] - a[0]) * (c[1] - a[1])


def _points_equal(a: np.ndarray, b: np.ndarray) -> bool:
    return bool(np.allclose(a, b, atol=1e-9))


def _point_segment_distance(
    p: np.ndarray, a: np.ndarray, b: np.ndarray
) -> float:
    """Euclidean distance from point *p* to segment ``[a, b]``."""
    ab = b - a
    denom = float(np.dot(ab, ab))
    if denom <= 1e-18:
        return float(np.linalg.norm(p - a))
    t = float(np.dot(p - a, ab)) / denom
    t = min(1.0, max(0.0, t))
    return float(np.linalg.norm(p - (a + t * ab)))


class _Funnel:
    """Incremental two-sided funnel from a fixed start point.

    Maintains the taut path from the start to the current "mouth" edge as an
    apex (with the committed distance ``apex_g`` already walked to reach it)
    plus a left and right chain of vertices beyond the apex. ``push_left`` /
    ``push_right`` extend the funnel by one portal vertex, advancing the apex
    (and committing distance) whenever the funnel collapses. Used by TA* only
    to score search nodes; the emitted geometry comes from :meth:`_funnel`.
    """

    __slots__ = ("apex", "apex_g", "left", "right")

    def __init__(self, apex: np.ndarray, apex_g: float = 0.0) -> None:
        self.apex = apex
        self.apex_g = apex_g
        self.left: list[np.ndarray] = []  # beyond apex, outermost last
        self.right: list[np.ndarray] = []

    def copy(self) -> "_Funnel":
        f = _Funnel(self.apex, self.apex_g)
        f.left = self.left.copy()
        f.right = self.right.copy()
        return f

    def mouth_left(self) -> np.ndarray:
        return self.left[-1] if self.left else self.apex

    def mouth_right(self) -> np.ndarray:
        return self.right[-1] if self.right else self.apex

    def g_to_mouth(self) -> float:
        """Shortest distance from start to reach the current mouth edge.

        The apex sees the whole mouth, so this is the committed apex distance
        plus the point-to-segment distance from the apex to the mouth edge.
        """
        return self.apex_g + _point_segment_distance(
            self.apex, self.mouth_left(), self.mouth_right()
        )

    def push_right(self, p: np.ndarray) -> None:
        # Tighten the right chain: keep only vertices where the chain still
        # turns clockwise (taut) towards p.
        while self.right:
            last = self.right[-1]
            prev = self.right[-2] if len(self.right) >= 2 else self.apex
            if _tri_area2(prev, last, p) > 0.0:
                break
            self.right.pop()
        else:
            # Right chain emptied: if p crossed over the left boundary, the
            # funnel collapses and the apex advances along the left chain.
            while self.left:
                l0 = self.left[0]
                if _tri_area2(self.apex, l0, p) < 0.0:
                    self.apex_g += float(np.linalg.norm(l0 - self.apex))
                    self.apex = l0
                    self.left.pop(0)
                else:
                    break
        self.right.append(p)

    def push_left(self, p: np.ndarray) -> None:
        while self.left:
            last = self.left[-1]
            prev = self.left[-2] if len(self.left) >= 2 else self.apex
            if _tri_area2(prev, last, p) < 0.0:
                break
            self.left.pop()
        else:
            while self.right:
                r0 = self.right[0]
                if _tri_area2(self.apex, r0, p) > 0.0:
                    self.apex_g += float(np.linalg.norm(r0 - self.apex))
                    self.apex = r0
                    self.right.pop(0)
                else:
                    break
        self.left.append(p)


class DirectPathRoutingEngine(RoutingEngine):
    """Trivial routing engine that returns a straight line ignoring geometry.

    Useful as a reference implementation and for open-space scenarios where
    agents should walk directly towards their destination.
    """

    def name(self) -> str:
        return "DirectPath"

    def set_geometry(self, _) -> None:
        pass

    def compute_waypoints(
        self,
        frm: tuple[float, float],
        to: tuple[float, float],
    ) -> list[tuple[float, float]]:
        return [frm, to]

    def is_routable(self, _: tuple[float, float]) -> bool:
        return True


class PythonTAStarRoutingEngine(RoutingEngine):
    """Pure-Python TA* (Triangulation A*) any-angle routing engine.

    A faithful funnel-based TA* (Demyen & Buro) over the triangulated
    nav-mesh: A* on the triangle adjacency graph where each search node
    carries the incremental funnel from the start, ``g`` is the exact taut
    distance to reach the node's mouth edge and ``h`` is the straight-line
    lower bound to the goal. The optimal triangle channel is then string-
    pulled into the taut path. Runs on the same mesh as
    :class:`TAStarRoutingEngine` (used here only to triangulate the geometry).
    """

    def __init__(self) -> None:
        super().__init__()
        self._native_tastar = TAStarRoutingEngine()  # only used to get the mesh
        self._vertices: np.ndarray | None = None
        self._polygons: list[list[int]] = []
        # neighbors[t][i] = triangle across edge (poly[i], poly[i+1]) or -1.
        self._neighbors: list[list[int]] = []

    def name(self) -> str:
        return "PythonTAStar"

    @classmethod
    def from_mesh(cls, verts, polys) -> "PythonTAStarRoutingEngine":
        engine = cls()
        engine._set_mesh(verts, polys)
        return engine

    def set_geometry(self, geometry) -> None:
        self._native_tastar.set_geometry(geometry)
        self._set_mesh(*self._native_tastar.mesh())

    def _set_mesh(self, verts, polys) -> None:
        self._vertices = np.asarray(verts, dtype=float)
        self._polygons = [list(p) for p in polys]
        self._build_adjacency()

    def mesh(self) -> tuple[list[tuple[float, float]], list[list[int]]]:
        if self._vertices is None:
            raise RuntimeError("set_geometry must be called first")
        verts = [(float(x), float(y)) for x, y in self._vertices]
        return verts, [list(p) for p in self._polygons]

    def _build_adjacency(self) -> None:
        # Map each undirected edge to the (triangle, edge-index) pairs using it.
        edge_to: dict[tuple[int, int], list[tuple[int, int]]] = {}
        for t, poly in enumerate(self._polygons):
            n = len(poly)
            for i in range(n):
                a, b = poly[i], poly[(i + 1) % n]
                key = (a, b) if a < b else (b, a)
                edge_to.setdefault(key, []).append((t, i))

        self._neighbors = [[-1] * len(poly) for poly in self._polygons]
        for users in edge_to.values():
            if len(users) != 2:
                continue
            (t0, i0), (t1, i1) = users
            self._neighbors[t0][i0] = t1
            self._neighbors[t1][i1] = t0

    def _locate(self, point: tuple[float, float]) -> int | None:
        """Index of the polygon containing *point*, or ``None``."""
        p = np.asarray(point, dtype=float)
        verts = self._vertices
        eps = 1e-9
        for poly_idx, poly in enumerate(self._polygons):
            ring = verts[poly]
            # CCW convex polygon: inside iff point is left-of every edge.
            edges = np.roll(ring, -1, axis=0) - ring
            rel = p - ring
            cross = edges[:, 0] * rel[:, 1] - edges[:, 1] * rel[:, 0]
            if np.all(cross >= -eps):
                return poly_idx
        return None

    def _orient(
        self, from_poly: int, a: int, b: int
    ) -> tuple[np.ndarray, np.ndarray]:
        """Left/right endpoints of edge ``{a, b}`` for travel leaving *from_poly*.

        The shared edge appears in *from_poly*'s CCW winding as ``(u, v)``;
        crossing it outward puts the edge head ``v`` on the left of travel and
        the tail ``u`` on the right.
        """
        poly = self._polygons[from_poly]
        n = len(poly)
        for i in range(n):
            u, v = poly[i], poly[(i + 1) % n]
            if (u == a and v == b) or (u == b and v == a):
                return self._vertices[v], self._vertices[u]
        raise AssertionError("edge not found in polygon")

    def _shared_edge(self, a: int, b: int) -> tuple[int, int]:
        shared = tuple(set(self._polygons[a]) & set(self._polygons[b]))
        return shared  # type: ignore[return-value]

    def _channel_portals(
        self, channel: list[int]
    ) -> tuple[list[np.ndarray], list[np.ndarray]]:
        lefts: list[np.ndarray] = []
        rights: list[np.ndarray] = []
        for a, b in zip(channel, channel[1:]):
            u, v = self._shared_edge(a, b)
            left, right = self._orient(a, u, v)
            lefts.append(left)
            rights.append(right)
        return lefts, rights

    @staticmethod
    def _funnel(
        frm: tuple[float, float],
        to: tuple[float, float],
        lefts: list[np.ndarray],
        rights: list[np.ndarray],
    ) -> list[tuple[float, float]]:
        """String-pull (Simple Stupid Funnel Algorithm) across the portals."""
        start = np.asarray(frm, dtype=float)
        goal = np.asarray(to, dtype=float)
        # Pad with degenerate start/goal portals so apex moves are uniform.
        left = [start, *lefts, goal]
        right = [start, *rights, goal]

        pts: list[np.ndarray] = [start]
        apex, p_left, p_right = start, start, start
        apex_i = left_i = right_i = 0

        i = 1
        while i < len(left):
            lv, rv = left[i], right[i]
            # Tighten the right side.
            if _tri_area2(apex, p_right, rv) <= 0.0:
                if _points_equal(apex, p_right) or _tri_area2(apex, p_left, rv) > 0.0:
                    p_right, right_i = rv, i
                else:
                    # Right crossed left: left vertex becomes a corner.
                    pts.append(p_left)
                    apex = p_left
                    apex_i = left_i
                    p_left = p_right = apex
                    left_i = right_i = apex_i
                    i = apex_i + 1
                    continue
            # Tighten the left side.
            if _tri_area2(apex, p_left, lv) >= 0.0:
                if _points_equal(apex, p_left) or _tri_area2(apex, p_right, lv) < 0.0:
                    p_left, left_i = lv, i
                else:
                    pts.append(p_right)
                    apex = p_right
                    apex_i = right_i
                    p_left = p_right = apex
                    left_i = right_i = apex_i
                    i = apex_i + 1
                    continue
            i += 1
        pts.append(goal)

        # Drop consecutive duplicates (degenerate portals can repeat a corner).
        out: list[tuple[float, float]] = []
        for p in pts:
            t = (float(p[0]), float(p[1]))
            if not out or out[-1] != t:
                out.append(t)
        return out

    def compute_waypoints(
        self,
        frm: tuple[float, float],
        to: tuple[float, float],
    ) -> list[tuple[float, float]]:
        if self._vertices is None:
            raise RuntimeError("set_geometry must be called first")

        start_tri = self._locate(frm)
        goal_tri = self._locate(to)
        if start_tri is None or goal_tri is None:
            raise RuntimeError(
                f"endpoint outside mesh (start={start_tri}, goal={goal_tri})"
            )
        if start_tri == goal_tri:
            return [frm, to]

        channel = self._search(frm, to, start_tri, goal_tri)
        if channel is None:
            raise RuntimeError("no path between endpoints")
        return self._funnel(frm, to, *self._channel_portals(channel))

    def _search(
        self,
        frm: tuple[float, float],
        to: tuple[float, float],
        start_tri: int,
        goal_tri: int,
    ) -> list[int] | None:
        """Plain TA* (Demyen & Buro): triangle channel start_tri -> goal_tri.

        A* over the triangle adjacency graph where each node carries the
        incremental funnel from the start, ``g`` is the funnel-exact distance
        to the node's mouth edge and ``h`` is the straight-line lower bound to
        the goal. Nodes are keyed by the directed edge ``(prev_tri, tri)`` and
        deduped by best ``g`` (the textbook TA* prune): fast and optimal in the
        vast majority of cases, with the rare suboptimality that motivated
        polyanya."""
        frm_p = np.asarray(frm, dtype=float)
        goal_p = np.asarray(to, dtype=float)

        # Open heap entries: (f, kind, counter, payload).
        #   interior: kind=0, payload=(key=(prev_tri, tri), g, funnel)
        #   terminal: kind=1, payload=(channel,)
        counter = itertools.count()
        heap: list = []
        parent: dict[tuple[int, int], tuple[int, int] | None] = {}
        best_g: dict[tuple[int, int], float] = {}

        def channel_of(key: tuple[int, int]) -> list[int]:
            seq: list[int] = []
            cur: tuple[int, int] | None = key
            while cur is not None:
                seq.append(cur[1])
                nxt = parent.get(cur)
                if nxt is None:
                    seq.append(cur[0])
                    break
                cur = nxt
            seq.reverse()
            return seq

        def expand(prev_tri: int, tri: int, funnel: _Funnel) -> None:
            poly = self._polygons[tri]
            mouth = set(self._shared_edge(prev_tri, tri))
            n = len(poly)
            for i in range(n):
                a, b = poly[i], poly[(i + 1) % n]
                if {a, b} == mouth:
                    continue
                nb = self._neighbors[tri][i]
                if nb < 0:
                    continue
                if nb == goal_tri:
                    heapq.heappush(
                        heap,
                        (0.0, 1, next(counter),
                         (channel_of((prev_tri, tri)) + [nb],)),
                    )
                    continue
                _, new_right = self._orient(tri, a, b)
                o_idx = a if a not in mouth else b
                o_pt = self._vertices[o_idx]
                child = funnel.copy()
                if _points_equal(new_right, o_pt):
                    child.push_right(o_pt)
                else:
                    child.push_left(o_pt)
                g = child.g_to_mouth()
                key = (tri, nb)
                if g >= best_g.get(key, float("inf")) - 1e-12:
                    continue
                best_g[key] = g
                parent[key] = (prev_tri, tri)
                h = _point_segment_distance(
                    goal_p, child.mouth_left(), child.mouth_right()
                )
                heapq.heappush(
                    heap, (g + h, 0, next(counter), (key, g, child))
                )

        # Seed from the start triangle.
        poly = self._polygons[start_tri]
        n = len(poly)
        for i in range(n):
            nb = self._neighbors[start_tri][i]
            if nb < 0:
                continue
            a, b = poly[i], poly[(i + 1) % n]
            if nb == goal_tri:
                heapq.heappush(
                    heap, (0.0, 1, next(counter), ([start_tri, nb],))
                )
                continue
            new_left, new_right = self._orient(start_tri, a, b)
            funnel = _Funnel(frm_p, 0.0)
            funnel.left = [new_left]
            funnel.right = [new_right]
            key = (start_tri, nb)
            best_g[key] = funnel.g_to_mouth()
            parent[key] = None
            h = _point_segment_distance(goal_p, new_left, new_right)
            heapq.heappush(
                heap,
                (best_g[key] + h, 0, next(counter), (key, best_g[key], funnel)),
            )

        while heap:
            _, kind, _, payload = heapq.heappop(heap)
            if kind == 1:
                return payload[0]  # goal channel
            key, g, funnel = payload
            if g > best_g.get(key, float("inf")) + 1e-12:
                continue  # stale: superseded by a cheaper funnel to this edge
            expand(key[0], key[1], funnel)

        return None

    def is_routable(self, point: tuple[float, float]) -> bool:
        if self._vertices is None:
            raise RuntimeError("set_geometry must be called first")
        return self._locate(point) is not None
