import math

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

    def place_in_Polygon(self, polygon, agent_radius, wall_distance, seed=None, max_iterations=10_000):
        """returns points inside each circle segment
            points have an agent_radius within which no other point may be placed.
            points will not be placed with less than wall_distance to the polygon
            points will be placed inside the polygon and inside the circle segment
            points are placed first in the segment that was created first
            if more that 10000 tries are needed to placed a valid point an Exception will be thrown
            no points may lay within an obstacle"""
        if seed is not None:
            np.random.seed(seed)
        samples = []
        for circle in self.circles:
            # if for the circle no exact number of agents is set it will be determined with the density
            big_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[1], polygon)
            small_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[0], polygon)
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
            box = get_bounding_box(polygon)
            dif_x, dif_y = box[1][0] - box[0][0], box[1][1] - box[0][1]
            entire_polygon_area = dif_x * dif_y

            grid = Grid(box, agent_radius)
            if entire_circle_area < entire_polygon_area:
                # inside the circle it is more likely to find a random point that is inside the polygon
                for placed_count in range(targeted_count + 1):
                    i = 0
                    while i < max_iterations:
                        i += 1
                        # determines a random radius within the circle segment
                        rho = np.sqrt(np.random.uniform(circle[0] ** 2, circle[1] ** 2))
                        # determines a random degree
                        theta = np.random.uniform(0, 2 * np.pi)
                        pt = self.mid_point[0] + rho * np.cos(theta), self.mid_point[1] + rho * np.sin(theta)
                        if check_distance_constraints(pt, wall_distance, grid, polygon):
                            samples.append(pt)
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
                            and check_distance_constraints(temp_point, wall_distance,grid, polygon):
                        samples.append(temp_point)
                        grid.append_point(temp_point)
                        iterations = 0
                        placed_count += 1
                    else:
                        iterations += 1
        return samples


class Grid:
    def __init__(self, box, agent_radius):
        self.box = box
        self.a_r = agent_radius
        width, height = box[1][0] - box[0][0], box[1][1] - box[0][1]
        # Cell side length
        self.c_s_l = agent_radius / np.sqrt(2)
        # Number of cells in the x- and y-directions of the grid
        self.nx, self.ny = int(width / self.c_s_l) + 1, int(height / self.c_s_l) + 1
        # A list of coordinates in the grid of cells
        self.coords_list = [(ix, iy) for ix in range(self.nx) for iy in range(self.ny)]
        # Initialize the dictionary of cells: each key is a cell's coordinates, the
        # corresponding value is the index of that cell's point's coordinates in the
        # samples list (or None if the cell is empty).
        self.cells = {coords: None for coords in self.coords_list}

    def append_point(self, pt):
        cell_coords = self.get_cell_coords(pt)
        self.cells[cell_coords] = pt

    def get_cell_coords(self, pt):
        """ Get the coordinates of the cell that pt = (x,y) falls in.
            box is bounding box containing the minimal/maximal x and y values"""
        return int((pt[0] - self.box[0][0]) // self.c_s_l), int((pt[1] - self.box[0][1]) // self.c_s_l)

    def no_neighbours_in_distance(self, pt):
        coords = self.get_cell_coords(pt)
        return not self.has_neighbour_in_distance(pt, coords)

    def has_neighbour_in_distance(self, pt, coords):
        """"returns true if there is any point in grid with lt or equal the distance `agent radius` to `pt`"""
        dxdy = [(-1, -2), (0, -2), (1, -2), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1),
                (-2, 0), (-1, 0), (1, 0), (2, 0), (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1),
                (-1, 2), (0, 2), (1, 2), (0, 0)]
        for dx, dy in dxdy:
            neighbour_coords = coords[0] + dx, coords[1] + dy
            if not (0 <= neighbour_coords[0] < self.nx and
                    0 <= neighbour_coords[1] < self.ny):
                # Points are not on the grid
                continue
            neighbour = self.cells[neighbour_coords]
            if neighbour is not None:
                # Inside a Cell close the the point is a potential neighbour
                dif_y, dif_x = neighbour[1] - pt[1], neighbour[0] - pt[0]
                distance = math.sqrt((dif_x**2) + (dif_y**2))
                if distance < self.a_r:
                    return True
        return False


