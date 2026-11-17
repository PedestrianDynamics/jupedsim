#! /usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Passengers wait on a platform and board a train through two doors.

- A platform of 30 m x 8 m. 40 passengers start at the back.
- They walk to a waiting area in the middle of the platform and wait there, each on a spot
  of their own.
- At t = 20 s the train arrives. The passengers are released and walk to the platform edge.
- They board through two doors on the edge, at x = 8-10 m and x = 20-22 m, alternating 1:1
  between the doors.
- At t = 35 s door 1 jams. Anyone still heading for door 1 boards through door 2 instead.
- At the end, print whether everyone is aboard, or how many passengers were left behind.

Runs without a viewer by default and writes the trajectory to an HDF5 file; pass --viewer
to watch it.
"""

import argparse
import pathlib

import jupedsim as jps
import shapely

parser = argparse.ArgumentParser()
parser.add_argument(
    "--viewer",
    action="store_true",
    help="watch the simulation in the 3D viewer instead of running it to the end",
)
parser.add_argument(
    "--output",
    type=pathlib.Path,
    default=pathlib.Path("ralf_train_platform_boarding_example.h5"),
    help="HDF5 file to write the trajectory to",
)
args = parser.parse_args()

PLATFORM = shapely.box(0, 0, 30, 8)
BACK_OF_PLATFORM = shapely.box(1, 0.5, 29, 2.5)
WAITING_AREA = shapely.box(6, 3, 24, 5.5)
PLATFORM_EDGE = shapely.box(6, 6, 24, 7)
DOOR1 = shapely.box(8, 7.5, 10, 8)
DOOR2 = shapely.box(20, 7.5, 22, 8)

PASSENGER_COUNT = 40
TRAIN_ARRIVES = 20.0
DOOR1_JAMS = 35.0
TRAIN_DEPARTS = 100.0

geo_builder = jps.GeometryBuilder()
platform = geo_builder.add_region(polygon=PLATFORM, height=0, name="platform")

sim = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geo_builder.build(),
    trajectory_writer=jps.Hdf5TrajectoryWriter(output_file=args.output),
)

waiting_area = sim.add_target(
    jps.Waypoint([(WAITING_AREA, platform)]), name="waiting area"
)
platform_edge = sim.add_target(
    jps.Waypoint([(PLATFORM_EDGE, platform)]), name="platform edge"
)
door1 = sim.add_target(jps.Exit([(DOOR1, platform)]), name="door 1")
door2 = sim.add_target(jps.Exit([(DOOR2, platform)]), name="door 2")

to_waiting_area = sim.add_route(
    [jps.FixedTransition(jps.AGENT_LOCATION, waiting_area)],
    name="to waiting area",
)
# Round robin: passengers may head for a particular door because of their seat
# reservation.
boarding = sim.add_route(
    [
        jps.FixedTransition(jps.AGENT_LOCATION, platform_edge),
        jps.RoundRobinTransition(platform_edge, {door1: 1, door2: 1}),
    ],
    name="boarding",
)

# The waiting area is split into random waiting spots, which passengers are steered to.
waiting_spots = [
    platform.at(x, y)
    for x, y in jps.distribute_by_number(
        polygon=WAITING_AREA,
        number_of_agents=2 * PASSENGER_COUNT,
        distance_to_agents=0.5,
        distance_to_polygon=0.1,
        seed=42,
    )
]

for x, y in jps.distribute_by_number(
    polygon=BACK_OF_PLATFORM,
    number_of_agents=PASSENGER_COUNT,
    distance_to_agents=0.5,
    distance_to_polygon=0.2,
    seed=1,
):
    sim.add_agent(platform.at(x, y), route=to_waiting_area)

train_arrived = False


def on_step(sim):
    global train_arrived
    if sim.elapsed_time() < TRAIN_ARRIVES:
        # Only those that reached the waiting area before the train arrives get a spot.
        for completion in sim.target_completions():
            if completion.target == waiting_area:
                sim.agent(completion.agent_id).reroute(
                    steer_to=waiting_spots.pop()
                )
    elif not train_arrived:
        train_arrived = True
        for agent in sim.agents():
            agent.reroute(route=boarding)

    # Every step, not once: the round robin at the platform edge keeps sending
    # passengers to door 1.
    if sim.elapsed_time() >= DOOR1_JAMS:
        for agent in sim.agents(filter=lambda a: a.current_target == door1):
            agent.reroute(first_target=door2)


with sim:
    if args.viewer:
        # Imported here: the viewer's dependencies are not needed without it.
        from simulation_viewer import SimulationViewer

        SimulationViewer(sim, on_step=on_step).run()
    else:
        while sim.elapsed_time() < TRAIN_DEPARTS and sim.agent_count() > 0:
            sim.iterate()
            on_step(sim)

    left_behind = sim.agent_count()
    if left_behind == 0:
        print("Everyone is aboard.")
    else:
        print(f"{left_behind} passengers were left behind.")
