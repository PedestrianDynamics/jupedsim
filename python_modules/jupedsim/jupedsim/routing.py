# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Any

import shapely

import jupedsim.native as py_jps
from jupedsim.geometry_utils import build_geometry


class RoutingEngine:
    def __init__(
        self,
        geometry: str
        | shapely.GeometryCollection
        | shapely.Polygon
        | shapely.MultiPolygon
        | shapely.MultiPoint
        | list[tuple[float, float]],
        **kwargs: Any,
    ) -> None:
        self._obj = py_jps.RoutingEngine(
            build_geometry(geometry, *kwargs)._obj
        )

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
