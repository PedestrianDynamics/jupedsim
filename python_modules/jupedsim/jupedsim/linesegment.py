# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps


class LineSegment:
    """Represents a line segment in 2D space, defined by two endpoints p1 and p2."""

    def __init__(self, obj: py_jps.LineSegment):
        """Initialize the LineSegment with a native LineSegment object.

        Args:
            obj (py_jps.LineSegment): The native LineSegment object to wrap.
        """
        self._obj = obj

    @property
    def p1(self) -> tuple[float, float]:
        """Get the first endpoint of the line segment."""
        return self._obj.p1

    @property
    def p2(self) -> tuple[float, float]:
        """Get the second endpoint of the line segment."""
        return self._obj.p2

    def distance_to_point(self, point: tuple[float, float]) -> float:
        """Calculate the distance from a given point to this line segment.

        Args:
            point (tuple[float, float]): The point to calculate the distance to.

        Returns:
            float: The distance from the point to the line segment.
        """
        return self._obj.dist_to(point)

    def closest_point(self, point: tuple[float, float]) -> tuple[float, float]:
        """Calculate the closest point on the line segment to a given point.

        Args:
            point (tuple[float, float]): The point to find the closest point to.

        Returns:
            tuple[float, float]: The closest point on the line segment to the given point.
        """
        return self._obj.shortest_point(point)
