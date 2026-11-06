# Route API draft, example 2: two floors.
# Sketch against examples/route_api_draft.py; the API does not exist yet.
#
# Ground floor 0..20 m x 0..10 m, upper floor 6..26 m x 0..10 m at 3 m.
# The floors overlap in plan between x = 6 and x = 20. A staircase rises
# from the ground floor's right wall (x = 20) to the upper floor's right
# wall (x = 26), rising 3 m over 6 m (about 27 degrees, within master's
# 50 degree limit). Agents start upstairs and leave through an exit on the
# ground floor's left wall.

import jupedsim as jps
import shapely

# The connector edges must be whole boundary edges, hence the extra
# vertices at y = 2.
ground_polygon = shapely.Polygon([(0, 0), (20, 0), (20, 2), (20, 10), (0, 10)])
upper_polygon = shapely.Polygon([(6, 0), (26, 0), (26, 2), (26, 10), (6, 10)])

surface = jps.WalkableSurface()
floor0 = surface.add_region(polygon=ground_polygon, height=0.0, name="floor0")
floor1 = surface.add_region(polygon=upper_polygon, height=3.0, name="floor1")
staircase = surface.connect_regions(
    from_region=floor0,
    from_edge=((20, 0), (20, 2)),
    to_region=floor1,
    to_edge=((26, 0), (26, 2)),
    name="staircase",
)

# (10, 5) lies on both floors: the region has to be named.
print(surface.regions_at((10, 5)))  # (floor0, floor1)

sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)

exit_ground = sim.add_exit(
    [(0, 4), (0.5, 4), (0.5, 6), (0, 6)], region=floor0, name="exit_ground"
)
evacuate = jps.Route([exit_ground])

# The staircase is not a stage: the exit's floor field routes over it.
upstairs = [
    sim.add_agent(floor1.at(x, 5.0), route=evacuate)
    for x in (10.0, 12.0, 14.0, 16.0)
]
print(upstairs[0].stage)  # Exit(id=0, name="exit_ground")

# Travel times to the exit, as seen on the upper floor.
grid = sim.travel_times(exit_ground)[floor1]
print(f"longest travel time upstairs: {grid.values.max():.1f} s")

while sim.agent_count() > 0 and sim.elapsed_time() < 300:
    sim.iterate()
    for agent in upstairs:
        if agent.alive and agent.position.region == staircase:
            print(f"t={sim.elapsed_time():.1f} s: {agent} on the staircase")
