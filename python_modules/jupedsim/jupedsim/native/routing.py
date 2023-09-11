# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps

from jupedsim.native.geometry import Geometry


class RoutingEngine:
    def __init__(self, geo: Geometry) -> None:
        self._obj = py_jps.RoutingEngine(geo._obj)

    def compute_waypoints(
        self, frm: tuple[float, float], to: tuple[float, float]
    ) -> list[tuple[float, float]]:
        return self._obj.compute_waypoints(frm, to)

    def is_routable(self, p: tuple[float, float]) -> bool:
        return self._obj.is_routable(p)

    def mesh(
        self,
    ) -> list[
        tuple[tuple[float, float], tuple[float, float], tuple[float, float]]
    ]:
        return self._obj.mesh()

    def edges_for(self, vertex_id: int):
        return self._obj.edges_for(vertex_id)
