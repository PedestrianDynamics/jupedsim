# "Last train" with the 2.0 route API draft.
# Written against examples/route_api_draft.py.
# Same scenario, geometry and seed as last_train_v1.py (JuPedSim 1.x).
#
# Platform 30 m x 8 m. 40 passengers enter at the back and wait in a
# waiting area. At t = 20 s the train arrives: they are released, walk to
# the platform edge and board through two doors, 1:1. At t = 35 s door 1
# jams: passengers heading for it board through door 2 instead.

import sys
from collections import Counter

import jupedsim as jps
import shapely

platform_polygon = shapely.box(0, 0, 30, 8)

surface = jps.WalkableSurface()
platform = surface.add_region(
    polygon=platform_polygon, height=0.0, name="platform"
)

sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)

# 1.x waiting set -> a waypoint over the waiting area plus a hold: agents
# are waiting in an area and stop where they enter it. This does not
# reproduce the 1.x waiting set's 40 assigned spots at y = 3 and y = 4.
waiting = sim.add_waypoint(
    shapely.box(5.5, 2.5, 25.5, 4.5), region=platform, name="waiting"
)
edge = sim.add_waypoint((15, 6.5), radius=1.0, region=platform, name="edge")
door1 = sim.add_exit(shapely.box(8, 7.5, 10, 8), region=platform, name="door1")
door2 = sim.add_exit(shapely.box(20, 7.5, 22, 8), region=platform, name="door2")

boarding = jps.Route(
    [
        (waiting, edge),
        (edge, jps.RoundRobin({door1: 1, door2: 1})),
    ],
    start=waiting,
)
# Hold before anyone moves: agents walk to the waiting area and stop.
boarding.hold(waiting)

positions = jps.distribute_by_number(
    polygon=shapely.box(0, 0, 30, 2),
    number_of_agents=40,
    distance_to_agents=0.5,
    distance_to_polygon=0.2,
    seed=1,
)
for x, y in positions:
    sim.add_agent(platform.at(x, y), route=boarding)

counts = Counter()
released = jammed = False
while sim.agent_count() > 0 and sim.elapsed_time() < 120:
    sim.iterate()
    counts.update(removal.exit for removal in sim.removed_agents())
    if not released and sim.elapsed_time() >= 20:
        waiting_now = len(boarding.held_at(waiting))
        print(
            f"t={sim.elapsed_time():.1f} s: train arrived, {waiting_now} waiting"
        )
        boarding.release(waiting)
        released = True
    if not jammed and sim.elapsed_time() >= 35:
        # The rule at edge skips the closed door: agents it sent to door 1
        # are reassigned to door 2 now (informed=True). None is stranded,
        # since door 2 stays open.
        heading = sim.agents_targeting(door1)
        assert len(heading) > 0, "no passenger was heading for door 1"
        door1.close(informed=True)
        print(
            f"t={sim.elapsed_time():.1f} s: door 1 jammed, "
            f"{len(heading)} redirected"
        )
        jammed = True

print({door.name: counts[door] for door in (door1, door2)})
if sim.agent_count() == 0:
    print(f"all aboard after {sim.elapsed_time():.1f} s")
else:
    print(f"train left; {sim.agent_count()} passengers remain")
    sys.exit(1)
assert released and jammed, "the train event never happened"
