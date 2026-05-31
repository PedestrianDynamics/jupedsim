"""Minimal end-to-end jupedsim simulation.

Builds a rectangular room with one exit, places agents with the
Collision-Free Speed Model, and iterates until everyone has left.
"""

import pathlib

import jupedsim as jps
import shapely

# 1. Geometry: a 10 m x 10 m room (walkable area as a shapely polygon).
geometry = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])

# 2. Simulation: choose an operational model, geometry, and trajectory output.
trajectory_file = pathlib.Path("basic_simulation.sqlite")
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geometry,
    trajectory_writer=jps.SqliteTrajectoryWriter(output_file=trajectory_file),
)

# 3. Stages + journey: a single exit the agents walk to.
exit_id = simulation.add_exit_stage([(9, 4), (10, 4), (10, 6), (9, 6)])
journey_id = simulation.add_journey(jps.JourneyDescription([exit_id]))

# 4. Agents: place a few on a small grid on the left side of the room.
positions = jps.distributions.distribute_by_number(
    polygon=shapely.Polygon([(0.5, 0.5), (3, 0.5), (3, 9.5), (0.5, 9.5)]),
    number_of_agents=20,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
for position in positions:
    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            journey_id=journey_id,
            stage_id=exit_id,
            position=position,
            radius=0.12,
        )
    )

# 5. Run until the room is empty (with an iteration safety cap).
while simulation.agent_count() > 0 and simulation.iteration_count() < 10_000:
    simulation.iterate()

print(
    f"Evacuated {20} agents in {simulation.iteration_count()} iterations "
    f"({simulation.elapsed_time():.1f} s). Trajectories: {trajectory_file}"
)