def intersecting_area_polygon_circle(mid_point, radius, polygon):
    """returns the intersecting area of circle and polygon"""
    # creates a point
    point = shply.Point(mid_point)
    # creates a layer with the size of the radius all around this point
    circle = point.buffer(radius)
    # creates a polygon in shapely
    poly = shply.Polygon(polygon)
    # returns the size of the intersecting area
    return poly.intersection(circle).area


def is_inside_circle(point, mid, min_r, max_r):
    """checks if a point is inside a Circle segment reaching from minimum radius to maximum radius"""
    dif_x = point[0] - mid[0]
    dif_y = point[1] - mid[1]
    circle_equation = dif_x ** 2 + dif_y ** 2
    return min_r ** 2 <= circle_equation <= max_r ** 2


def get_bounding_box(polygon):
    """returns an Axis Aligned Bounding Box containing the minimal/maximal x and y values
    formatted like : [(min(x_values), min(y_values)), (max(x_values), max(y_values))]"""
    x_values, y_values = [], []
    for point in polygon:
        x_values.append(point[0])
        y_values.append(point[1])

    return [(min(x_values), min(y_values)), ((max(x_values)), max(y_values))]


def min_distance_to_polygon(pt, polygon):
    """returns the minimal distance between a point and every line segment of a polygon"""
    pt = shply.Point(pt)
    polygon = shply.Polygon(polygon)
    min_dist = polygon.exterior.distance(pt)
    for hole in polygon.interiors:
        candidate_dist = hole.distance(pt)
        min_dist = min(min_dist, candidate_dist)
    return min_dist


def create_random_points(polygon, count, agent_radius, wall_distance,
                         seed=None, density=None, max_iterations=10000):
    """returns points randomly placed inside the polygon

        :param polygon: List of corner points given as tuples
        :param count: number of points placed
        :param density: select a density: number of agents will be calculated and param count will not be used
        :param agent_radius: minimal distance between points
        :param wall_distance: minimal distance between points and the polygon
        :param seed: define a seed for random generation
        :param max_iterations: no more than max_iterations must find a point inside the polygon
        :return: list of created points
    """
    if seed is not None:
        np.random.seed(seed)
    if density is not None:
        area = shply.Polygon(polygon).area
        count = round(density * area)

    box = get_bounding_box(polygon)
    grid = Grid(box, agent_radius)
    samples = []
    created_points = 0
    iterations = 0
    while created_points < count:
        if iterations > max_iterations:
            msg = f"Only {created_points} of {count}  could be placed."
            if density is not None:
                msg += f"\nexpected density: {density} p/m², " \
                       f"actual density: {round(created_points / area, 2)} p/m²"
            raise AgentCount(msg)
        temp_point = (np.random.uniform(box[0][0], box[1][0]), np.random.uniform(box[0][1], box[1][1]))
        if check_distance_constraints(temp_point, wall_distance, grid, polygon):
            samples.append(temp_point)
            grid.append_point(temp_point)

            iterations = 0
            created_points += 1
        else:
            iterations += 1

    return samples


def check_distance_constraints(pt, wall_distance, grid, polygon):
    """ Determines if a point has enough distance to other points and to the walls
     Uses a Grid to determine neighbours
    :param grid: the grid of the polygon
    :param pt: point that is being checked
    :param wall_distance: minimal distance between point and the polygon
    :param polygon: Polygon in which the points must lie
    :return: if valid: True else: False
    """

    if not shply.Polygon(polygon).contains(shply.Point(pt)):
        return False
    if min_distance_to_polygon(pt, polygon) < wall_distance:
        return False
    return grid.no_neighbours_in_distance(pt)
