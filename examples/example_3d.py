# SPDX-License-Identifier: LGPL-3.0-or-later
"""Example showing nearest waypoint/exit stage routing.

Six agents walk towards a waypoint with multiple areas and each targets the area closer
to its start position. Down the stairs, use different point-based waypoints to split the
agents and use an exit with multiple areas. Once again agents target the nearer one.
"""

import argparse
import pathlib
from itertools import pairwise

import jupedsim as jps

parser = argparse.ArgumentParser()
parser.add_argument(
    "--viewer",
    action="store_true",
    help="watch the simulation in the 3D viewer instead of running it to the end",
)
parser.add_argument(
    "--output",
    type=pathlib.Path,
    default=pathlib.Path("example_3d.h5"),
    help="HDF5 file to write the trajectory to",
)
args = parser.parse_args()

MAX_ITERATIONS = 20000

GROUND_FLOOR = 0.0
LANDING = 1.5
UPPER_FLOOR = 3.0


def rect(x0, y0, x1, y1):
    return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]


# (8, 9) and (13, 9) split the stairwell sides so each run has an edge of its own to attach to.
STAIRWELL = [(8, 6), (14, 6), (14, 12), (8, 12), (8, 9)]

geo_builder = jps.GeometryBuilder()
ground = geo_builder.add_region(
    boundary=rect(0, 0, 20, 20),
    holes=[STAIRWELL, rect(3, 3, 5, 5), rect(15, 15, 17, 17)],
    height=GROUND_FLOOR,
)
upper = geo_builder.add_region(
    boundary=rect(0, 0, 20, 20),
    holes=[STAIRWELL, rect(15, 3, 17, 5), rect(3, 15, 5, 17)],
    height=UPPER_FLOOR,
)
landing = geo_builder.add_region(
    boundary=[(13, 6), (14, 6), (14, 12), (13, 12), (13, 9)],
    height=LANDING,
)
geo_builder.connect_regions(
    from_region=ground,
    from_edge=((8, 6), (8, 9)),
    to_region=landing,
    to_edge=((13, 6), (13, 9)),
)
geo_builder.connect_regions(
    from_region=landing,
    from_edge=((13, 9), (13, 12)),
    to_region=upper,
    to_edge=((8, 9), (8, 12)),
)

sim = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geo_builder.build(),
    trajectory_writer=jps.Hdf5TrajectoryWriter(output_file=args.output),
)

# An area on either side of the stairwell, close to the outer wall.
upper_waypoint = sim.add_target(
    jps.Waypoint(
        [(rect(10, 0.5, 12, 1.5), upper), (rect(10, 18.5, 12, 19.5), upper)]
    )
)
north_waypoint = sim.add_target(
    jps.Waypoint([(jps.circle_to_polygon(center=(11, 16), radius=1), ground)])
)
south_waypoint = sim.add_target(
    jps.Waypoint([(jps.circle_to_polygon(center=(11, 2), radius=1), ground)])
)
# One exit in the north-west corner and one in the south-east corner.
exit_ = sim.add_target(
    jps.Exit(
        [(rect(0.5, 17, 1.5, 19), ground), (rect(18.5, 1, 19.5, 3), ground)]
    )
)


def add_route_via(ground_waypoint):
    targets = [upper_waypoint, ground_waypoint, exit_]
    return sim.add_route(
        [jps.FixedTransition(jps.AGENT_LOCATION, targets[0])]
        + [
            jps.FixedTransition(source, target)
            for source, target in pairwise(targets)
        ]
    )


routes = [
    add_route_via(north_waypoint),
    add_route_via(south_waypoint),
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
        upper.at(*position),
        route=routes[i % 2],
        state=jps.CollisionFreeSpeedModelState(),
    )


def on_step(sim):
    # Runs on every iterate step. This is where one could add / retarget agents.
    # We don't in this example.
    pass


with sim:
    if args.viewer:
        # Imported here: the viewer's dependencies are not needed without it.
        from simulation_viewer import SimulationViewer

        SimulationViewer(sim, on_step=on_step).run()
    else:
        while sim.agent_count() > 0 and sim.iteration_count() < MAX_ITERATIONS:
            sim.iterate()
        print(
            f"{sim.agent_count()} agents left after {sim.iteration_count()} iterations"
        )
