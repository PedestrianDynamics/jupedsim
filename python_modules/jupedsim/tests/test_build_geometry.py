# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import numpy as np
import pytest
import shapely

from jupedsim.util import GeometryError, _polygons_from_geometry_collection


@pytest.mark.parametrize(
    "geometry_input",
    [
        shapely.Polygon(
            [(0, 3.1), (1.2, 5.4), (4.1, 7.9), (7.1, 3.0), (0, 3.1)]
        ),
        shapely.LinearRing(
            [(0, 3.1), (1.2, 5.4), (4.1, 7.9), (7.1, 3.0), (0, 3.1)]
        ),
        shapely.Polygon(
            [(-5.1, -3.2), (-3.1, 2.4), (-1.1, -2.5), (-5.1, -3.2)],
            [[(-4.1, -2.3), (-2.1, -2.3), (-2.1, -1.1), (-4.1, -1.1)]],
        ),
        shapely.Polygon(
            [
                (2.4, -1.3),
                (4.1, -5),
                (3, -7.2),
                (-1.3, -10.1),
                (-2, -3.1),
                (2.4, -1.3),
            ],
            [
                [(3.1, -3.5), (1.7, -2.3), (0.9, -4.1)],
                [(-1.2, -3.9), (-0.8, -6.1), (0.1, -5.9), (0.2, -4.3)],
                [(0.7, -7.5), (2.1, -6.5), (2.5, -4.7)],
            ],
        ),
        shapely.MultiPolygon(
            [
                shapely.Polygon(
                    [(0, 3.1), (1.2, 5.4), (4.1, 7.9), (7.1, 3.0), (0, 3.1)]
                ),
                shapely.Polygon(
                    [(-5.1, -3.2), (-3.1, 2.4), (-1.1, -2.5), (-5.1, -3.2)],
                    [[(-4.1, -2.3), (-2.1, -2.3), (-2.1, -1.1), (-4.1, -1.1)]],
                ),
                shapely.Polygon(
                    [
                        (2.4, -1.3),
                        (4.1, -5),
                        (3, -7.2),
                        (-1.3, -10.1),
                        (-2, -3.1),
                        (2.4, -1.3),
                    ],
                    [
                        [(3.1, -3.5), (1.7, -2.3), (0.9, -4.1)],
                        [(-1.2, -3.9), (-0.8, -6.1), (0.1, -5.9), (0.2, -4.3)],
                        [(0.7, -7.5), (2.1, -6.5), (2.5, -4.7)],
                    ],
                ),
            ]
        ),
        shapely.GeometryCollection(
            [
                shapely.Polygon(
                    [(0, 3.1), (1.2, 5.4), (4.1, 7.9), (7.1, 3.0), (0, 3.1)]
                ),
                shapely.Polygon(
                    [(-5.1, -3.2), (-3.1, 2.4), (-1.1, -2.5), (-5.1, -3.2)],
                    [[(-4.1, -2.3), (-2.1, -2.3), (-2.1, -1.1), (-4.1, -1.1)]],
                ),
                shapely.Polygon(
                    [
                        (2.4, -1.3),
                        (4.1, -5),
                        (3, -7.2),
                        (-1.3, -10.1),
                        (-2, -3.1),
                        (2.4, -1.3),
                    ],
                    [
                        [(3.1, -3.5), (1.7, -2.3), (0.9, -4.1)],
                        [(-1.2, -3.9), (-0.8, -6.1), (0.1, -5.9), (0.2, -4.3)],
                        [(0.7, -7.5), (2.1, -6.5), (2.5, -4.7)],
                    ],
                ),
            ]
        ),
    ],
)
def test_polygons_from_geometry_collection(geometry_input):
    if (
        shapely.get_type_id(geometry_input)
        == shapely.GeometryType.GEOMETRYCOLLECTION
    ) or (
        shapely.get_type_id(geometry_input)
        == shapely.GeometryType.MULTIPOLYGON
    ):
        reference = [geo for geo in geometry_input.geoms]
    else:
        reference = [geometry_input]
        geometry_input = shapely.GeometryCollection([geometry_input])

    result = _polygons_from_geometry_collection(geometry_input)

    assert len(result) == len(result)
    for poly in result:
        assert shapely.get_type_id(poly) == shapely.GeometryType.POLYGON

    assert shapely.equals_exact(
        shapely.GeometryCollection(result),
        shapely.GeometryCollection(result),
    )


@pytest.mark.parametrize(
    "geometry_input, error_msg",
    [
        (
            shapely.GeometryCollection(
                [
                    shapely.Polygon(
                        [
                            (0, 3.1),
                            (1.2, 5.4),
                            (4.1, 7.9),
                            (7.1, 3.0),
                            (0, 3.1),
                        ]
                    ),
                    shapely.Point(
                        (-5.1, -3.2),
                    ),
                    shapely.Polygon(
                        [
                            (2.4, -1.3),
                            (4.1, -5),
                            (3, -7.2),
                            (-1.3, -10.1),
                            (-2, -3.1),
                            (2.4, -1.3),
                        ],
                        [
                            [(3.1, -3.5), (1.7, -2.3), (0.9, -4.1)],
                            [
                                (-1.2, -3.9),
                                (-0.8, -6.1),
                                (0.1, -5.9),
                                (0.2, -4.3),
                            ],
                            [(0.7, -7.5), (2.1, -6.5), (2.5, -4.7)],
                        ],
                    ),
                ]
            ),
            "Unexpected geometry type found in GeometryCollection:",
        ),
    ],
)
def test_polygons_from_geometry_collection_error(geometry_input, error_msg):
    with pytest.raises(GeometryError, match=f".*{error_msg}.*"):
        _polygons_from_geometry_collection(geometry_input)
