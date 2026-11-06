# Route API draft, example 5: closure with rerouting.
# Sketch against examples/route_api_draft.py; the API does not exist yet.
#
# A hall with two exits. Agents pick the exit with the least travel time.
# After 30 s smoke blocks the east exit: it is closed, and agents still
# heading for the corridor in front of it switch to a route via the
# west corridor.

import jupedsim as jps
import shapely

hall_polygon = shapely.Polygon([(0, 0), (40, 0), (40, 20), (0, 20)])

surface = jps.WalkableSurface()
hall = surface.add_region(polygon=hall_polygon, height=0.0, name="hall")

sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)

east_corridor = sim.add_waypoint(
    (32, 10), radius=1.0, region=hall, name="east_corridor"
)
west_corridor = sim.add_waypoint(
    (8, 10), radius=1.0, region=hall, name="west_corridor"
)
east = sim.add_exit(
    [(39.5, 9), (40, 9), (40, 11), (39.5, 11)], region=hall, name="east"
)
west = sim.add_exit(
    [(0, 9), (0.5, 9), (0.5, 11), (0, 11)], region=hall, name="west"
)

# The entry rule picks a corridor when the agent is added; the corridors
# lead to their exits.
choose = jps.Route(
    [(east_corridor, east), (west_corridor, west)],
    start=jps.LeastTravelTime([east_corridor, west_corridor]),
)
via_west = jps.Route([west_corridor, west])

positions = jps.distribute_by_number(
    polygon=shapely.Polygon([(16, 6), (24, 6), (24, 14), (16, 14)]),
    number_of_agents=80,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
for x, y in positions:
    sim.add_agent(hall.at(x, y), route=choose)

smoke = False
while sim.agent_count() > 0 and sim.elapsed_time() < 600:
    sim.iterate()
    if smoke or sim.elapsed_time() < 30:
        continue
    smoke = True
    # Agents still heading for the east corridor take the west route.
    for agent in sim.agents_targeting(east_corridor, route=choose):
        agent.follow(via_west)
    # Agents already past the corridor are stranded at the closed exit or
    # rerouted; with a fixed transition corridor -> east they are stranded.
    stranded = east.close(informed=True)
    for agent in stranded:
        agent.follow(via_west)
    print(
        f"t={sim.elapsed_time():.1f} s: east closed, {len(stranded)} rerouted"
    )

print(f"evacuated after {sim.elapsed_time():.1f} s")
