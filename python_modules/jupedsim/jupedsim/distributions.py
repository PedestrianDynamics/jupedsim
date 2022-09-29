from jupedsim.Grid import Grid
import numpy as np
import shapely.geometry as shply


class AgentCount(Exception):
    def __init__(self, message):
        self.message = message


class Overlapping(Exception):
    def __init__(self, message):
        self.message = message


class NegativeNumber(Exception):
    def __init__(self, message):
        self.message = message


class Distribution:
    def __init__(self, mid):
        self.circles = []
        self.mid_point = mid

    def create_circle(self, min_radius, max_radius, number=None, density=None):
        """creates a circle segment around the mid point of the object
            - reaching from min_radius to max_radius
            - either state a number of agents or a density for this circle segment
            - if a set number is given density will be ignored
            - if another circle around this object intersects with the new circle the method will raise an Exception
        """
        if number is None and density is None:
            raise AgentCount(f"no number of agents and no density given when"
                             f" creating a Circle from {min_radius} to {max_radius} with center at {self.mid_point}")
        if min_radius < 0 or max_radius < 0:
            raise NegativeNumber(f"It is not possible to create a Circle with a negativ radius:"
                                 f" creating a Circle from {min_radius} to {max_radius} was not possible")
        if min_radius > max_radius:
            raise Overlapping(f"minimum radius bigger than maximum radius"
                              f" creating a Circle from {min_radius} to {max_radius} was not possible")

        for circle in self.circles:
            if min_radius < max_radius <= circle[0] or circle[1] <= min_radius < max_radius:
                continue
            else:
                raise Overlapping(f"the new Circle from {min_radius} to {max_radius} would overlap with"
                                  f" the existing circle from {circle[0]} to {circle[1]}")
        self.circles.append((min_radius, max_radius, number, density))

    def remove_circle(self, min_radius, max_radius):
        """removes a circle segment from min_radius to max_radius
            if no segment is found with the exact radius nothing will happen"""
        for circle in self.circles:
            if circle[0] == min_radius and circle[1] == max_radius:
                self.circles.remove(circle)
                break

    def place_in_Polygon(self, polygon, agent_radius, wall_distance, seed=None, max_iterations=10_000, obstacles=None):
        """returns points inside each circle segment
            points have an agent_radius within which no other point may be placed.
            points will not be placed with less than wall_distance to the polygon
            points will be placed inside the polygon and inside the circle segment
            points are placed first in the segment that was created first
            if more that 10000 tries are needed to placed a valid point an Exception will be thrown
            obstacles are holes inside the polygon"""
        if seed is not None:
            np.random.seed(seed)
        box = get_bounding_box(polygon)
        if obstacles is None:
            holes = []
        else:
            holes = obstacles
        s_polygon = shply.Polygon(polygon, holes)
        grid = Grid(box, agent_radius)
        for circle in self.circles:
            # if for the circle no exact number of agents is set it will be determined with the density
            big_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[1], s_polygon)
            small_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[0], s_polygon)
            placeable_area = big_circle_area - small_circle_area
            if circle[2] is None:
                density = circle[3]
                targeted_count = round(density * placeable_area)
            else:
                targeted_count = circle[2]

            # it is being checked whether to place points inside the circle segment or around the polygon
            # determine the entire area of the circle segment
            entire_circle_area = np.pi * (circle[1] ** 2 - circle[0] ** 2)
            # determine the area where a point might be placed around the polygon
            dif_x, dif_y = box[1][0] - box[0][0], box[1][1] - box[0][1]
            entire_polygon_area = dif_x * dif_y

            if entire_circle_area < entire_polygon_area:
                # inside the circle it is more likely to find a random point that is inside the polygon
                for placed_count in range(targeted_count):
                    # inside the circle it is more likely to find a random point that is inside the polygon
                    i = 0
                    while i < max_iterations:
                        i += 1
                        # determines a random radius within the circle segment
                        rho = np.sqrt(np.random.uniform(circle[0] ** 2, circle[1] ** 2))
                        # determines a random degree
                        theta = np.random.uniform(0, 2 * np.pi)
                        pt = self.mid_point[0] + rho * np.cos(theta), self.mid_point[1] + rho * np.sin(theta)
                        if check_distance_constraints(pt, wall_distance, grid, s_polygon):
                            grid.append_point(pt)
                            break

                    if i >= max_iterations and placed_count != targeted_count:
                        message = f"the desired amount of agents in the Circle from {circle[0]} to {circle[1]} " \
                                  f"could not be achieved.\nOnly {placed_count} of {targeted_count}  could be placed."
                        if circle[2] is None:
                            # if the circle has no number of agents
                            # the expected and actual density will be added to the exception message
                            message += f"\nexpected density: {circle[3]} p/m², " \
                                       f"actual density: {round(placed_count / placeable_area, 2)} p/m²"
                        raise AgentCount(message)
            else:
                # placing points around the polygon is more likely to find a random point that is inside the circle
                placed_count = 0
                iterations = 0
                while placed_count < targeted_count:
                    if iterations > max_iterations:
                        message = f"the desired amount of agents in the Circle from {circle[0]} to {circle[1]} " \
                                  f"could not be achieved.\nOnly {placed_count} of {targeted_count}  could be placed."
                        if circle[2] is None:
                            # if the circle has no number of agents
                            # the expected and actual density will be added to the exception message
                            message += f"\nexpected density: {circle[3]} p/m², " \
                                       f"actual density: {round(placed_count / placeable_area, 2)} p/m²"
                        raise AgentCount(message)
                    temp_point = (np.random.uniform(box[0][0], box[1][0]), np.random.uniform(box[0][1], box[1][1]))
                    if is_inside_circle(temp_point, self.mid_point, circle[0], circle[1]) \
                            and check_distance_constraints(temp_point, wall_distance, grid, s_polygon):
                        grid.append_point(temp_point)
                        iterations = 0
                        placed_count += 1
                    else:
                        iterations += 1
        return grid.get_samples()


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
    area = polygon.area
    number = round(density * area)
    return distribute_by_number(polygon, number, agent_distance, distance_to_polygon, seed, max_iterations)


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

    # todo: needs errors implementation
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
                # inside the circle it is more likely to find a random point that is inside the polygon
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

    number_of_agents = []
    for circle_segment, density in zip(circle_segment_radii, densities):
        big_circle_area = intersecting_area_polygon_circle(center_point, circle_segment[1], polygon)
        small_circle_area = intersecting_area_polygon_circle(center_point, circle_segment[0], polygon)
        placeable_area = big_circle_area - small_circle_area
        number_of_agents.append(density * placeable_area)

    return distribute_in_circles_by_number(polygon=polygon, agent_distance=agent_radius,
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
