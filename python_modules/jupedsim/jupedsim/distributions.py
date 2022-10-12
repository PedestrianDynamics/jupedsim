from jupedsim.grid import Grid
import numpy as np
import shapely.geometry as shply


class AgentCount(Exception):
    def __init__(self, message):
        self.message = message


class IncorrectParameterError(Exception):
    def __init__(self, message):
        self.message = message


class OverlappingCirclesError(Exception):
    def __init__(self, message):
        self.message = message


class NegativeValueError(Exception):
    def __init__(self, message):
        self.message = message


def intersecting_area_polygon_circle(mid_point, radius, polygon):
    """returns the intersecting area of circle and polygon"""
    # creates a point
    point = shply.Point(mid_point)
    # creates a layer with the size of the radius all around this point
    circle = point.buffer(radius)
    # returns the size of the intersecting area
    return polygon.intersection(circle).area


def is_inside_circle(point, mid, min_r, max_r):
    """checks if a point is inside a Circle segment reaching from minimum radius to maximum radius"""
    dif_x = point[0] - mid[0]
    dif_y = point[1] - mid[1]
    circle_equation = dif_x ** 2 + dif_y ** 2
    return min_r ** 2 <= circle_equation <= max_r ** 2


def get_bounding_box(polygon):
    """returns an Axis Aligned Bounding Box containing the minimal/maximal x and y values
    formatted like : [(min(x_values), min(y_values)), (max(x_values), max(y_values))]
    polygon needs to be a shapely polygon
    """
    corner_points = list(polygon.exterior.coords)
    x_values, y_values = [], []
    for point in corner_points:
        x_values.append(point[0])
        y_values.append(point[1])

    return [(min(x_values), min(y_values)), ((max(x_values)), max(y_values))]


def min_distance_to_polygon(pt, polygon):
    """returns the minimal distance between a point and every line segment of a polygon"""
    pt = shply.Point(pt)
    min_dist = polygon.exterior.distance(pt)
    for hole in polygon.interiors:
        candidate_dist = hole.distance(pt)
        min_dist = min(min_dist, candidate_dist)
    return min_dist


def distribute_by_number(polygon, number_of_agents, agent_distance, distance_to_polygon,
                         seed=None, max_iterations=10000):
    """"returns number_of_agents points randomly placed inside the polygon

        :param polygon: shapely polygon in which the agents will be placed
        :param number_of_agents: number of agents distributed
        :param agent_distance: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
        :return: list of created points"""

    if not isinstance(polygon, shply.polygon.Polygon):
        raise IncorrectParameterError(f"Polygon is expected to be a shapely Polygon")
    box = get_bounding_box(polygon)

    if seed is not None:
        np.random.seed(seed)

    grid = Grid(box, agent_distance)
    created_points = 0
    iterations = 0
    while created_points < number_of_agents:
        if iterations > max_iterations:
            msg = f"Only {created_points} of {number_of_agents}  could be placed." \
                  f" density: {round(created_points / polygon.area, 2)} p/m²"
            raise AgentCount(msg)
        temp_point = (np.random.uniform(box[0][0], box[1][0]), np.random.uniform(box[0][1], box[1][1]))
        if check_distance_constraints(temp_point, distance_to_polygon, grid, polygon):
            grid.append_point(temp_point)

            iterations = 0
            created_points += 1
        else:
            iterations += 1

    return grid.get_samples()


def distribute_by_density(polygon, density, agent_distance, distance_to_polygon, seed=None, max_iterations=10000):
    """returns points randomly placed inside the polygon with the given density

        :param polygon: shapely polygon in which the agents will be placed
        :param density: Density of agents inside the polygon
        :param agent_distance: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
        :return: list of created points"""

    if not isinstance(polygon, shply.polygon.Polygon):
        raise IncorrectParameterError(f"Polygon is expected to be a shapely Polygon")
    area = polygon.area
    number = round(density * area)
    return distribute_by_number(polygon, number, agent_distance, distance_to_polygon, seed, max_iterations)


