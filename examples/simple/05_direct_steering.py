"""Direct steering: set an agent's target each iteration manually.

A direct-steering stage lets you drive an agent by assigning `agent.target`
every step instead of using stages/journeys for navigation.
"""

import pathlib

import jupedsim as jps
import shapely

geometry = shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)])

trajectory_file = pathlib.Path("direct_steering.sqlite")
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geometry,
    trajectory_writer=jps.SqliteTrajectoryWriter(
        output_file=trajectory_file, commit_every_nth_write=1
    ),
)

steering_stage = simulation.add_direct_steering_stage()
journey_id = simulation.add_journey(jps.JourneyDescription([steering_stage]))

agent_id = simulation.add_agent(
    jps.CollisionFreeSpeedModelAgentParameters(
        journey_id=journey_id,
        stage_id=steering_stage,
        position=(2, 5),
        radius=0.12,
    )
)

# Drive the agent to a sequence of waypoints by setting `target` each step.
waypoints = [(10, 5), (10, 9), (18, 9)]
waypoint_index = 0
done = False
while (
    simulation.agent_count() > 0
    and simulation.iteration_count() < 10_000
    and not done
):
    agent = simulation.agent(agent_id)
    agent.target = waypoints[waypoint_index]
    if (
        shapely.Point(agent.position).distance(
            shapely.Point(waypoints[waypoint_index])
        )
        < 0.5
    ):
        waypoint_index += 1
        if waypoint_index >= len(waypoints):
            simulation.mark_agent_for_removal(agent_id)
            done = True
    simulation.iterate()

print(
    f"Done in {simulation.iteration_count()} iterations. Trajectories: {trajectory_file}"
)
