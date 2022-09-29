# agent_radius -> agent_distance
# wall_distance -> distance_to_polygon
# fill_circles as two methods destribute_in_circles_by_number/distribute_in_circles_by_density

def distribute_by_number(polygon, number_of_agents, agent_distance, distance_to_polygon, seed=None, max_iterations=10000):
    """returns number_of_agents points randomly placed inside the polygon

        :param polygon: shapely polygon in which the agents will be placed
        :param number_of_agents: number of agents distributed
        :param agent_distance: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
        :return: list of created points"""
    pass


def distribute_by_density(polygon, density, agent_distance, distance_to_polygon, seed=None, max_iterations=10000):
    """returns points randomly placed inside the polygon with the given density

        :param polygon: shapely polygon in which the agents will be placed
        :param density: Density of agents inside the polygon 
        :param agent_distance: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
        :return: list of created points"""
    pass


def distribute_in_circles_by_number(polygon, agent_distance, distance_to_polygon,
                                    center_point, circle_segment_radii, numbers_of_agents,
                                    seed=None, max_iterations=10_000):
    """returns points randomly placed inside the polygon inside each the circle segments

        :param polygon: shapely polygon in which the agents will be placed
        :param agent_distance: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param center_point: the Center point of the circle segments
        :param circle_segment_radii: a list of minimal and maximal radius for each circle segment
            Circle segments must not overlap
            formatted like [(minimum_radius, maximum_radius)]
        :param numbers_of_agents: a list of number of agents for each Circle segment
            the position of the number corresponds to the order in which the Circle segments are given
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
        :return: list of created points"""
    pass


def distribute_in_circles_by_density(polygon, agent_radius, distance_to_polygon,
                                     center_point, circle_segment_radii, densities,
                                     seed=None, max_iterations=10_000):
    """returns points randomly placed inside the polygon inside each the circle segments

        :param polygon: shapely polygon in which the agents will be placed
        :param agent_radius: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param center_point: the Center point of the circle segments
        :param circle_segment_radii: a list of minimal and maximal radius for each circle segment
            Circle segments must not overlap
            formatted like [(minimum_radius, maximum_radius)]
        :param densities: a list of densities for each Circle segment
            the position of the number corresponds to the order in which the Circle segments are given
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
        :return: list of created points"""
    pass

# not included in this Pull request


def distribute_by_percentage(polygon, percent, agent_distance, distance_to_polygon, seed=None, max_iterations=10000):
    """returns a percentage of agents that can fit inside the polygon
       fills the polygon entirely and then selects the percentage of placed agents

        :param polygon: shapely polygon in which the agents will be placed
        :param percent: percentage of agents selected - 100% ≙ completely filled polygon 0% ≙ 0 placed points
        :param agent_distance: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
        :return: list of created points"""
    pass


def distribute_til_full(polygon, agent_distance, distance_to_polygon, seed=None, max_iterations=10000):
    """returns as many randomly placed points as fit into the polygon
        :param polygon: shapely polygon in which the agents will be placed
        :param agent_distance: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
        :return: list of created points"""
    pass
