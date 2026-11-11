# Route API draft, example 3: weighted split.
# Sketch against examples/route_api_draft.py; the API does not exist yet.
#
# A hall with a decision point in the middle and two exits. Two of every
# three agents take the left exit, one the right. Once half of the agents
# have decided, the split is changed to 1:1. The change is triggered by
# state, not by time: the agents reach the decision point within seconds.

from collections import Counter

import jupedsim as jps
import shapely

hall_polygon = shapely.Polygon([(0, 0), (30, 0), (30, 20), (0, 20)])

surface = jps.WalkableSurface()
hall = surface.add_region(polygon=hall_polygon, height=0.0, name="hall")

sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)

decision = sim.add_waypoint((15, 10), radius=1.0, region=hall, name="decision")
left = sim.add_exit(
    [(0, 9), (0.5, 9), (0.5, 11), (0, 11)], region=hall, name="left"
)
right = sim.add_exit(
    [(29.5, 9), (30, 9), (30, 11), (29.5, 11)], region=hall, name="right"
)

split = sim.add_route(
    [(decision, jps.RoundRobin({left: 2, right: 1}))],
    start=decision,
)
print(split)
# Route(start=decision)
#   decision -> RoundRobin(left: 2, right: 1)

positions = jps.distribute_by_number(
    polygon=shapely.Polygon([(10, 2), (20, 2), (20, 6), (10, 6)]),
    number_of_agents=60,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
for x, y in positions:
    sim.add_agent(hall.at(x, y), route=split)

# Count the exits agents actually left through (None: agent.remove()).
counts = Counter()
changed = False
while sim.agent_count() > 0 and sim.elapsed_time() < 300:
    sim.iterate()
    counts.update(removal.exit for removal in sim.removed_agents())
    # Agents whose stage is still decision have not decided yet.
    pending = sim.agents_targeting(decision, route=split)
    if not changed and len(pending) <= 30:
        # Acceptance: some agents decide under the changed weights.
        assert pending, "all agents decided before the weight change"
        # A patch: right keeps its weight, so now left: 1, right: 1.
        split.update_weights(decision, {left: 1})
        changed = True

print({exit_.name: counts[exit_] for exit_ in (left, right)})
