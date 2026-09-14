# SPDX-License-Identifier: LGPL-3.0-or-later
"""Five agents walking up a U-shaped stair, from the ground floor to an exit above.

Run from the repo root:

    PYTHONPATH=python_modules/jupedsim:build/lib python examples/example_3d.py

The world is a surface mesh, so every place in it needs a height as well as an
(x, y): the two floors sit on top of each other and share their footprint.
That is what the ``z_hint`` arguments below are for -- the exit is the one
upstairs, the agents start downstairs.
"""

from pathlib import Path

import jupedsim as jps
from simulation_viewer import SimulationViewer

OBJ = Path(__file__).parents[0] / "geometry/multi_level_u_stair.obj"

GROUND_FLOOR = 0.0
UPPER_FLOOR = 3.0

sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=OBJ)

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


viewer = SimulationViewer(sim, on_step=on_step, geometry_obj=OBJ)
viewer.run()
