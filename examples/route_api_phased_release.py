# Route API draft, example 4: phased release.
#
# Two rooms open into a shared corridor that leads to one exit. Room A
# evacuates at once; room B waits at its door and is released after 60 s.

import sys

import jupedsim as jps
import shapely

# Rooms A and B side by side at the bottom, corridor along the top.
floor_polygon = shapely.Polygon(
    [(0, 0), (10, 0), (10, 8), (12, 8), (12, 0), (22, 0), (22, 12), (0, 12)]
)
room_a = shapely.Polygon([(0, 0), (10, 0), (10, 8), (0, 8)])
room_b = shapely.Polygon([(12, 0), (22, 0), (22, 8), (12, 8)])

surface = jps.WalkableSurface()
floor = surface.add_region(polygon=floor_polygon, height=0.0, name="floor")

sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)

door_a = sim.add_waypoint((5, 8.5), radius=0.8, region=floor, name="door_a")
door_b = sim.add_waypoint((17, 8.5), radius=0.8, region=floor, name="door_b")
exit_ = sim.add_exit(
    [(0, 10), (0.5, 10), (0.5, 12), (0, 12)], region=floor, name="exit"
)

route_a = jps.Route([door_a, exit_])
route_b = jps.Route([door_b, exit_])

for polygon, route in ((room_a, route_a), (room_b, route_b)):
    positions = jps.distribute_by_number(
        polygon=polygon,
        number_of_agents=30,
        distance_to_agents=0.4,
        distance_to_polygon=0.2,
        seed=1,
    )
    for x, y in positions:
        sim.add_agent(floor.at(x, y), route=route)

# Hold room B at its door before anyone moves. The hold applies to door_b
# on route_b until release(), so it also stops agents that target door_b
# later; room A's agents on route_a are not affected.
route_b.hold(door_b)

released = False
while sim.agent_count() > 0 and sim.elapsed_time() < 600:
    sim.iterate()
    if not released and sim.elapsed_time() >= 60:
        # held_at() lists agents stopped at door_b; release() also frees
        # room B agents still walking there.
        waiting_now = len(route_b.held_at(door_b))
        print(f"{waiting_now} agents waiting at door_b")
        assert waiting_now > 0, "room B was not held at door_b"
        route_b.release(door_b)
        released = True

if sim.agent_count() == 0:
    print(f"evacuated after {sim.elapsed_time():.1f} s")
else:
    print(f"time limit reached; {sim.agent_count()} agents remain")
    sys.exit(1)
assert released, "room B was never released"
