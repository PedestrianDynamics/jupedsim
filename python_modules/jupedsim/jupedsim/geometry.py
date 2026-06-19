# SPDX-License-Identifier: LGPL-3.0-or-later

import shapely

import jupedsim.native as py_jps
from jupedsim.linesegment import LineSegment


class Geometry:
    """Geometry object representing the area agents can move on.

    Gain access to the simulation's walkable area by calling:

    .. code :: python

        sim.get_geometry()
    """

    def __init__(self, obj: py_jps.Geometry):
        self._obj = obj

    def boundary(self) -> list[tuple[float, float]]:
        """Access the boundary polygon of the walkable area.

        Returns:
            List of 2d points describing the polygon.
        """
        return self._obj.boundary()

    def holes(self) -> list[list[tuple[float, float]]]:
        """Access holes (inner boundaries) of the walkable area.

        Returns:
            A list of polygons forming holes inside the boundary.
        """
        return self._obj.holes()

    def as_wkt(self) -> str:
        """_summary_

        Returns:
            String: _description_
        """
        poly = shapely.Polygon(self.boundary(), holes=self.holes())
        return shapely.to_wkt(
            poly,
            rounding_precision=-1,
        )

    def linesegments_close_to(
        self, point: tuple[float, float]
    ) -> list[LineSegment]:
        """Find line segments of the geometry that are within a certain distance to a point.

        Args:
            point (tuple[float, float]): The point to check against.
            distance (float): The maximum distance for line segments to be included.

        Returns:
            List of LineSegment objects that are close to the given point.
        """
        ls_list = []
        for ls in self._obj.linesegments_close_to(point):
            ls_list.append(LineSegment(ls))
        return ls_list

    def get_walls_close_to(
        self, point: tuple[float, float]
    ) -> list[LineSegment]:
        """Find line segments of the geometry that are within a certain distance to a point.

        Args:
            point (tuple[float, float]): The point to check against.
            distance (float): The maximum distance for line segments to be included.

        Returns:
            List of LineSegment objects that are close to the given point.
        """
        ls_list = []
        for ls in self._obj.linesegments_close_to(point):
            ls_list.append(LineSegment(ls))
        return ls_list

    def get_walls_in_distance_to(
        self, point: tuple[float, float], distance: float
    ) -> list[LineSegment]:
        """Find line segments of the geometry that are within a certain distance to a point.

        Args:
            point (tuple[float, float]): The point to check against.
            distance (float): The maximum distance for line segments to be included.

        Returns:
            List of LineSegment objects that are close to the given point.
        """
        ls_list = []
        for ls in self._obj.linesegments_in_distance_to(distance, point):
            ls_list.append(LineSegment(ls))
        return ls_list
