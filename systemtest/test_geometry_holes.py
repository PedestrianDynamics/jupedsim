# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import shapely


def test_geometry_holes_returns_coordinates():
    outer = [(0, 0), (100, 0), (100, 100), (0, 100)]
    hole = [(40, 40), (60, 40), (60, 60), (40, 60)]
    poly = shapely.Polygon(outer, holes=[hole])

    surface = jps.WalkableSurface()
    room = surface.add_region(polygon=poly)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=surface
    )
    geo = simulation.get_geometry()

    polygon = shapely.from_wkt(geo.polygon(region_id=room).as_wkt())
    assert len(polygon.interiors) == 1
    assert len(polygon.interiors[0].coords) >= 4
