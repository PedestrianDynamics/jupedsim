# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps


class Geometry:
    """Geometry object for simulations."""

    def __init__(self, obj: py_jps.Geometry):
        self._obj = obj

    def boundary(self) -> list[tuple[float, float]]:
        return self._obj.boundary()

    def holes(self) -> list[list[tuple[float, float]]]:
        return self._obj.holes()
