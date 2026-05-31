"""Notifiable queue + waiting set.

A queue stage releases agents in batches once it has filled; a separate
waiting set holds agents until it is deactivated.
"""

import pathlib

import jupedsim as jps
import shapely

geometry = shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)])

trajectory_file = pathlib.Path("queues_waiting.sqlite")
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geometry,
    trajectory_writer=jps.SqliteTrajectoryWriter(output_file=trajectory_file),
)

# Queue: agents line up at these positions before release.
queue_id = simulation.add_queue_stage([(10, 5), (10.4, 5), (10.8, 5)])
queue = simulation.get_stage(queue_id)
exit_id = simulation.add_exit_stage([(19, 4), (20, 4), (20, 6), (19, 6)])

journey = jps.JourneyDescription([queue_id, exit_id])
journey.set_transition_for_stage(queue_id, jps.Transition.create_fixed_transition(exit_id))
journey_id = simulation.add_journey(journey)

positions = jps.distributions.distribute_by_number(
    polygon=shapely.Polygon([(0.5, 0.5), (4, 0.5), (4, 9.5), (0.5, 9.5)]),
    number_of_agents=12,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
for position in positions:
    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            journey_id=journey_id,
            stage_id=queue_id,
            position=position,
            radius=0.12,
        )
    )

# Release two agents from the queue every 100 iterations.
while simulation.agent_count() > 0 and simulation.iteration_count() < 10_000:
    if simulation.iteration_count() % 100 == 0 and queue.count_enqueued() > 0:
        queue.pop(2)
    simulation.iterate()

print(f"Done in {simulation.iteration_count()} iterations. Trajectories: {trajectory_file}")
