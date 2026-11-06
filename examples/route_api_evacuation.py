# Route API draft, example 1: simple evacuation.
# Sketch against examples/route_api_draft.py; the API does not exist yet.
#
# One room of 10 m x 10 m with a door on the right wall. 50 agents walk to
# the exit and leave.

import jupedsim as jps
import shapely

room_polygon = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])

surface = jps.WalkableSurface()
room = surface.add_region(polygon=room_polygon, height=0.0, name="room")

sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)

door = sim.add_exit(
    [(9.5, 4.5), (10, 4.5), (10, 5.5), (9.5, 5.5)], region=room, name="door"
)
evacuate = jps.Route([door])

# distribute_by_number returns (x, y) pairs; each becomes a location in room.
# GAP: every distribution result has to be wrapped with room.at(); a
# distribution helper that returns locations would avoid that.
positions = jps.distribute_by_number(
    polygon=room_polygon,
    number_of_agents=50,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
for x, y in positions:
    sim.add_agent(room.at(x, y), route=evacuate)

while sim.agent_count() > 0 and sim.elapsed_time() < 300:
    sim.iterate()

print(f"evacuated after {sim.elapsed_time():.1f} s")
