# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim as jps
import pytest
from shapely import Polygon, LinearRing


def test_new_geometry_definition_v1():

    ground_floor = {
        "exterior": [(0, 0), (10, 0), (10, 10), (0, 10)],
        "interior": [[(2, 2), (6, 2), (6, 2.2), (6, 3.8), (6, 4), (2, 4)]],
        "height": 0.0,
    }

    first_floor = {
        "exterior": [(0, 0), (10, 0), (10, 10), (0, 10)],
        "interior": [[(2, 4), (2, 3.8), (2, 2.2), (2, 2), (6, 4), (2, 4)]],
        "height": 3.0,
    }

    walkable_surface = jps.WalkableSurface()

    id_0 = walkable_surface.add_region(**ground_floor)
    id_1 = walkable_surface.add_region(**first_floor)
    id_0_to_1 = walkable_surface.connect_regions(
        from_region=id_0,
        from_edge=((6, 2.2), (6, 3.8)),
        to_region=id_1,
        to_edge=((2, 2.2), (2, 3.8)),
    )

    # layer_0 = geo.add_layer(polygon=p1, height=0)
    # layer_1 = geo.add_layer(polygon=p2, height=3.6)
    # layer 0, edge 2 connects to layer_1, 4
    # connect(layer_0, 2, layer_1, 4),
