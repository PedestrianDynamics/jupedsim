# Route API draft, example 2: two floors.
# Sketch against examples/route_api_draft.py; the API does not exist yet.
#
# Two floors over the same 20 m x 10 m footprint, the upper one at 3 m,
# both with a stairwell 12..18 m x 6..8 m. A staircase in the stairwell
# rises 3 m over 6 m (about 27 degrees, within master's 50 degree limit)
# from the stairwell's west edge on the ground floor to its east edge on
# the upper floor. Each floor lies on the side of its edge away from the
# staircase, so the surfaces join without folding back. Agents start
# upstairs and leave through an exit on the ground floor's left wall.

import jupedsim as jps
import numpy as np
import shapely

# The connector edges must be whole boundary edges: here, the stairwell's
# west and east edges.
footprint = [(0, 0), (20, 0), (20, 10), (0, 10)]
stairwell = [(12, 6), (18, 6), (18, 8), (12, 8)]
ground_polygon = shapely.Polygon(footprint, holes=[stairwell])
upper_polygon = shapely.Polygon(footprint, holes=[stairwell])

surface = jps.WalkableSurface()
floor0 = surface.add_region(polygon=ground_polygon, height=0.0, name="floor0")
floor1 = surface.add_region(polygon=upper_polygon, height=3.0, name="floor1")
staircase = surface.connect_regions(
    from_region=floor0,
    from_edge=((12, 6), (12, 8)),
    to_region=floor1,
    to_edge=((18, 6), (18, 8)),
    name="staircase",
)

# (10, 5) lies on both floors: the region has to be supplied explicitly.
print(surface.regions_at((10, 5)))  # (floor0, floor1)

sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)

exit_ground = sim.add_exit(
    [(0, 4), (0.5, 4), (0.5, 6), (0, 6)], region=floor0, name="exit_ground"
)
evacuate = sim.add_route([exit_ground])

# The staircase is not a stage: the exit's floor field routes over it.
upstairs = [
    sim.add_agent(floor1.at(x, 5.0), route=evacuate)
    for x in (10.0, 12.0, 14.0, 16.0)
]
print(upstairs[0].stage)  # Exit(id=0, name="exit_ground")

# Travel times to the exit, as seen on the upper floor.
grid = sim.travel_times(exit_ground)[floor1]
# Cells outside the walkable area are inf; skip them.
finite = grid.values[np.isfinite(grid.values)]
print(f"longest travel time upstairs: {finite.max():.1f} s")

while sim.agent_count() > 0 and sim.elapsed_time() < 300:
    sim.iterate()
    for agent in upstairs:
        if agent.alive and agent.location.region == staircase:
            print(f"t={sim.elapsed_time():.1f} s: {agent} on the staircase")
