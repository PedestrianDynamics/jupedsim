# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps


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
