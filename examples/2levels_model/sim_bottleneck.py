# SPDX-License-Identifier: LGPL-3.0-or-later
import pathlib
import jupedsim as jps
import pedpy
import numpy as np
from numpy.random import normal  # normal distribution of free movement speed
from shapely import Polygon, GeometryCollection

## Setup geometries
room1 = Polygon([(-10, -10), (10, -10), (10, 10), (-10, 10)])
room2 = Polygon([(15, -5), (25, -5), (25, 5), (15, 5)])
corridor = Polygon([(10, -0.38), (28, -0.38), (28, 0.38), (10, 0.38)])

area = GeometryCollection(corridor.union(room1.union(room2)))
walkable_area = pedpy.WalkableArea(area.geoms[0])
# pedpy.plot_walkable_area(walkable_area=walkable_area).set_aspect("equal")

## Setup spawning area
spawning_area = Polygon([(-8, -8), (8, -8), (8, 8), (-8, 8)])
num_agents = 5
pos_in_spawning_area = jps.distributions.distribute_by_number(
    polygon=spawning_area,
    number_of_agents=num_agents,
    distance_to_agents=0.6,
    distance_to_polygon=0.15,
    seed=1,
)
exit_area = Polygon([(27, -2), (28, -2), (28, 2), (27, 2)])


## Setup Simulation
trajectory_file = "bottleneck_SocialForceModelIPP.sqlite"  # output file
writer = jps.SqliteIPPTrajectoryWriter(
    output_file=pathlib.Path(trajectory_file)
)
simulation = jps.Simulation(
    model=jps.SocialForceModelIPP(),
    geometry=area,
    trajectory_writer=writer,
)

exit_id = simulation.add_exit_stage(exit_area.exterior.coords[:-1])
journey = jps.JourneyDescription([exit_id])
journey_id = simulation.add_journey(journey)

## Spawn agents
v_distribution = normal(1.5, 0.2, num_agents)

for pos, v0 in zip(pos_in_spawning_area, v_distribution):
    agent_id = simulation.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=pos,
            orientation=(0.0, 0.0),
            journey_id=journey_id,
            stage_id=exit_id,
            velocity=(0.0, 0.0),
            ground_support_position=pos,
            ground_support_velocity=(0.0, 0.0),
            height=1.75,            # agent height [m]
            desired_speed=v0,       # v0 [m/s]
            reaction_time=0.5,      # tau [s]
            lambda_u=0.5,           # unbalancing rate [1/s]
            lambda_b=1.0,           # balancing rate [1/s]
            balance_speed=1.0,      # coupling speed v_s [m/s]
            damping=1.0,            # velocity dissipation [1/s]
            agent_scale=5.0,        # A repulsion amplitude [N]
            force_distance=0.5,     # B upper body range [m]
            leg_force_distance=0.3, # B_leg leg range [m]
            radius=0.15,            # upper body radius [m]
        )
    )

## run simulation
max_iteration = 8000
while (
    simulation.agent_count() > 0
    and simulation.iteration_count() <= max_iteration
):
    simulation.iterate()
    if simulation.iteration_count() == max_iteration:
        print("Simulation stopped after " + str(max_iteration) + " iterations.")

writer.close()
