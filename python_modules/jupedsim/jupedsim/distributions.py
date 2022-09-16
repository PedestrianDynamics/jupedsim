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

    def place_in_Polygon(self, polygon, agent_radius, wall_distance, seed=None, obstacles=None, max_iterations=10_000):
        """returns points inside each circle segment
            points have an agent_radius within which no other point may be placed.
            points will not be placed with less than wall_distance to the polygon
            points will be placed inside the polygon and inside the circle segment
            points are placed first in the segment that was created first
            if more that 10000 tries are needed to placed a valid point an Exception will be thrown
            no points may lay within an obstacle"""
        if obstacles is None:
            obstacles = []
        if seed is not None:
            np.random.seed(seed)
        samples = []
        for circle in self.circles:
            # if for the circle no exact number of agents is set it will be determined with the density
            big_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[1], polygon)
            small_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[0], polygon)
            placeable_area = big_circle_area - small_circle_area
            # it is expected that the entire obstacle intersects with the area
            for obstacle in obstacles:
                placeable_area -= intersecting_area_polygon_circle(self.mid_point, circle[1], obstacle)
                placeable_area += intersecting_area_polygon_circle(self.mid_point, circle[0], obstacle)
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
                        if point_valid(pt, agent_radius, wall_distance, grid, polygon, samples, obstacles):
                            samples.append(pt)
                            grid.append_point(pt, len(samples) - 1)
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
                            and point_valid(temp_point, agent_radius, wall_distance, grid, polygon, samples,
                                            obstacles):
                        samples.append(temp_point)
                        grid.append_point(temp_point, len(samples) - 1)
                        iterations = 0
                        placed_count += 1
                    else:
                        iterations += 1
        return samples


class Grid:
    def __init__(self, box, agent_radius):
        self.box = box
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

    def append_point(self, pt, sample_number):
        """adds the point´s sample number to the corresponding cell in the dictionary"""
        cell_coords = self.get_cell_coords(pt)
        self.cells[cell_coords] = sample_number

    def get_cell_coords(self, pt):
        """ Get the coordinates of the cell that pt = (x,y) falls in.
            box is bounding box containing the minimal/maximal x and y values"""
        return int((pt[0] - self.box[0][0]) // self.c_s_l), int((pt[1] - self.box[0][1]) // self.c_s_l)

    def determine_neighbours(self, coords):
        """Return the indexes of points in cells neighbouring cell at coords.

        For the cell at coords = (x,y), return the indexes of points in the cells
        with neighbouring coordinates illustrated below: I.e. those cells that could
        contain points closer than r.

                                         ooo
                                        ooooo
                                        ooXoo
                                        ooooo
                                         ooo

        """
        dxdy = [(-1, -2), (0, -2), (1, -2), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1),
                (-2, 0), (-1, 0), (1, 0), (2, 0), (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1),
                (-1, 2), (0, 2), (1, 2), (0, 0)]
        neighbours = []
        for dx, dy in dxdy:
            neighbour_coords = coords[0] + dx, coords[1] + dy
            if not (0 <= neighbour_coords[0] < self.nx and
                    0 <= neighbour_coords[1] < self.ny):
                # We're off the grid: no neighbours here.
                continue
            neighbour_cell = self.cells[neighbour_coords]
            if neighbour_cell is not None:
                # This cell is occupied: store this index of the contained point.
                neighbours.append(neighbour_cell)
        return neighbours

    def get_neighbours(self, pt):
        coords = self.get_cell_coords(pt)
        return self.determine_neighbours(coords)


class GridMock(Grid):
    def __init__(self):
        pass


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
    shapely_polygon = polygon[:]
    shapely_polygon.append(polygon[0])
    return shply.LineString(shapely_polygon).distance(shply.Point(pt))


def create_random_points(polygon, count, agent_radius, wall_distance,
                         seed=None, obstacles=None, density=None, max_iterations=10000):
    """returns points randomly placed inside the polygon

        :param polygon: List of corner points given as tuples
        :param count: number of points placed
        :param density: select a density: number of agents will be calculated and param count will not be used
        :param agent_radius: minimal distance between points
        :param wall_distance: minimal distance between points and the polygon
        :param seed: define a seed for random generation
        :param obstacles: list of polygons, no points can be inside the obstacle
        :param max_iterations: no more than max_iterations must find a point inside the polygon
        :return: list of created points
    """
    if obstacles is None:
        obstacles = []
    if seed is not None:
        np.random.seed(seed)
    if density is not None:
        area = shply.Polygon(polygon).area
        # it is expected that the entire obstacle intersects with the area
        for obstacle in obstacles:
            area -= shply.Polygon(obstacle).area
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
        if point_valid(temp_point, agent_radius, wall_distance, grid, polygon, samples, obstacles):
            samples.append(temp_point)
            grid.append_point(temp_point, len(samples) - 1)

            iterations = 0
            created_points += 1
        else:
            iterations += 1

    return samples


