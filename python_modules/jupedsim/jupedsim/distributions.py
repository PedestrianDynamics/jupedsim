import numpy as np
import shapely.geometry as shply

from jupedsim.grid import Grid


class AgentNumberError(Exception):
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


def __intersecting_area_polygon_circle(mid_point, radius, polygon):
    """returns the intersecting area of circle and polygon"""
    # creates a point
    point = shply.Point(mid_point)
    # creates a layer with the size of the radius all around this point
    circle = point.buffer(radius)
    # returns the size of the intersecting area
    return polygon.intersection(circle).area


def __is_inside_circle(point, mid, min_r, max_r):
    """checks if a point is inside a Circle segment reaching from minimum radius to maximum radius"""
    dif_x = point[0] - mid[0]
    dif_y = point[1] - mid[1]
    circle_equation = dif_x**2 + dif_y**2
    return min_r**2 <= circle_equation <= max_r**2


def __get_bounding_box(polygon):
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


def __min_distance_to_polygon(pt, polygon):
    """returns the minimal distance between a point and every line segment of a polygon"""
    pt = shply.Point(pt)
    min_dist = polygon.exterior.distance(pt)
    for hole in polygon.interiors:
        candidate_dist = hole.distance(pt)
        min_dist = min(min_dist, candidate_dist)
    return min_dist


def distribute_by_number(
    *,
    polygon,
    number_of_agents,
    distance_to_agents,
    distance_to_polygon,
    seed=None,
    max_iterations=10000,
):
    """ "returns number_of_agents points randomly placed inside the polygon

    :param polygon: shapely polygon in which the agents will be placed
    :param number_of_agents: number of agents distributed
    :param distance_to_agents: minimal distance between the centers of agents
    :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
    :param seed: define a seed for random generation, Default value is None which corresponds to a random value
    :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
    :return: list of created points"""

    if not isinstance(polygon, shply.polygon.Polygon):
        raise IncorrectParameterError(
            f"Polygon is expected to be a shapely Polygon"
        )
    box = __get_bounding_box(polygon)

    np.random.seed(seed)

    grid = Grid(box, distance_to_agents)
    created_points = 0
    iterations = 0
    while created_points < number_of_agents:
        if iterations > max_iterations:
            msg = (
                f"Only {created_points} of {number_of_agents}  could be placed."
                f" density: {round(created_points / polygon.area, 2)} p/m²"
            )
            raise AgentNumberError(msg)
        temp_point = (
            np.random.uniform(box[0][0], box[1][0]),
            np.random.uniform(box[0][1], box[1][1]),
        )
        if __check_distance_constraints(
            temp_point, distance_to_polygon, grid, polygon
        ):
            grid.append_point(temp_point)

            iterations = 0
            created_points += 1
        else:
            iterations += 1

    return grid.get_samples()


def distribute_by_density(
    *,
    polygon,
    density,
    distance_to_agents,
    distance_to_polygon,
    seed=None,
    max_iterations=10000,
):
    """returns points randomly placed inside the polygon with the given density

    :param polygon: shapely polygon in which the agents will be placed
    :param density: Density of agents inside the polygon
    :param distance_to_agents: minimal distance between the centers of agents
    :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
    :param seed: define a seed for random generation, Default value is None which corresponds to a random value
    :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
    :return: list of created points"""

    if not isinstance(polygon, shply.polygon.Polygon):
        raise IncorrectParameterError(
            f"Polygon is expected to be a shapely Polygon"
        )
    area = polygon.area
    number = round(density * area)
    return distribute_by_number(
        polygon=polygon,
        number_of_agents=number,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=seed,
        max_iterations=max_iterations,
    )


