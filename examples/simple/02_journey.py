"""Journey with waypoints and a fixed transition.

Agents pass through two waypoints in order, then leave via the exit.
"""

import pathlib

import jupedsim as jps
import shapely

geometry = shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)])

trajectory_file = pathlib.Path("journey.sqlite")
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geometry,
    trajectory_writer=jps.SqliteTrajectoryWriter(output_file=trajectory_file),
)

# Two waypoints (position, arrival distance) and a final exit.
wp1 = simulation.add_waypoint_stage((10, 8), 0.5)
wp2 = simulation.add_waypoint_stage((10, 2), 0.5)
exit_id = simulation.add_exit_stage([(19, 4), (20, 4), (20, 6), (19, 6)])

# Journey wires the stages in order with fixed transitions.
journey = jps.JourneyDescription([wp1, wp2, exit_id])
journey.set_transition_for_stage(wp1, jps.Transition.create_fixed_transition(wp2))
journey.set_transition_for_stage(wp2, jps.Transition.create_fixed_transition(exit_id))
journey_id = simulation.add_journey(journey)

positions = jps.distributions.distribute_by_number(
    polygon=shapely.Polygon([(0.5, 0.5), (3, 0.5), (3, 9.5), (0.5, 9.5)]),
    number_of_agents=15,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
for position in positions:
    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            journey_id=journey_id,
            stage_id=wp1,
            position=position,
            radius=0.12,
        )
    )

while simulation.agent_count() > 0 and simulation.iteration_count() < 10_000:
    simulation.iterate()

print(f"Done in {simulation.iteration_count()} iterations. Trajectories: {trajectory_file}")
