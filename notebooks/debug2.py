from shapely import GeometryCollection, Polygon, to_wkt
import pathlib
import jupedsim as jps
import pedpy
import pandas as pd
import numpy as np
from numpy.random import normal  # normal distribution of free movement speed
import matplotlib.pyplot as plt

spawning_area = Polygon([(0, 0), (6, 0), (6, 2), (0, 2)])
num_agents = 20
pos_in_spawning_area = jps.distributions.distribute_by_number(
    polygon=spawning_area,
    number_of_agents=num_agents,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
exit_area = Polygon([(10, 11), (12, 11), (12, 12), (10, 12)])
trajectory_file = "corner.sqlite"
area = Polygon([(0, 0), (12, 0), (12, 12), (10, 12), (10, 2), (0, 2)])
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=area,
    trajectory_writer=jps.SqliteTrajectoryWriter(
        output_file=pathlib.Path(trajectory_file)
    ),
)
exit_id = simulation.add_exit_stage(exit_area.exterior.coords[:-1])
journey = jps.JourneyDescription([exit_id])
journey_id = simulation.add_journey(journey)

v_distribution = normal(1.34, 0.05, num_agents)

for pos, v0 in zip(pos_in_spawning_area, v_distribution):
    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            journey_id=journey_id, stage_id=-1, position=pos, v0=v0
            # position=pos, v0=v0
        )
    )