def __catch_wrong_inputs(
    polygon, center_point, circle_segment_radii, fill_parameters
):
    """checks if an input parameter is incorrect and raises an Exception"""
    if not isinstance(polygon, shply.polygon.Polygon):
        raise IncorrectParameterError(
            f"Polygon is expected to be a shapely Polygon"
        )
    try:
        if len(center_point) != 2:
            raise IncorrectParameterError(
                f"Center_point expected a tuple of 2 numbers, {len(center_point)} were given"
            )
    except TypeError:
        # center point is no tuple or list
        raise IncorrectParameterError(
            f"Center_point expected a tuple of 2 numbers, given Type: {type(center_point)}"
        )
    if len(circle_segment_radii) != len(fill_parameters):
        raise IncorrectParameterError(
            f"the number of circle segments does not match the number of fill parameters.\n"
            f"radii given for {len(circle_segment_radii)} circle segments,"
            f"fill parameter given for {len(fill_parameters)} circle segments"
        )
    for i, c_s_radius in enumerate(circle_segment_radii):
        if c_s_radius[0] < 0 or c_s_radius[1] < 0:
            raise NegativeValueError(
                f"Circle segment {c_s_radius[0]} : {c_s_radius[1]} is expected to be positiv"
            )
        if c_s_radius[0] >= c_s_radius[1]:
            raise OverlappingCirclesError(
                f"inner radius bigger than/equal to outer radius\n"
                f"a Circle segment from {c_s_radius[0]} to {c_s_radius[1]} is not possible"
            )
        j = 0
        while j < i:
            if (
                c_s_radius[0] < c_s_radius[1] <= circle_segment_radii[j][0]
                or circle_segment_radii[j][1] <= c_s_radius[0] < c_s_radius[1]
            ):
                j = j + 1
                continue
            else:
                raise OverlappingCirclesError(
                    f"the Circle from {c_s_radius[0]} to {c_s_radius[1]} overlaps with others"
                )


