"""Notifiable queue + waiting set.

Agents first line up in a queue that releases them in batches. They then
gather in a waiting set that holds them in place until it is deactivated,
after which they continue to the exit.
"""

import pathlib

import jupedsim as jps
import shapely

geometry = shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)])

trajectory_file = pathlib.Path("queues_waiting.sqlite")
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geometry,
    trajectory_writer=jps.SqliteTrajectoryWriter(
        output_file=trajectory_file, commit_every_nth_write=1
    ),
)

# Queue: agents line up here and are released in batches.
queue_id = simulation.add_queue_stage([(7, 5), (7.4, 5), (7.8, 5)])
queue = simulation.get_stage(queue_id)

# Waiting set: when active it holds agents; when inactive it behaves as a
# waypoint so agents pass straight through.
waiting_id = simulation.add_waiting_set_stage([(13, 5), (13, 5.4), (13, 4.6)])
waiting = simulation.get_stage(waiting_id)
waiting.state = jps.WaitingSetState.ACTIVE

exit_id = simulation.add_exit_stage([(19, 4), (20, 4), (20, 6), (19, 6)])

journey = jps.JourneyDescription([queue_id, waiting_id, exit_id])
journey.set_transition_for_stage(
    queue_id, jps.Transition.create_fixed_transition(waiting_id)
)
journey.set_transition_for_stage(
    waiting_id, jps.Transition.create_fixed_transition(exit_id)
)
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

# Release two agents from the queue every 100 iterations. Once the queue has
# filled and then fully drained, open the waiting set so the gathered agents
# can proceed to the exit.
queue_was_filled = False
while simulation.agent_count() > 0 and simulation.iteration_count() < 10_000:
    if simulation.iteration_count() % 100 == 0 and queue.count_enqueued() > 0:
        queue.pop(2)
    if queue.count_enqueued() > 0:
        queue_was_filled = True
    if (
        queue_was_filled
        and queue.count_enqueued() == 0
        and waiting.state == jps.WaitingSetState.ACTIVE
    ):
        waiting.state = jps.WaitingSetState.INACTIVE
    simulation.iterate()

print(
    f"Done in {simulation.iteration_count()} iterations. Trajectories: {trajectory_file}"
)
