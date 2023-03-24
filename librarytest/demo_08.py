# Copyright © 2012-2022 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
# Demo 08 - Sources
import logging
import pathlib
from jupedsim.distributions import distribute_by_number
import shapely
from math import sqrt
import random
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
    # logging
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    jps.set_debug_callback(log_debug)
    jps.set_info_callback(log_info)
    jps.set_warning_callback(log_warn)
    jps.set_error_callback(log_error)

    # geometry
    geo_builder = jps.GeometryBuilder()
    geo_builder.add_accessible_area([(0, 0), (9.2, 0), (9.2, 9.2), (0, 9.2)])
    geo_builder.add_accessible_area([(9.2, 2.8), (17.2, 2.8), (17.2, 6.4), (9.2,  6.4)])
    geo_builder.add_accessible_area([(3.2, 9.2), (5.6, 9.2), (5.6, 11.2), (3.2,  11.2)]) 
    geo_builder.add_accessible_area([(3.6, -2), (5.2, -2), (5.2, 0), (3.6,  0)])
    geo_builder.add_accessible_area([(-2, 2.8), (0, 2.8), (0, 6.4), (-2,  6.4)])
    geometry = geo_builder.build()

    # 		      (3.2,  11.2)(5.6, 11.2)
    #		(0, 9.2)(3.2, 9.2)(5.6, 9.2)(9.2, 9.2)				 
    # (-2,  6.4)(0, 6.4)	  	    (9.2,  6.4)		(17.2, 6.4) 
    # (-2, 2.8)	(0, 2.8)		    (9.2, 2.8)		(17.2, 2.8)
    # 		(0, 0)  (3.6,  0) (5.2, 0)  (9.2, 0)
    #		      	(3.6, -2) (5.2, -2)	   

    # destinations
    areas_builder = jps.AreasBuilder()
    # Left exit
    destination = 1
    areas_builder.add_area(
        id=destination,
        polygon=[(-2, 2.8), (-1.8, 2.8), (-1.8, 6.4), (-2, 6.4)],
        labels=["exit"],
    )
    # bottom exit
    destination = 2
    areas_builder.add_area(
        id=destination,
        polygon=[(3.6, -2), (5.2, -2), (5.2, -1.8), (3.6, -1.8)],
        labels=["exit"],
    )
    # Right exit
    destination = 3
    areas_builder.add_area(
        id=destination,
        polygon=[(17, 2.8), (17.2, 2.8), (17.2, 6.4), (17, 6.4)],
        labels=["exit"],
    )
    # upper exit
    destination = 4
    areas_builder.add_area(
        id=destination,
        polygon=[(3.2, 11), (5.6, 11), (5.6, 11.2), (3.2, 11.2)],
        labels=["exit"],
    )
    areas = areas_builder.build()

    model_builder = jps.VelocityModelBuilder(
        a_ped=8, d_ped=0.1, a_wall=5, d_wall=0.02
    )
    
    profile_id = 3
    model_builder.add_parameter_profile(
        id=profile_id, time_gap=1, tau=0.5, v0=1, radius=0.15
    )

    model = model_builder.build()

    simulation = jps.Simulation(
        model=model, geometry=geometry, areas=areas, dt=0.01
    )

    # waypoints
    journeys = []
    journey_ids = []
    journey_ends = []
    
    LEFT = 0
    journey_ends.append((-1.9, 4.6))
    journeys.append(jps.Journey.make_waypoint_journey([((0, 4.6), 1.7), (journey_ends[LEFT], 0.1)]))
    journey_ids.append(simulation.add_journey(journeys[LEFT]))
    
    BOTTOM = 1
    journey_ends.append((4.4, -1.9))
    journeys.append(jps.Journey.make_waypoint_journey([((4.4, 0), 0.7), (journey_ends[BOTTOM], 0.1)]))
    journey_ids.append(simulation.add_journey(journeys[BOTTOM]))
    
    RIGHT = 2
    journey_ends.append((17.1, 4.6))
    journeys.append(jps.Journey.make_waypoint_journey([((9.2, 4.6), 1.7), (journey_ends[RIGHT], 0.1)]))
    journey_ids.append(simulation.add_journey(journeys[RIGHT]))
    
    UP = 3
    journey_ends.append((4.4, 11.1))
    journeys.append(jps.Journey.make_waypoint_journey([((4.4, 9.2), 1.1), (journey_ends[UP], 0.1)]))
    journey_ids.append(simulation.add_journey(journeys[UP]))
    

    agent_parameters = jps.VelocityModelAgentParameters()
    agent_parameters.journey_id = journey_ids[RIGHT]
    agent_parameters.orientation = (1.0, 0.0)
    agent_parameters.position = (0.0, 0.0)
    agent_parameters.profile_id = profile_id
    
    origin_seed = 12542
    random.seed(origin_seed)
    polygons = []
    polygons.append(shapely.Polygon([(3.5, 3.5), (5.5, 3.5), (5.5, 5.5), (3.5, 5.5)]))
    polygons.append(shapely.Polygon([(0, 0), (3, 0), (3, 3), (0, 3)]))
    polygons.append(shapely.Polygon([(6, 6), (9, 6), (9, 9), (6, 9)]))
    polygons.append(shapely.Polygon([(6, 0), (9, 0), (9, 3), (6, 3)]))
    polygons.append(shapely.Polygon([(0, 6), (3, 6), (3, 9), (0, 9)]))
    
    agents = []
    for polygon in polygons:
        seed = random.randint(10000, 99999)
        agents.extend(
            distribute_by_number(polygon=polygon, number_of_agents=5,
		                         distance_to_agents=0.30,
		                         distance_to_polygon=0.20, seed=seed)
        )
    
    for x, y in agents:
        agent_parameters.position = (x, y)
        
        # select journey with nearest exit
        distances_to_ends = []
        for end in journey_ends:
            distances_to_ends.append(
                sqrt((end[0]-x)**2 + (end[1]-y)**2)
            )
        min_distance = min(distances_to_ends)
        index = distances_to_ends.index(min_distance)
        agent_parameters.journey_id = journey_ids[index]
        
        simulation.add_agent(agent_parameters)

    print("Running simulation")

    writer = JpsCoreStyleTrajectoryWriter(pathlib.Path("08.txt"))
    fps = 10;
    writer.begin_writing(fps)
    
    agents_added = False
    time_to_deploy = 2.0
    iteration_to_deploy = time_to_deploy * fps * fps;
    while simulation.agent_count() > 0 or not agents_added:
        if simulation.iteration_count() == iteration_to_deploy:
            agents = []
            for polygon in polygons:
                seed = random.randint(10000, 99999)
                agents.extend(
                    distribute_by_number(polygon=polygon, number_of_agents=5,
	                                     distance_to_agents=0.5,
	                                     distance_to_polygon=0.20, seed=seed)
                )
            
            for x, y in agents:
                agent_parameters.position = (x, y)
                
                # select journey with nearest exit
                distances_to_ends = []
                for end in journey_ends:
                    distances_to_ends.append(
                        sqrt((end[0]-x)**2 + (end[1]-y)**2)
                    )
                min_distance = min(distances_to_ends)
                index = distances_to_ends.index(min_distance)
                agent_parameters.journey_id = journey_ids[index]
                
                simulation.add_agent(agent_parameters)
                agents_added = True
                        
        simulation.iterate()
        if simulation.iteration_count() % fps == 0:	
            writer.write_iteration_state(simulation)
    writer.end_writing()
    print(
        f"Simulation completed after {simulation.iteration_count()} iterations"
    )


if __name__ == "__main__":
    main()

