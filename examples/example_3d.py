# SPDX-License-Identifier: LGPL-3.0-or-later
"""Example showing nearest waypoint/exit stage routing.

Six agents walk towards a waypoint with multiple areas and each targets the area closer
to its start position. Down the stairs, use different point-based waypoints to split the
agents and use an exit with multiple areas. Once again agents target the nearer one.
"""

from itertools import pairwise

import jupedsim as jps
from simulation_viewer import SimulationViewer

GROUND_FLOOR = 0.0
LANDING = 1.5
UPPER_FLOOR = 3.0


def rect(x0, y0, x1, y1):
    return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]


# (8, 9) and (13, 9) split the stairwell sides so each run has an edge of its own to attach to.
STAIRWELL = [(8, 6), (14, 6), (14, 12), (8, 12), (8, 9)]

surface = jps.WalkableSurface()
ground = surface.add_region(
    boundary=rect(0, 0, 20, 20),
    holes=[STAIRWELL, rect(3, 3, 5, 5), rect(15, 15, 17, 17)],
    height=GROUND_FLOOR,
)
upper = surface.add_region(
    boundary=rect(0, 0, 20, 20),
    holes=[STAIRWELL, rect(15, 3, 17, 5), rect(3, 15, 5, 17)],
    height=UPPER_FLOOR,
)
landing = surface.add_region(
    boundary=[(13, 6), (14, 6), (14, 12), (13, 12), (13, 9)],
    height=LANDING,
)
surface.connect_regions(
    from_region=ground,
    from_edge=((8, 6), (8, 9)),
    to_region=landing,
    to_edge=((13, 6), (13, 9)),
)
surface.connect_regions(
    from_region=landing,
    from_edge=((13, 9), (13, 12)),
    to_region=upper,
    to_edge=((8, 9), (8, 12)),
)

sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)

# An area on either side of the stairwell, close to the outer wall.
upper_waypoint_id = sim.add_waypoint_stage(
    [(rect(10, 0.5, 12, 1.5), upper), (rect(10, 18.5, 12, 19.5), upper)]
)
north_waypoint_id = sim.add_waypoint_stage(
    [(jps.circle_to_polygon(center=(11, 16), radius=1), ground)]
)
south_waypoint_id = sim.add_waypoint_stage(
    [(jps.circle_to_polygon(center=(11, 2), radius=1), ground)]
)
# One exit in the north-west corner and one in the south-east corner.
exit_id = sim.add_exit_stage(
    [(rect(0.5, 17, 1.5, 19), ground), (rect(18.5, 1, 19.5, 3), ground)]
)


def add_journey_via(ground_waypoint_id):
    stages = [upper_waypoint_id, ground_waypoint_id, exit_id]
    journey = jps.JourneyDescription(stages)
    for stage_id, next_stage_id in pairwise(stages):
        journey.set_transition_for_stage(
            stage_id, jps.Transition.create_fixed_transition(next_stage_id)
        )
    return sim.add_journey(journey)


journey_ids = [
    add_journey_via(north_waypoint_id),
    add_journey_via(south_waypoint_id),
]

# Three agents south and three north of the stairwell's centre line.
start_positions = [
    (17.5, 6.5),
    (18.5, 6.5),
    (18.0, 7.3),
    (17.5, 11.5),
    (18.5, 11.5),
    (18.0, 10.7),
]
for i, position in enumerate(start_positions):
    sim.add_agent(
        journey_id=journey_ids[i % 2],
        stage_id=upper_waypoint_id,
        position=position,
        state=jps.CollisionFreeSpeedModelState(),
        region_id=upper,
    )


def on_step(sim):
    # Runs on every iterate step. This is where one could add / retarget agents.
    # We don't in this example.
    pass


viewer = SimulationViewer(sim, on_step=on_step)
viewer.run()
