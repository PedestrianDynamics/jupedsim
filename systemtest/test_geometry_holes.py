# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import shapely


def test_geometry_holes_returns_coordinates():
    outer = [(0, 0), (100, 0), (100, 100), (0, 100)]
    hole = [(40, 40), (60, 40), (60, 60), (40, 60)]
    poly = shapely.Polygon(outer, holes=[hole])

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=poly,
    )
    geo = simulation.get_geometry()

    holes = geo.holes()
    assert holes is not None
    assert len(holes) == 1
    assert len(holes[0]) >= 4
