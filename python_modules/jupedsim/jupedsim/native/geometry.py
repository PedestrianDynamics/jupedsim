# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps


class Geometry:
    """Geometry object for simulations."""

    def __init__(self, obj):
        self._obj = obj


class GeometryBuilder:
    def __init__(self):
        self._obj = py_jps.GeometryBuilder()

    def add_accessible_area(self, polygon: list[tuple[float, float]]) -> None:
        """Adds an area which can be accessed by the agents to the geometry.

        Args:
            polygon (list[tuple[float, float]]): list of x,y coordinates of
                the points of a polygon
        """
        self._obj.add_accessible_area(polygon)

    def exclude_from_accessible_area(
        self, polygon: list[tuple[float, float]]
    ) -> None:
        """Marks an area as un-accessible by the agents to the geometry.

        Args:
            polygon (list[tuple[float, float]]): list of x,y coordinates of
                the points of a polygon
        """

        self._obj.exclude_from_accessible_area(polygon)

    def build(self) -> Geometry:
        """Builds a Geometry from the given accessible and un-accessible areas.

        Returns:
            Geometry object
        """
        return Geometry(self._obj.build())
