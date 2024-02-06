# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import numpy as np
import shapely

from jupedsim.internal.grid import Grid


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
    point = shapely.Point(mid_point)
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
    pt = shapely.Point(pt)
    min_dist = polygon.exterior.distance(pt)
    for hole in polygon.interiors:
        candidate_dist = hole.distance(pt)
        min_dist = min(min_dist, candidate_dist)
    return min_dist


def distribute_by_number(
    *,
    polygon: shapely.Polygon,
    number_of_agents: int,
    distance_to_agents: float,
    distance_to_polygon: float,
    seed: int | None = None,
    max_iterations: int = 10000,
) -> list[tuple[float, float]]:
    """Generates specified number of randomized 2D coordiantes.

    This function will generate the speficied number of 2D coordiantes where
    all coordiantes are inside the specified geometry and generated coordinates
    are constraint by distance_to_agents and distance_to_polygon. This function
    may not always by able to generate the requested coordinate because it
    cannot do so without violating the constraints. In this case the function
    will stop after max_iterations and raise an Exception.

    Arguments:
        polygon: polygon where the agents shall be placed
        number_of_agents: number of agents to be distributed
        distance_to_agents: minimal distance between the centers of agents
        distance_to_polygon: minimal distance between the center of agents
            and the polygon edges
        seed: Will be used to seed the random number generator.
        max_iterations: Up to max_iterations are attempts are made to
            place a random point without conastraint violation, default is 10_000

    Returns:
        2D coordiantes

    Raises:
        :class:`AgentNumberError`: if not all agents could be placed.
        :class:`IncorrectParameterError`: if polygon is not of type
            :class:`~shapely.Polygon`

    """
    if not isinstance(polygon, shapely.Polygon):
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
    polygon: shapely.Polygon,
    density: float,
    distance_to_agents: float,
    distance_to_polygon: float,
    seed: int | None = None,
    max_iterations: int = 10000,
) -> list[tuple[float, float]]:
    """Generates randomized 2D coordinates based on a desired agent density per
    square meter.

    This function will generate as many 2D coordinates as required to reach the
    desired density. Essentially this function tries to place area * density
    many agents while adhering to the distance_to_polygon and
    distance_to_agents constraints. This function may not always by able to
    generate the requested coordinate because it cannot do so without violating
    the constraints. In this case the function will stop after max_iterations
    and raise an Exception.

    Arguments:
        polygon: Area where to generate 2D coordinates in.
        density: desired density in agents per square meter
        distance_to_agents: minimal distance between the centers of agents
        distance_to_polygon: minimal distance between the center of agents
            and the polygon edges
        seed: Will be used to seed the random number generator.
        max_iterations: Up to max_iterations are attempts are made to
            place a random point without constraint violation, default is 10_000


    Returns:
        2D coordiantes

    Raises:
        :class:`AgentNumberError`: if not all agents could be placed.
        :class:`IncorrectParameterError`: if polygon is not of type
            :class:`~shapely.Polygon`

    """

    if not isinstance(polygon, shapely.Polygon):
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

    if not isinstance(polygon, shapely.Polygon):
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
    polygon: shapely.Polygon,
    distance_to_agents: float,
    distance_to_polygon: float,
    center_point: tuple[float, float],
    circle_segment_radii: list[tuple[float, float]],
    numbers_of_agents: list[int],
    seed=None,
    max_iterations=10_000,
) -> list[tuple[float, float]]:
    """Generates randomized 2D coordiantes in a user defined number of rings.

    This function will generate 2D coordinates in the intersection of the
    polygon and the rings specified by the centerpoint and the min/max radii of
    each ring. `number_of_agents` is expected to contain the number of agents
    to be placed for each ring. This function may not always by able to
    generate the requested coordinate because it cannot do so without violating
    the constraints. In this case the function will stop after max_iterations
    and raise an Exception.

    Arguments:
        polygon: polygon where agents can be placed.
        distance_to_agents: minimal distance between the centers of agents
        distance_to_polygon: minimal distance between the center of agents
            and the polygon edges
        center_point: Center point of the rings.
        circle_segment_radii: min/max radius per ring, rings may not overlap
        number_of_agents: agents to be placed per ring
        seed: Will be used to seed the random number generator.
        max_iterations: Up to max_iterations are attempts are made to
            place a random point without conastraint violation, default is 10_000

    Returns:
        2D coordiantes

    Raises:
        :class:`AgentNumberError`: if not all agents could be placed.
        :class:`IncorrectParameterError`: if polygon is not of type
            :class:`~shapely.Polygon`
        :class:`OverlappingCirclesError`: if rings in circle_segment_radii
            overlapp
    """

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
    polygon: shapely.Polygon,
    distance_to_agents: float,
    distance_to_polygon: float,
    center_point: tuple[float, float],
    circle_segment_radii: list[tuple[float, float]],
    densities: list[float],
    seed: int | None = None,
    max_iterations: int = 10_000,
) -> list[tuple[float, float]]:
    """Generates randomized 2D coordiantes in a user defined number of rings
    with defined density.

    This function will generate 2D coordinates in the intersection of the
    polygon and the rings specified by the centerpoint and the min/max radii of
    each ring. The number of positions generated is defined by the desired
    density and available space of each ring. This function may not always by
    able to generate the requested coordinate because it cannot do so without
    violating the constraints. In this case the function will stop after
    max_iterations and raise an Exception.

    Arguments:
        polygon: polygon where agents can be placed.
        distance_to_agents: minimal distance between the centers of agents
        distance_to_polygon: minimal distance between the center of agents
            and the polygon edges
        center_point: Center point of the rings.
        circle_segment_radii: min/max radius per ring, rings may not overlap
        desnities: density in positionsper square meter for each ring
        seed: Will be used to seed the random number generator.
        max_iterations: Up to max_iterations are attempts are made to place a
            random point without conastraint violation, default is 10_000

    Returns:
        2D coordiantes

    Raises:
        :class:`AgentNumberError`: if not all agents could be placed.
        :class:`IncorrectParameterError`: if polygon is not of type
            :class:`~shapely.Polygon`
        :class:`OverlappingCirclesError`: if rings in circle_segment_radii
            overlapp
    """

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