def distribute_in_circles_by_number(
    *,
    polygon,
    distance_to_agents,
    distance_to_polygon,
    center_point,
    circle_segment_radii,
    numbers_of_agents,
    seed=None,
    max_iterations=10_000,
):
    """returns points randomly placed inside the polygon inside each the circle segments

    :param polygon: shapely polygon in which the agents will be placed
    :param distance_to_agents: minimal distance between the centers of agents
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
    __catch_wrong_inputs(
        polygon=polygon,
        center_point=center_point,
        circle_segment_radii=circle_segment_radii,
        fill_parameters=numbers_of_agents,
    )
    np.random.seed(seed)
    box = __get_bounding_box(polygon)
    grid = Grid(box, distance_to_agents)

    for circle_segment, number in zip(circle_segment_radii, numbers_of_agents):
        outer_radius = circle_segment[1]
        inner_radius = circle_segment[0]
        big_circle_area = __intersecting_area_polygon_circle(
            center_point, outer_radius, polygon
        )
        small_circle_area = __intersecting_area_polygon_circle(
            center_point, inner_radius, polygon
        )
        placeable_area = big_circle_area - small_circle_area

        # checking whether to place points
        # inside the circle segment or
        # inside the bounding box of the intersection of polygon and Circle Segment

        # determine the entire area of the circle segment
        entire_circle_area = np.pi * (outer_radius**2 - inner_radius**2)
        # determine the area where a point might be placed around the polygon
        sec_box = __box_of_intersection(polygon, center_point, outer_radius)
        dif_x, dif_y = (
            sec_box[1][0] - sec_box[0][0],
            sec_box[1][1] - sec_box[0][1],
        )
        bounding_box_area = dif_x * dif_y

        if entire_circle_area < bounding_box_area:
            # inside the circle it is more likely to find a random point that is inside the polygon
            for placed_count in range(number):
                i = 0
                while i < max_iterations:
                    i += 1
                    # determines a random radius within the circle segment
                    rho = np.sqrt(
                        np.random.uniform(inner_radius**2, outer_radius**2)
                    )
                    # determines a random degree
                    theta = np.random.uniform(0, 2 * np.pi)
                    pt = center_point[0] + rho * np.cos(theta), center_point[
                        1
                    ] + rho * np.sin(theta)
                    if __check_distance_constraints(
                        pt, distance_to_polygon, grid, polygon
                    ):
                        grid.append_point(pt)
                        break

                if i >= max_iterations and placed_count != number:
                    message = (
                        f"the desired amount of agents in the Circle segment from"
                        f" {inner_radius} to {outer_radius} could not be achieved."
                        f"\nOnly {placed_count} of {number}  could be placed."
                        f"\nactual density: {round(placed_count / placeable_area, 2)} p/m²"
                    )
                    raise AgentNumberError(message)
        else:
            # placing point inside the bounding box is more likely to find a random point that is inside the circle
            placed_count = 0
            iterations = 0
            while placed_count < number:
                if iterations > max_iterations:
                    message = (
                        f"the desired amount of agents in the Circle segment from"
                        f" {inner_radius} to {outer_radius} could not be achieved."
                        f"\nOnly {placed_count} of {number}  could be placed."
                        f"\nactual density: {round(placed_count / placeable_area, 2)} p/m²"
                    )
                    raise AgentNumberError(message)
                temp_point = (
                    np.random.uniform(sec_box[0][0], sec_box[1][0]),
                    np.random.uniform(sec_box[0][1], sec_box[1][1]),
                )
                if __is_inside_circle(
                    temp_point, center_point, inner_radius, outer_radius
                ) and __check_distance_constraints(
                    temp_point, distance_to_polygon, grid, polygon
                ):
                    grid.append_point(temp_point)
                    iterations = 0
                    placed_count += 1
                else:
                    iterations += 1
    return grid.get_samples()


def distribute_in_circles_by_density(
    *,
    polygon,
    distance_to_agents,
    distance_to_polygon,
    center_point,
    circle_segment_radii,
    densities,
    seed=None,
    max_iterations=10_000,
):
    """returns points randomly placed inside the polygon inside each the circle segments

    :param polygon: shapely polygon in which the agents will be placed
    :param distance_to_agents: minimal distance between the centers of agents
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

    __catch_wrong_inputs(
        polygon=polygon,
        center_point=center_point,
        circle_segment_radii=circle_segment_radii,
        fill_parameters=densities,
    )
    number_of_agents = []
    for circle_segment, density in zip(circle_segment_radii, densities):
        big_circle_area = __intersecting_area_polygon_circle(
            center_point, circle_segment[1], polygon
        )
        small_circle_area = __intersecting_area_polygon_circle(
            center_point, circle_segment[0], polygon
        )
        placeable_area = big_circle_area - small_circle_area
        number_of_agents.append(int(density * placeable_area))

    return distribute_in_circles_by_number(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        center_point=center_point,
        circle_segment_radii=circle_segment_radii,
        numbers_of_agents=number_of_agents,
        seed=seed,
        max_iterations=max_iterations,
    )


