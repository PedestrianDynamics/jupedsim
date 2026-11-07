# Route API draft, example 5: closure with rerouting.
#
# A hall with two exits. Agents pick the exit with the least travel time.
# When the first agent leaves through the east exit, smoke blocks it: it
# is closed, and agents still heading for the corridor in front of it
# switch to a route via the west corridor. The closure is triggered by
# state, not by time, so east-bound agents are still in the hall.

import sys

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

# One route per exit; each agent takes the exit it reaches fastest.
via_east = jps.Route([east_corridor, east])
via_west = jps.Route([west_corridor, west])
routes = {east: via_east, west: via_west}

positions = jps.distribute_by_number(
    polygon=shapely.Polygon([(16, 6), (24, 6), (24, 14), (16, 14)]),
    number_of_agents=80,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
for x, y in positions:
    location = hall.at(x, y)
    best = min(routes, key=lambda e: sim.travel_time(e, location))
    sim.add_agent(location, route=routes[best])

smoke = False
affected = 0
while sim.agent_count() > 0 and sim.elapsed_time() < 600:
    sim.iterate()
    left_east = any(r.exit == east for r in sim.removed_agents())
    if smoke or not left_east:
        continue
    smoke = True
    # Agents still heading for the east corridor take the west route.
    # They and the agents stranded below are disjoint sets (target
    # east_corridor vs. target east), so the order of the two loops does
    # not matter.
    heading = sim.agents_targeting(east_corridor, route=via_east)
    for agent in heading:
        agent.follow(via_west)
    # Agents already past the corridor are stranded at the closed exit or
    # rerouted; with a fixed transition corridor -> east they are stranded.
    stranded = east.close(informed=True)
    for agent in stranded:
        agent.follow(via_west)
    affected = len(heading) + len(stranded)
    # Acceptance: the closure reroutes at least one agent.
    assert affected > 0, "no agent was heading east when it closed"
    print(f"t={sim.elapsed_time():.1f} s: east closed, {affected} rerouted")

if sim.agent_count() == 0:
    print(f"evacuated after {sim.elapsed_time():.1f} s")
else:
    print(f"time limit reached; {sim.agent_count()} agents remain")
    sys.exit(1)
# The closure happened and affected agents.
assert smoke, "the east exit never closed"
assert affected > 0, "the closure affected no agent"
