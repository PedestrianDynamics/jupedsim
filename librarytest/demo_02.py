# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib

import py_jupedsim as jps
from jupedsim.serialization import JpsCoreStyleTrajectoryWriter


def log_debug(msg):
    logging.debug(msg)


def log_info(msg):
    logging.info(msg)


def log_warn(msg):
    logging.warning(msg)


def log_error(msg):
    logging.error(msg)


def main():
	#logging
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    jps.set_debug_callback(log_debug)
    jps.set_info_callback(log_info)
    jps.set_warning_callback(log_warn)
    jps.set_error_callback(log_error)

	# geometry
    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([(50, 100), (60, 100), (60, 104), (50, 104)])
    geo_builder.add_accessible_area([(60, 101.4), (62, 101.4), (62, 102.6), (60, 102.6)])
    geo_builder.add_accessible_area([(62, 100), (65, 100), (65, 104), (62, 104)])
    geometry = geo_builder.build()
	
	
	# (50, 104)	(60, 104)	(62, 104)    (65, 104)
	# 		(60, 102.6)	(62, 102.6)    
	#
	#		(60, 101.4)	(62, 101.4)
	# (50, 100)	(60, 100)	(62, 100)     (65, 100)
	
	# destination
    destination = 1
    areas_builder = jps.AreasBuilder()
    areas_builder.add_area(
        id=destination,
        polygon=[(64.5, 102.6), (64.5, 101.4), (65, 101.4), (65, 102.6)],
        labels=["exit"],
    )
    areas = areas_builder.build()

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    profile_id = 3
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1.0, radius=0.3
    )

    model = model_builder.build()

    simulation = jps.Simulation(
        model=model, geometry=geometry, areas=areas, dt=0.01
    )
    
	# waypoints?
    journey = jps.Journey.make_waypoint_journey([((61, 102), 1)])

    journey_id = simulation.add_journey(journey)

    agent_parameters = jps.AgentParameters()
    agent_parameters.journey_id = journey_id
    agent_parameters.orientation_x = 1.0
    agent_parameters.orientation_y = 0.0
    agent_parameters.x = 0.0
    agent_parameters.y = 0.0
    agent_parameters.profile_id = profile_id
	
	# 30 agents needed
    for x, y in [(52, 102)]:
    	# (54, 101), (51, 103), (52, 103), (53.5, 102)
        agent_parameters.x = x
        agent_parameters.y = y
        simulation.add_agent(agent_parameters)

    print("Running simulation")

    writer = JpsCoreStyleTrajectoryWriter(pathlib.Path("02.txt"))
    writer.begin_writing(10)

    while simulation.agent_count() > 0:
        simulation.iterate()
        if simulation.iteration_count() % 10 == 0:
            writer.write_iteration_state(simulation)
    writer.end_writing()
    print(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )


if __name__ == "__main__":
    main()
