"""Multi-stage routing with switch_agent_journey.

Agents start toward a near exit, then half are re-routed at runtime to a
second exit to balance load.
"""

import pathlib

import jupedsim as jps
import shapely

geometry = shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)])

trajectory_file = pathlib.Path("routing.sqlite")
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=geometry,
    trajectory_writer=jps.SqliteTrajectoryWriter(
        output_file=trajectory_file, commit_every_nth_write=1
    ),
)

exit_top = simulation.add_exit_stage([(19, 7), (20, 7), (20, 9), (19, 9)])
exit_bottom = simulation.add_exit_stage([(19, 1), (20, 1), (20, 3), (19, 3)])

journey_top = simulation.add_journey(jps.JourneyDescription([exit_top]))
journey_bottom = simulation.add_journey(jps.JourneyDescription([exit_bottom]))

positions = jps.distributions.distribute_by_number(
    polygon=shapely.Polygon([(0.5, 0.5), (4, 0.5), (4, 9.5), (0.5, 9.5)]),
    number_of_agents=20,
    distance_to_agents=0.4,
    distance_to_polygon=0.2,
    seed=1,
)
agent_ids = [
    simulation.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            journey_id=journey_top,
            stage_id=exit_top,
            position=position,
            radius=0.12,
        )
    )
    for position in positions
]

# Run, re-routing the second half to the bottom exit mid-simulation so the
# switch happens while agents are already moving (dynamic load balancing).
rerouted = False
while simulation.agent_count() > 0 and simulation.iteration_count() < 10_000:
    if not rerouted and simulation.iteration_count() == 200:
        for agent_id in agent_ids[len(agent_ids) // 2 :]:
            simulation.switch_agent_journey(
                agent_id, journey_bottom, exit_bottom
            )
        rerouted = True
    simulation.iterate()

print(
    f"Done in {simulation.iteration_count()} iterations. Trajectories: {trajectory_file}"
)
