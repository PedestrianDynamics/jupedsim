# SPDX-License-Identifier: LGPL-3.0-or-later
"""An office block of five storeys emptying through a single ground-floor exit.

Run from the repo root:

    PYTHONPATH=python_modules/jupedsim:build/lib python examples/example_office_5floor.py

The building is a surface mesh: five storeys at z = 0, 3.5, 7, 10.5 and 14, each a row of
rooms north and south of a central corridor, joined by two U-shaped stairwells --

    west stairwell   x 8.5..10.5,  y 1.7..4.7   (off the south rooms)
    east stairwell   x 35.5..37.5, y 11.4..14.4 (off the north rooms)

The only way out is a door on the south face of a room roughly midway between the two
stairwells. Nobody is told which stairwell to use: everyone targets that one exit, and the
router sends each of them down the one that is nearer. Which is the point of putting the exit
in the middle -- where an agent starts decides the way it goes out.
"""

from pathlib import Path

import jupedsim as jps
from simulation_viewer import SimulationViewer

OBJ = Path(__file__).parents[0] / "geometry/office_5floors.obj"

# Storey heights, ground floor first.
FLOORS = [0.0, 3.5, 7.0, 10.5, 14.0]

# Room centres, as (x, y): rooms sit either side of the corridor at y 7..9.
SOUTH, NORTH = 3.5, 12.5

# The way out: a door on the south face, in the room around x = 21 -- almost exactly
# between the two stairwells, so neither is the obvious one from everywhere.
EXIT_POLYGON = [(20.2, 0.4), (22.4, 0.4), (22.4, 1.2), (20.2, 1.2)]

# Five together in a room on the top floor, far east: their way down is the east stairwell.
TOP_FLOOR_GROUP = [
    (39.4, NORTH),
    (40.1, NORTH),
    (40.8, NORTH),
    (39.75, NORTH - 0.8),
    (40.45, NORTH - 0.8),
]

# Ten more scattered over the other storeys. Most of them start west of the exit, so most
# take the west stairwell -- but not all, and that is what makes it worth watching.
SCATTERED = [
    (2.15, NORTH, FLOORS[1]),
    (5.95, NORTH, FLOORS[1]),
    (29.0, SOUTH, FLOORS[1]),
    (5.95, SOUTH, FLOORS[2]),
    (13.6, SOUTH, FLOORS[2]),
    (17.45, NORTH, FLOORS[2]),
    (5.95, NORTH, FLOORS[3]),
    (2.15, NORTH, FLOORS[3]),
    (13.6, SOUTH, FLOORS[3]),
    (32.8, SOUTH, FLOORS[4]),
]


def build() -> jps.Simulation:
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=OBJ, dt=0.01
    )
    exit_id = sim.add_exit_stage(EXIT_POLYGON, z_hint=FLOORS[0])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    starts = [(x, y, FLOORS[-1]) for x, y in TOP_FLOOR_GROUP] + SCATTERED
    for x, y, z in starts:
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=(x, y),
            state=jps.CollisionFreeSpeedModelState(),
            z_hint=z,
        )
    return sim


if __name__ == "__main__":
    sim = build()
    viewer = SimulationViewer(sim, geometry_obj=OBJ)
    viewer.run()