def create_points_everywhere(polygon, agent_radius, wall_distance, seed=None, obstacles=None, max_iterations=10000):
    """creates points all over the polygon with bridson´s poisson-disk algorithm
        :param polygon: List of corner points given as tuples
        :param agent_radius: minimal distance between points
        :param wall_distance: minimal distance between points and the polygon
        :param seed: define a seed for random generation
        :param obstacles: list of polygons, no points can be inside the obstacle
        :param max_iterations: there will be max_iterations to find a point inside the polygon
        :return: list of created points
    """
    if seed is not None:
        np.random.seed(seed)
    if obstacles is None:
        obstacles = []
    # Choose up to k points around each reference point as candidates for a new sample point
    k = 30
    box = get_bounding_box(polygon)
    grid = Grid(box, agent_radius)
    # Pick a random point to start with.
    active = nsamples = samples = None
    i = 0
    while True:
        if i > max_iterations:
            raise AgentCount("No Agents could be placed inside the polygon")
        flag = False

        pt = np.random.uniform(box[0][0], box[1][0]), np.random.uniform(box[0][1], box[1][1])
        for obstacle in obstacles:
            if shply.Polygon(obstacle).contains(shply.Point(pt)):
                flag = True
                break
        if flag:
            i += 1
            continue
        if point_valid(pt, agent_radius, wall_distance, grid, polygon, [], obstacles):
            samples = [pt]
            # Our first sample is indexed at 0 in the samples list...
            grid.append_point(pt, 0)
            # ... and it is active, in the sense that we're going to look for more points
            # in its neighbourhood.
            active = [0]
            nsamples = 1
            break
        else:
            i += 1
            continue

    while active:
        temp_seed = None
        # choose a random "reference" point from the active list.
        idx = np.random.choice(active)
        refpt = samples[idx]
        # Try to pick a new point relative to the reference point.
        if seed is not None:
            # the function can not be called with the same seed every time
            # a random seed will be chosen every time
            temp_seed = np.random.randint(0, 50000)
        pt = get_point(k, refpt, polygon, agent_radius, wall_distance, grid, samples, temp_seed, obstacles)
        if pt:
            # Point pt is valid: add it to the samples list and mark it as active
            samples.append(pt)
            nsamples += 1
            active.append(len(samples) - 1)
            grid.append_point(pt, len(samples) - 1)
        else:
            # We had to give up looking for valid points near ref.pt, so remove it
            # from the list of "active" points.
            active.remove(idx)

    return samples


def get_point(k, refpt, polygon, agent_radius, wall_distance, grid, samples, seed=None, obstacles=None):
    """Try to find a candidate point relative to refpt to emit in the sample.

    We draw up to k points from the annulus of inner radius r, outer radius 2r
    around the reference point, refpt. If none of them are suitable (because
    they're too close to existing points in the sample), return False.
    Otherwise, return the pt.

    :param k: up to k points are drawn until a reference point is declared as inactive
    :param refpt: points are drawn around this reference point
    :param polygon: Polygon in which the points must lie
    :param agent_radius: minimal distance between points
    :param wall_distance: minimal distance between points and the polygon
    :param samples: already placed points
    :param c_s_l: Cell side length of the grid
    :param nxny: Number of cells in the x- and y-direction as Tuple: (nx, ny)
    :param cells: dictionary with key: cell, value: point
    :param seed: define a seed for random generation
    :param obstacles: list of polygons, no points can be inside the obstacle
    """
    if seed is not None:
        np.random.seed(seed)
    box = get_bounding_box(polygon)

    i = 0
    while i < k:
        i += 1
        rho = np.sqrt(np.random.uniform(agent_radius ** 2, 4 * agent_radius ** 2))
        theta = np.random.uniform(0, 2 * np.pi)
        pt = refpt[0] + rho * np.cos(theta), refpt[1] + rho * np.sin(theta)
        if not (box[0][0] <= pt[0] < box[1][0] and box[0][1] <= pt[1] < box[1][1]):
            # This point falls outside the domain, so try again.
            continue
        if point_valid(pt, agent_radius, wall_distance, grid, polygon, samples, obstacles):
            return pt

    # We failed to find a suitable point in the vicinity of refpt. The Point will be declared as inactive
    return False


def point_valid(pt, agent_radius, wall_distance, grid, polygon, samples, obstacles=None):
    """ Determines if a point is valid by using a Grid to determine neighbours
    :param grid: the grid of the polygon
    :param pt: point that is being checked
    :param agent_radius:minimal distance between points
    :param wall_distance: minimal distance between point and the polygon
    :param polygon: Polygon in which the points must lie
    :param samples: already placed points
    :param obstacles: list of polygons, point must not lay within a polygon
    :return: if valid: True else: False
    """
    if obstacles is None:
        obstacles = []
    if not shply.Polygon(polygon).contains(shply.Point(pt)):
        return False
    for obstacle in obstacles:
        if shply.Polygon(obstacle).contains(shply.Point(pt)):
            return False
    if min_distance_to_polygon(pt, polygon) < wall_distance:
        return False
    for idx in grid.get_neighbours(pt):
        nearby_pt = samples[idx]
        # Squared distance between or candidate point, pt, and this nearby_pt.
        distance2 = (nearby_pt[0] - pt[0]) ** 2 + (nearby_pt[1] - pt[1]) ** 2
        if distance2 < agent_radius ** 2:
            # The points are too close, so pt is not a candidate.
            return False
    # All points tested: if we're here, pt is valid
    return True
