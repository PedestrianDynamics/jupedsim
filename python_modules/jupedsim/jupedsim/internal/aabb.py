# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
class AABB:
    def __init__(
        self, *, xmin: float, xmax: float, ymin: float, ymax: float
    ) -> None:
        if xmax < xmin or ymax < ymin:
            raise Exception(
                f"Invalid arguments to create AABB: max values have to be larger than min values."
            )
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax

    @property
    def width(self) -> float:
        return abs(self.xmax - self.xmin)

    @property
    def height(self) -> float:
        return abs(self.ymax - self.ymin)

    @property
    def center(self) -> tuple[float, float]:
        return (self.xmin + self.width / 2, self.ymin + self.height / 2)

    @staticmethod
    def combine(aabb, *other) -> "AABB":
        xmin = aabb.xmin
        ymin = aabb.ymin
        xmax = aabb.xmax
        ymax = aabb.ymax

        for aabb in other:
            xmin = min(xmin, aabb.xmin)
            ymin = min(ymin, aabb.ymin)
            xmax = max(xmax, aabb.xmax)
            ymax = max(ymax, aabb.ymax)

        return AABB(xmin=xmin, xmax=xmax, ymin=ymin, ymax=ymax)