def distribute_until_filled(
    *,
    polygon: shapely.Polygon,
    distance_to_agents: float,
    distance_to_polygon: float,
    seed: int | None = None,
    max_iterations: int = 10_000,
    k: int = 30,
) -> list[tuple[float, float]]:
    """Generates randomized 2D coordiantes that fill the specified area.

    This function will generate 2D coordinates in the specified area. The
    number of positions generated depends on the ability to place aditional
    points. This function may not always by able to generate the requested
    coordinate because it cannot do so without violating the constraints. In
    this case the function will stop after max_iterations and raise an
    Exception.

    Arguments:
        polygon: polygon where agents can be placed.
        distance_to_agents: minimal distance between the centers of agents
        distance_to_polygon: minimal distance between the center of agents
            and the polygon edges
        seed: Will be used to seed the random number generator.
        max_iterations: Up to max_iterations are attempts are made to
            place a random point without conastraint violation, default is 10_000
        k: maximum number of attempts to place neighbors to already inserted
            points. A higher value will result in a higher density but will greatly
            increase runtim.

    Returns:
        2D coordiantes

    Raises:
        :class:`AgentNumberError`: if not all agents could be placed.
        :class:`IncorrectParameterError`: if polygon is not of type
            :class:`~shapely.Polygon`
    """
    if not isinstance(polygon, shapely.Polygon):
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

    # Uses https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
    # "Fast Poisson Disk Sampling in Arbitrary Dimensions"
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
    polygon: shapely.Polygon,
    percent: float,
    distance_to_agents: float,
    distance_to_polygon: float,
    seed: int | None = None,
    max_iterations: int = 10000,
    k: int = 30,
):
    """Generates randomized 2D coordiantes that fill the specified area to a
    percentage of a possible maximum.

    This function will generate 2D coordinates in the specified area. The
    number of positions generated depends on the ability to place aditional
    points. This function may not always by able to generate the requested
    coordinate because it cannot do so without violating the constraints. In
    this case the function will stop after max_iterations and raise an
    Exception.

    Arguments:
        polygon: polygon where agents can be placed.
        percent: percent value of occupancy to generate. needs to be in
            the intervall (0, 100]
        distance_to_agents: minimal distance between the centers of agents
        distance_to_polygon: minimal distance between the center of agents
            and the polygon edges
        seed: Will be used to seed the random number generator.
        max_iterations: Up to max_iterations are attempts are made to
            place a random point without conastraint violation, default is 10_000
        k: maximum number of attempts to place neighbors to already inserted
            points. A higher value will result in a higher density but will greatly
            increase runtim.

    Returns:
        2D coordiantes

    Raises:
        :class:`AgentNumberError`: if not all agents could be placed.
        :class:`IncorrectParameterError`: if polygon is not of type
            :class:`~shapely.Polygon`
    """
    samples = distribute_until_filled(
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
    if not polygon.contains(shapely.Point(pt)):
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
    point = shapely.Point(center_point)
    # creates a layer with the size of the radius all around this point
    circle = point.buffer(outer_radius)
    # returns the size of the intersecting area
    shapely_bounds = polygon.intersection(circle).bounds
    return [shapely_bounds[:2], shapely_bounds[2:]]
