"""Example simulation using the Anticipation Velocity Model (AVM) in Jupedsim with a geometry that has sharp corners."""

import pathlib

import jupedsim as jps
import pedpy
from shapely.geometry import Polygon
from shapely.ops import unary_union

N = 1000

# Define the list of coordinate lists
default_geometry_coordinates = [
    [(19.35, 56.24), (24.38, 64.50), (49.15, 49.15), (41.80, 42.57)],
    [(41.80, 42.57), (43.86, 31.48), (48.63, 32.51), (49.15, 49.15)],
    [(48.63, 32.51), (49.15, 49.15), (50.31, 50.57), (58.31, 46.05)],
    [(48.63, 32.51), (51.21, 20.00), (67.08, 40.25), (58.31, 46.05)],
    [(51.21, 20.00), (56.63, 17.54), (69.40, 38.83), (67.08, 40.25)],
    [(58.31, 46.05), (67.08, 40.25), (88.03, 77.40), (72.11, 69.14)],
    [
        (72.11, 69.14),
        (73.01, 69.84),
        (79.78, 81.86),
        (79.08, 82.56),
        (85.14, 79.85),
        (88.03, 77.40),
    ],
    [(79.08, 82.56), (85.14, 79.85), (89.78, 86.95), (85.40, 92.49)],
    [(79.85, 95.33), (79.85, 103.97), (87.98, 102.56), (85.40, 92.49)],
    [(102.81, 70.82), (106.04, 72.24), (111.71, 68.11), (110.04, 65.40)],
    [(89.78, 86.95), (92.88, 90.56), (106.04, 72.24), (102.81, 70.82)],
    [(85.40, 92.49), (92.75, 93.91), (92.88, 90.56), (89.78, 86.95)],
    [(85.40, 92.49), (87.98, 102.56), (97.78, 100.62), (92.75, 93.91)],
    [(87.98, 102.56), (97.78, 100.62), (114.42, 103.59), (114.04, 106.17)],
    [
        (114.42, 103.59),
        (97.78, 100.62),
        (92.75, 93.91),
        (92.88, 90.56),
        (106.04, 72.24),
        (111.71, 68.11),
    ],
]


for coords in default_geometry_coordinates:
    poly = Polygon(coords)
    if not poly.is_valid:
        print("Invalid polygon:", coords)
    if not poly.is_simple:
        print("Non-simple polygon:", coords)

print("Geometry is fine!")

polygons = [Polygon(coords) for coords in default_geometry_coordinates]

walkable_area = unary_union(polygons)
walkable_area = pedpy.WalkableArea(walkable_area)
pedpy.plot_walkable_area(
    walkable_area=walkable_area, color="lightblue", alpha=0.5
)


print(f"Distributing {N} agents.")
output_file = "traj.sqlite"
simulation = jps.Simulation(
    model=jps.AnticipationVelocityModel(),
    dt=0.05,
    geometry=walkable_area.polygon,
    trajectory_writer=jps.SqliteTrajectoryWriter(
        output_file=pathlib.Path(output_file), every_nth_frame=1
    ),
)
goal_1 = simulation.add_exit_stage(
    [
        (113.792, 106.106),
        (113.502, 106.082),
        (113.022, 103.660),
        (114.308, 103.700),
    ]
)
goal_2 = simulation.add_exit_stage(
    [
        (109.723, 69.131),
        (110.907, 68.319),
        (109.883, 65.822),
        (108.334, 67.112),
    ]
)

journey1 = jps.JourneyDescription([goal_1])
journey2 = jps.JourneyDescription([goal_2])

journey1id = simulation.add_journey(journey1)
journey2id = simulation.add_journey(journey2)

spawning_area = walkable_area.polygon

pos_in_spawning_area = jps.distribute_by_number(
    polygon=spawning_area,
    number_of_agents=N,
    distance_to_agents=0.30,
    distance_to_polygon=0.15,
    seed=1,
)


for i, pos in enumerate(pos_in_spawning_area):
    goal_id = goal_1 if i % 2 == 0 else goal_2
    journey_id = journey1id if i % 2 == 0 else journey2id
    simulation.add_agent(
        parameters=jps.AnticipationVelocityModelAgentParameters(
            journey_id=journey_id,
            position=pos,
            stage_id=goal_id,
            radius=0.15,
        )
    )

niterations = 15000
print(f"Start simulation with max {niterations} iterations.")

while (
    simulation.agent_count() > 0 and simulation.iteration_count() < niterations
):
    simulation.iterate()

simulation._writer.close()

print(
    f"Simulation finished after {simulation.iteration_count()} iteration ({simulation.elapsed_time()} seconds.)"
)
print(f"Agents remaining: {simulation.agent_count()}.")
