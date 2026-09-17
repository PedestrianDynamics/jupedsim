# SPDX-License-Identifier: LGPL-3.0-or-later
"""Five agents walking up a U-shaped stair, from the ground floor to an exit above."""

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
    exterior=rect(0, 0, 20, 20),
    interior=[STAIRWELL, rect(3, 3, 5, 5), rect(15, 15, 17, 17)],
    height=GROUND_FLOOR,
)
upper = surface.add_region(
    exterior=rect(0, 0, 20, 20),
    interior=[STAIRWELL, rect(15, 3, 17, 5), rect(3, 15, 5, 17)],
    height=UPPER_FLOOR,
)
landing = surface.add_region(
    exterior=[(13, 6), (14, 6), (14, 12), (13, 12), (13, 9)],
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

exit_id = sim.add_exit_stage(
    [(3.5, 12.5), (4.5, 12.5), (4.5, 13.5), (3.5, 13.5)],
    z_hint=UPPER_FLOOR,
)
journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

start_positions = [
    (3.3, 7.6),
    (4.0, 7.6),
    (3.3, 8.3),
    (4.0, 8.3),
    (3.65, 6.9),
]
for position in start_positions:
    sim.add_agent(
        journey_id=journey_id,
        stage_id=exit_id,
        position=position,
        state=jps.CollisionFreeSpeedModelState(),
        z_hint=GROUND_FLOOR,
    )


def on_step(sim):
    # Runs on every iterate step. This is where one could add / retarget agents.
    # We don't in this example.
    pass


viewer = SimulationViewer(sim, on_step=on_step)
viewer.run()