def distribute_till_full(
    *,
    polygon,
    distance_to_agents,
    distance_to_polygon,
    seed=None,
    max_iterations=10_000,
    k=30,
):
    """returns as many randomly placed points as fit into the polygon.
    Points are distributed using Bridson’s algorithm for Poisson-disc sampling
    The algorithm is explained in Robert Bridson´s Paper "Fast Poisson Disk Sampling in Arbitrary Dimensions"
        :param polygon: shapely polygon in which the agents will be placed
        :param distance_to_agents: minimal distance between the centers of agents
        :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
        :param seed: define a seed for random generation, Default value is None which corresponds to a random value
        :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
        :param k: around each point k point will be created before the point is considered inactive
        :return: list of created points"""
    if not isinstance(polygon, shply.polygon.Polygon):
        raise IncorrectParameterError(
            f"Polygon is expected to be a shapely Polygon"
        )
    box = __get_bounding_box(polygon)

    np.random.seed(seed)
    # initialises a list for active Points and a Grid administering all created points
    active = []
    grid = Grid(box, distance_to_agents)
    # initialisation of the first point
    iteration = 0
    while iteration < max_iterations:
        first_point = (
            np.random.uniform(box[0][0], box[1][0]),
            np.random.uniform(box[0][1], box[1][1]),
        )
        if __check_distance_constraints(
            first_point, distance_to_polygon, grid, polygon
        ):
            grid.append_point(first_point)
            active.append(first_point)
            break
        iteration = iteration + 1
    if iteration >= max_iterations:
        raise IncorrectParameterError(
            "The first point could not be placed inside the polygon."
            " Check if there is enough space for agents provided inside the polygon"
        )

    # while points are active a random reference point is selected
    while active:
        ref_point = active[np.random.randint(0, len(active))]
        iteration = 0
        # tries to find a point around the reference Point
        while iteration < k:
            # determines a random radius within a circle segment
            # with radius from distance_to_agents to distance_to_agents * 2
            rho = np.sqrt(
                np.random.uniform(
                    distance_to_agents**2, 4 * distance_to_agents**2
                )
            )
            # determines a random degree
            theta = np.random.uniform(0, 2 * np.pi)
            pt = ref_point[0] + rho * np.cos(theta), ref_point[
                1
            ] + rho * np.sin(theta)
            if __check_distance_constraints(
                pt, distance_to_polygon, grid, polygon
            ):
                grid.append_point(pt)
                active.append(pt)
                break
            iteration = iteration + 1

        # if there was no point found around the reference point it is considered inactive
        if iteration >= k:
            active.remove(ref_point)

    return grid.get_samples()


def distribute_by_percentage(
    *,
    polygon,
    percent,
    distance_to_agents,
    distance_to_polygon,
    seed=None,
    max_iterations=10000,
    k=30,
):
    """returns points for the desired percentage of agents that fit inside the polygon (max possible number)
    fills the polygon entirely using Bridson’s algorithm for Poisson-disc sampling and then selects the percentage of placed agents


     :param polygon: shapely polygon in which the agents will be placed
     :param percent: percentage of agents selected - 100% ≙ completely filled polygon 0% ≙ 0 placed points
     :param distance_to_agents: minimal distance between the centers of agents
     :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
     :param seed: define a seed for random generation, Default value is None which corresponds to a random value
     :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
     :return: list of created points"""
    samples = distribute_till_full(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=seed,
        max_iterations=max_iterations,
        k=k,
    )
    sample_amount = len(samples)
    needed_amount = round(sample_amount * (percent / 100))
    np.random.seed(seed)
    np.random.shuffle(samples)

    return samples[:needed_amount]


def __check_distance_constraints(pt, wall_distance, grid, polygon):
    """Determines if a point has enough distance to other points and to the walls
     Uses a Grid to determine neighbours
    :param grid: the grid of the polygon
    :param pt: point that is being checked
    :param wall_distance: minimal distance between point and the polygon
    :param polygon: shapely Polygon in which the points must lie
    :return:True or False"""
    if not polygon.contains(shply.Point(pt)):
        return False
    if __min_distance_to_polygon(pt, polygon) < wall_distance:
        return False
    return grid.no_neighbours_in_distance(pt)


def __box_of_intersection(polygon, center_point, outer_radius):
    """returns an Axis Aligned Bounding Box containing the intersection of a Circle and the polygon
     @:param polygon is a shapely Polygon
     @:param center_point is the Center point of the Circle
    @:param outer_radius is the radius of the Circle
    @:return bounding box formatted like [(min(x_values), min(y_values)), (max(x_values), max(y_values))]
    """
    # creates a point
    point = shply.Point(center_point)
    # creates a layer with the size of the radius all around this point
    circle = point.buffer(outer_radius)
    # returns the size of the intersecting area
    shapely_bounds = polygon.intersection(circle).bounds
    return [shapely_bounds[:2], shapely_bounds[2:]]