def catch_wrong_inputs(polygon, center_point, circle_segment_radii, fill_parameters):
    """checks if an input parameter is incorrect and raises an Exception"""
    if not isinstance(polygon, shply.polygon.Polygon):
        raise IncorrectParameterError(f"Polygon is expected to be a shapely Polygon")
    try:
        if len(center_point) != 2:
            raise IncorrectParameterError(f"Center_point expected a tuple of 2 numbers, {len(center_point)} were given")
    except TypeError:
        # center point is no tuple or list
        raise IncorrectParameterError(f"Center_point expected a tuple of 2 numbers")
    if len(circle_segment_radii) != len(fill_parameters):
        raise IncorrectParameterError(f"the number of circle segments does not match the number of fill parameters.\n"
                                      f"radii given for {len(circle_segment_radii)} circle segments,"
                                      f"fill parameter given for {len(fill_parameters)} circle segments")
    for i, c_s_radius in enumerate(circle_segment_radii):
        if c_s_radius[0] < 0 or c_s_radius[1] < 0:
            raise NegativeValueError(f"Circle segment {c_s_radius[0]} : {c_s_radius[1]} is expected to be positiv")
        if c_s_radius[0] >= c_s_radius[1]:
            raise OverlappingCirclesError(f"minimum radius bigger than maximum radius\n"
                                          f"a Circle segment from {c_s_radius[0]} to {c_s_radius[1]} is not possible")
        j = 0
        while j < i:
            if c_s_radius[0] < c_s_radius[1] <= circle_segment_radii[j][0] \
                    or circle_segment_radii[j][1] <= c_s_radius[0] < c_s_radius[1]:
                j = j+1
                continue
            else:
                raise OverlappingCirclesError(f"the new Circle would overlap with"
                                              f"the existing circle from {c_s_radius[0]} to {c_s_radius[1]}")


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

    # catch wrong inputs
    catch_wrong_inputs(polygon=polygon, center_point=center_point,
                       circle_segment_radii=circle_segment_radii, fill_parameters=numbers_of_agents)
    if seed is not None:
        np.random.seed(seed)
    box = get_bounding_box(polygon)
    grid = Grid(box, agent_distance)
    for circle_segment, number in zip(circle_segment_radii, numbers_of_agents):
        big_circle_area = intersecting_area_polygon_circle(center_point, circle_segment[1], polygon)
        small_circle_area = intersecting_area_polygon_circle(center_point, circle_segment[0], polygon)
        placeable_area = big_circle_area - small_circle_area

        # it is being checked whether to place points inside the circle segment or around the polygon
        # determine the entire area of the circle segment
        entire_circle_area = np.pi * (circle_segment[1] ** 2 - circle_segment[0] ** 2)
        # determine the area where a point might be placed around the polygon
        dif_x, dif_y = box[1][0] - box[0][0], box[1][1] - box[0][1]
        entire_polygon_area = dif_x * dif_y

        if entire_circle_area < entire_polygon_area:
            # inside the circle it is more likely to find a random point that is inside the polygon
            for placed_count in range(number):
                i = 0
                while i < max_iterations:
                    i += 1
                    # determines a random radius within the circle segment
                    rho = np.sqrt(np.random.uniform(circle_segment[0] ** 2, circle_segment[1] ** 2))
                    # determines a random degree
                    theta = np.random.uniform(0, 2 * np.pi)
                    pt = center_point[0] + rho * np.cos(theta), center_point[1] + rho * np.sin(theta)
                    if check_distance_constraints(pt, distance_to_polygon, grid, polygon):
                        grid.append_point(pt)
                        break

                if i >= max_iterations and placed_count != number:
                    message = f"the desired amount of agents in the Circle segment from" \
                              f" {circle_segment[0]} to {circle_segment[1]} could not be achieved." \
                              f"\nOnly {placed_count} of {number}  could be placed." \
                              f"\nactual density: {round(placed_count / placeable_area, 2)} p/m²"
                    raise AgentCount(message)
        else:
            # placing points around the polygon is more likely to find a random point that is inside the circle
            placed_count = 0
            iterations = 0
            while placed_count < number:
                if iterations > max_iterations:
                    message = f"the desired amount of agents in the Circle segment from" \
                              f" {circle_segment[0]} to {circle_segment[1]} could not be achieved." \
                              f"\nOnly {placed_count} of {number}  could be placed." \
                              f"\nactual density: {round(placed_count / placeable_area, 2)} p/m²"
                    raise AgentCount(message)
                temp_point = (np.random.uniform(box[0][0], box[1][0]), np.random.uniform(box[0][1], box[1][1]))
                if is_inside_circle(temp_point, center_point, circle_segment[0], circle_segment[1]) \
                        and check_distance_constraints(temp_point, distance_to_polygon, grid, polygon):
                    grid.append_point(temp_point)
                    iterations = 0
                    placed_count += 1
                else:
                    iterations += 1
    return grid.get_samples()


def distribute_in_circles_by_density(polygon, agent_distance, distance_to_polygon,
                                     center_point, circle_segment_radii, densities,
                                     seed=None, max_iterations=10_000):
    """returns points randomly placed inside the polygon inside each the circle segments

        :param polygon: shapely polygon in which the agents will be placed
        :param agent_distance: minimal distance between the centers of agents
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

    catch_wrong_inputs(polygon=polygon, center_point=center_point,
                       circle_segment_radii=circle_segment_radii, fill_parameters=densities)
    number_of_agents = []
    for circle_segment, density in zip(circle_segment_radii, densities):
        big_circle_area = intersecting_area_polygon_circle(center_point, circle_segment[1], polygon)
        small_circle_area = intersecting_area_polygon_circle(center_point, circle_segment[0], polygon)
        placeable_area = big_circle_area - small_circle_area
        number_of_agents.append(int(density * placeable_area))

    return distribute_in_circles_by_number(polygon=polygon, agent_distance=agent_distance,
                                           distance_to_polygon=distance_to_polygon, center_point=center_point,
                                           circle_segment_radii=circle_segment_radii,
                                           numbers_of_agents=number_of_agents, seed=seed, max_iterations=max_iterations)


def check_distance_constraints(pt, wall_distance, grid, polygon):
    """Determines if a point has enough distance to other points and to the walls
     Uses a Grid to determine neighbours
    :param grid: the grid of the polygon
    :param pt: point that is being checked
    :param wall_distance: minimal distance between point and the polygon
    :param polygon: shapely Polygon in which the points must lie
    :return:True or False"""
    if not polygon.contains(shply.Point(pt)):
        return False
    if min_distance_to_polygon(pt, polygon) < wall_distance:
        return False
    return grid.no_neighbours_in_distance(pt)
