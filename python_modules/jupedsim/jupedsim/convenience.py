# SPDX-License-Identifier: LGPL-3.0-or-later

import math

import shapely


def circle_to_polygon(
    *, center: tuple[float, float], radius: float
) -> shapely.Polygon:
    """Polygon approximating a circle, with its corners on the circle.

    The edges are at most 0.4 long, and there are at least four corners.

    Args:
        center: Center of the circle.
        radius: Radius of the circle. Must be positive.

    Returns:
        The polygon.

    Raises:
        ValueError: If ``radius`` is not positive.
    """
    if not radius > 0:
        raise ValueError(f"Circle radius must be positive, got {radius}")
    max_edge_length = 0.4
    # shapely puts quad_segs edges on each quarter, each 2 r sin(pi / (4 quad_segs)) long.
    max_sin = min(max_edge_length / (2 * radius), 1.0)
    quad_segs = math.ceil(math.pi / (4 * math.asin(max_sin)))
    return shapely.Point(center).buffer(radius, quad_segs=quad_segs)
