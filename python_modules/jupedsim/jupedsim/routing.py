# SPDX-License-Identifier: LGPL-3.0-or-later

import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import dijkstra

import jupedsim.native as py_jps

# Alias the routing engines.
RoutingEngine = py_jps.RoutingEngine
TAStarRoutingEngine = py_jps.TAStarRoutingEngine


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
    """Pure-Python TA* (Triangulation A*) routing engine based on
    scipy.sparse.csgraph.dijkstra. Use TAStarRoutingEngine for getting
    the same mesh.
    """

    def __init__(self) -> None:
        super().__init__()
        self._native_tastar = TAStarRoutingEngine()  # only used to get the mesh
        self._vertices: np.ndarray | None = None
        self._polygons: list[list[int]] = []
        self._centroids: np.ndarray | None = None
        self._graph: csr_matrix | None = None

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
        self._build_dual_graph()

    def mesh(self) -> tuple[list[tuple[float, float]], list[list[int]]]:
        if self._vertices is None:
            raise RuntimeError("set_geometry must be called first")
        verts = [(float(x), float(y)) for x, y in self._vertices]
        return verts, [list(p) for p in self._polygons]

    def _build_dual_graph(self) -> None:
        verts = self._vertices
        # Centroid per polygon, for both point-location fallbacks and edge cost.
        self._centroids = np.array(
            [verts[p].mean(axis=0) for p in self._polygons]
        )

        # Map each undirected edge (sorted vertex-index pair) to the polygons
        # that use it. An edge shared by exactly two polygons is a portal.
        edge_to_polys: dict[tuple[int, int], list[int]] = {}
        for poly_idx, poly in enumerate(self._polygons):
            n = len(poly)
            for i in range(n):
                a, b = poly[i], poly[(i + 1) % n]
                key = (a, b) if a < b else (b, a)
                edge_to_polys.setdefault(key, []).append(poly_idx)

        rows: list[int] = []
        cols: list[int] = []
        data: list[float] = []
        for polys in edge_to_polys.values():
            if len(polys) != 2:
                continue
            p, q = polys
            cost = float(
                np.linalg.norm(self._centroids[p] - self._centroids[q])
            )
            rows += [p, q]
            cols += [q, p]
            data += [cost, cost]

        n = len(self._polygons)
        self._graph = csr_matrix((data, (rows, cols)), shape=(n, n))

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

    def _portal_midpoint(self, a: int, b: int) -> tuple[float, float]:
        shared = sorted(set(self._polygons[a]) & set(self._polygons[b]))
        mid = self._vertices[shared].mean(axis=0)
        return (float(mid[0]), float(mid[1]))

    def compute_waypoints(
        self,
        frm: tuple[float, float],
        to: tuple[float, float],
    ) -> list[tuple[float, float]]:
        if self._graph is None:
            raise RuntimeError("set_geometry must be called first")

        start = self._locate(frm)
        goal = self._locate(to)
        if start is None or goal is None:
            raise RuntimeError(
                f"endpoint outside mesh (start={start}, goal={goal})"
            )
        if start == goal:
            return [frm, to]

        _, predecessors = dijkstra(
            self._graph,
            directed=False,
            indices=start,
            return_predecessors=True,
        )
        if predecessors[goal] < 0:
            raise RuntimeError("no path between endpoints")

        # Reconstruct the polygon channel start -> goal.
        channel: list[int] = []
        node = goal
        while node != start:
            channel.append(node)
            node = predecessors[node]
        channel.append(start)
        channel.reverse()

        waypoints: list[tuple[float, float]] = [frm]
        for a, b in zip(channel, channel[1:]):
            waypoints.append(self._portal_midpoint(a, b))
        waypoints.append(to)
        return waypoints

    def is_routable(self, point: tuple[float, float]) -> bool:
        if self._vertices is None:
            raise RuntimeError("set_geometry must be called first")
        return self._locate(point) is not None
