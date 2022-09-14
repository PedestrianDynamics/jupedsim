from math import sqrt
import matplotlib.pyplot as plt
import numpy as np
import shapely.geometry as shply

# Define Infinite
INT_MAX = 10000
FOREVER = 10000


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

    def place_in_Polygon(self, polygon, agent_radius, wall_distance, seed=None, obstacles=None):
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
            borders = get_borders(polygon)
            dif_x, dif_y = borders[1] - borders[0], borders[3] - borders[2]
            entire_polygon_area = dif_x * dif_y

            # creates a grid
            width, height = borders[1] - borders[0], borders[3] - borders[2]
            # Cell side length
            c_s_l = agent_radius / np.sqrt(2)
            # Number of cells in the x- and y-directions of the grid
            nx, ny = int(width / c_s_l) + 1, int(height / c_s_l) + 1
            nxny = nx, ny
            # A list of coordinates in the grid of cells
            coords_list = [(ix, iy) for ix in range(nx) for iy in range(ny)]
            # Initialize the dictionary of cells: each key is a cell's coordinates, the
            # corresponding value is the index of that cell's point's coordinates in the
            # samples list (or None if the cell is empty).
            cells = {coords: None for coords in coords_list}
            if entire_circle_area < entire_polygon_area:
                # inside the circle it is more likely to find a random point that is inside the polygon
                for placed_count in range(targeted_count + 1):
                    i = 0
                    while i < INT_MAX:
                        i += 1
                        # determines a random radius within the circle segment
                        rho = np.sqrt(np.random.uniform(circle[0] ** 2, circle[1] ** 2))
                        # determines a random degree
                        theta = np.random.uniform(0, 2 * np.pi)
                        pt = self.mid_point[0] + rho * np.cos(theta), self.mid_point[1] + rho * np.sin(theta)
                        if point_valid(pt, agent_radius, wall_distance, c_s_l, polygon, samples, nxny, cells,
                                       obstacles):
                            samples.append(pt)
                            cells[get_cell_coords(pt, c_s_l, borders)] = len(samples) - 1
                            break

                    if i >= INT_MAX and placed_count != targeted_count:
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
                    if iterations > FOREVER:
                        message = f"the desired amount of agents in the Circle from {circle[0]} to {circle[1]} " \
                                  f"could not be achieved.\nOnly {placed_count} of {targeted_count}  could be placed."
                        if circle[2] is None:
                            # if the circle has no number of agents
                            # the expected and actual density will be added to the exception message
                            message += f"\nexpected density: {circle[3]} p/m², " \
                                       f"actual density: {round(placed_count / placeable_area, 2)} p/m²"
                        raise AgentCount(message)
                    temp_point = (np.random.uniform(borders[0], borders[1]), np.random.uniform(borders[2], borders[3]))
                    if is_inside_circle(temp_point, self.mid_point, circle[0], circle[1]) \
                            and point_valid(temp_point, agent_radius, wall_distance, c_s_l, polygon, samples, nxny,
                                            cells, obstacles):
                        samples.append(temp_point)
                        cells[get_cell_coords(temp_point, c_s_l, borders)] = len(samples) - 1
                        iterations = 0
                        placed_count += 1
                    else:
                        iterations += 1
        return samples

    def show_points(self, polygon, points, radius, obstacles=None):
        """illustrates the polygon, circle segments and points"""
        if obstacles is None:
            obstacles = []
        samples = points
        borders = get_borders(polygon)
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)
        for circle in self.circles:
            ax.add_patch(plt.Circle(radius=circle[0], xy=self.mid_point, fill=False))
            ax.add_patch(plt.Circle(radius=circle[1], xy=self.mid_point, fill=False))
        for elem in samples:
            ax.add_patch(plt.Circle(radius=radius / 2, xy=elem, fill=False))
            ax.add_patch(plt.Circle(radius=0.1, xy=elem, color="r"))

        n = len(polygon)
        i = 0
        while True:
            following = (i + 1) % n
            x_value = [polygon[i][0], polygon[following][0]]
            y_value = [polygon[i][1], polygon[following][1]]
            plt.plot(x_value, y_value, color='blue')

            i += 1
            if following == 0:
                break

        for obstacle in obstacles:
            n = len(obstacle)
            i = 0
            while True:
                following = (i + 1) % n
                x_value = [obstacle[i][0], obstacle[following][0]]
                y_value = [obstacle[i][1], obstacle[following][1]]
                plt.plot(x_value, y_value, color='black')

                i += 1
                if following == 0:
                    break

        plt.xlim(borders[0], borders[1])
        plt.ylim(borders[2], borders[3])
        plt.axis('equal')
        plt.show()


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


def is_inside_polygon(points: list, p: tuple) -> bool:
    """ Returns true if the point p lies inside the polygon with n vertices """
    # gets borders around the obstacle
    borders = get_borders(points)
    if not borders[0] < p[0] < borders[1] and not borders[2] < p[1] < borders[3]:
        return False

    # the point lies within the borders of the obstacle
    n = len(points)

    # There must be at least 3 vertices
    # in polygon
    if n < 3:
        return False

    # Create a point for line segment
    # from p to infinite
    extreme = (INT_MAX, p[1])

    # To count number of points in polygon
    # whose y-coordinate is equal to
    # y-coordinate of the point
    decrease = 0
    count = i = 0

    while True:
        # find the number of the next line segment
        following = (i + 1) % n

        if points[i][1] == p[1]:
            decrease += 1

        # Check if the line segment from 'p' to
        # 'extreme' intersects with the line
        # segment from 'polygon[i]' to 'polygon[next]'
        if (do_intersect(points[i],
                         points[following],
                         p, extreme)):

            # If the point 'p' is collinear with line
            # segment 'i-next', then check if it lies
            # on segment. If it lies, return true, otherwise false
            if orientation(points[i], p,
                           points[following]) == 0:
                return on_segment(points[i], p,
                                  points[following])

            count += 1

        i = following

        if i == 0:
            # if i is back at 0 every segment has been checked
            break

    # Reduce the count by decrease amount
    # as these points would have been added twice
    count -= decrease

    # Return true if count is odd, false otherwise
    return count % 2 == 1


def do_intersect(p1, q1, p2, q2):
    """returns if the lines intersect"""
    # Find the four orientations needed for
    # general and special cases
    o1 = orientation(p1, q1, p2)
    o2 = orientation(p1, q1, q2)
    o3 = orientation(p2, q2, p1)
    o4 = orientation(p2, q2, q1)

    # General case
    if (o1 != o2) and (o3 != o4):
        return True

    # Special Cases
    # p1, q1 and p2 are collinear and
    # p2 lies on segment p1q1
    if (o1 == 0) and (on_segment(p1, p2, q1)):
        return True

    # p1, q1 and p2 are collinear and
    # q2 lies on segment p1q1
    if (o2 == 0) and (on_segment(p1, q2, q1)):
        return True

    # p2, q2 and p1 are collinear and
    # p1 lies on segment p2q2
    if (o3 == 0) and (on_segment(p2, p1, q2)):
        return True

    # p2, q2 and q1 are collinear and
    # q1 lies on segment p2q2
    if (o4 == 0) and (on_segment(p2, q1, q2)):
        return True

    return False


def orientation(p: tuple, q: tuple, r: tuple) -> int:
    """The function returns orientation of ordered triplet (p, q, r) with following values
    0 --> p, q and r are collinear
    1 --> Clockwise
    2 --> Counterclockwise """
    val = (((q[1] - p[1]) *
            (r[0] - q[0])) -
           ((q[0] - p[0]) *
            (r[1] - q[1])))

    if val == 0:
        return 0  # Collinear
    if val > 0:
        return 1  # Clockwise
    else:
        return 2  # Counterclockwise


def on_segment(p: tuple, q: tuple, r: tuple) -> bool:
    """ Given three collinear points p, q, r,
    the function checks if point q lies on line segment 'p->r'"""
    if ((q[0] <= max(p[0], r[0])) &
            (q[0] >= min(p[0], r[0])) &
            (q[1] <= max(p[1], r[1])) &
            (q[1] >= min(p[1], r[1]))):
        return True

    return False


def get_borders(polygon):
    """returns a list containing the minimal/maximal x and y values
    formatted like : [min(x_values), max(x_values), min(y_values), max(y_values)]"""
    x_values, y_values = [], []
    for point in polygon:
        x_values.append(point[0])
        y_values.append(point[1])

    return [min(x_values), max(x_values), min(y_values), max(y_values)]


def min_distance_to_polygon(pt, polygon):
    """returns the minimal distance between a point and every line segment of a polygon"""
    distances = []
    n = len(polygon)
    i = 0
    while True:
        # find the number of the next line segment
        following = (i + 1) % n
        distances.append(distance_to_segment(polygon[i], polygon[following], pt))
        i = following
        if i == 0:
            break
    return min(distances)


def distance_to_segment(a, b, e):
    """returns the minimal distance between point e and the line segment from a to b"""
    # vector AB
    ab = [None, None]
    ab[0] = b[0] - a[0]
    ab[1] = b[1] - a[1]

    # vector BP
    be = [None, None]
    be[0] = e[0] - b[0]
    be[1] = e[1] - b[1]

    # vector AP
    ae = [None, None]
    ae[0] = e[0] - a[0]
    ae[1] = e[1] - a[1]

    # Variables to store dot product

    # Calculating the dot product
    ab_be = ab[0] * be[0] + ab[1] * be[1]
    ab_ae = ab[0] * ae[0] + ab[1] * ae[1]

    # Minimum distance from
    # point E to the line segment

    # Case 1
    if ab_be > 0:

        # Finding the magnitude
        y = e[1] - b[1]
        x = e[0] - b[0]
        req_ans = sqrt(x * x + y * y)

    # Case 2
    elif ab_ae < 0:
        y = e[1] - a[1]
        x = e[0] - a[0]
        req_ans = sqrt(x * x + y * y)

    # Case 3
    else:

        # Finding the perpendicular distance
        x1 = ab[0]
        y1 = ab[1]
        x2 = ae[0]
        y2 = ae[1]
        mod = sqrt(x1 * x1 + y1 * y1)
        req_ans = abs(x1 * y2 - y1 * x2) / mod

    return req_ans


def distance_between(pt1, pt2):
    """returns the distance between point1 and point2"""
    dx = pt2[0] - pt1[0]
    dy = pt2[1] - pt1[1]
    return sqrt(dx ** 2 + dy ** 2)


def create_random_points(polygon, count, agent_radius, wall_distance, seed=None, obstacles=None, density=None):
    """returns points randomly placed inside the polygon

        :param polygon: List of corner points given as tuples
        :param count: number of points placed
        :param density: select a density: number of agents will be calculated and param count will not be used
        :param agent_radius: minimal distance between points
        :param wall_distance: minimal distance between points and the polygon
        :param seed: define a seed for random generation
        :param obstacles: list of polygons, no points can be inside the obstacle
        :return: list of created points
        if more that 10000 tries are needed to placed a valid point an Exception will be thrown
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

    # creates a grid
    borders = get_borders(polygon)
    width, height = borders[1] - borders[0], borders[3] - borders[2]
    # Cell side length
    c_s_l = agent_radius / np.sqrt(2)
    # Number of cells in the x- and y-directions of the grid
    nx, ny = int(width / c_s_l) + 1, int(height / c_s_l) + 1
    nxny = nx, ny
    # A list of coordinates in the grid of cells
    coords_list = [(ix, iy) for ix in range(nx) for iy in range(ny)]
    # Initialize the dictionary of cells: each key is a cell's coordinates, the
    # corresponding value is the index of that cell's point's coordinates in the
    # samples list (or None if the cell is empty).
    cells = {coords: None for coords in coords_list}

    samples = []
    created_points = 0
    iterations = 0
    while created_points < count:
        if iterations > FOREVER:
            msg = f"Only {created_points} of {count}  could be placed."
            if density is not None:
                msg += f"\nexpected density: {density} p/m², " \
                       f"actual density: {round(created_points / area, 2)} p/m²"
            raise AgentCount(msg)
        temp_point = (np.random.uniform(borders[0], borders[1]), np.random.uniform(borders[2], borders[3]))
        if point_valid(temp_point, agent_radius, wall_distance, c_s_l, polygon, samples, nxny, cells, obstacles):
            samples.append(temp_point)
            cells[get_cell_coords(temp_point, c_s_l, borders)] = len(samples) - 1

            iterations = 0
            created_points += 1
        else:
            iterations += 1

    return samples


def create_points_everywhere(polygon, agent_radius, wall_distance, seed=None, obstacles=None):
    """creates points all over the polygon with bridson´s poisson-disk algorithm
        :param polygon: List of corner points given as tuples
        :param agent_radius: minimal distance between points
        :param wall_distance: minimal distance between points and the polygon
        :param seed: define a seed for random generation
        :param obstacles: list of polygons, no points can be inside the obstacle
        :return: list of created points
    """
    if seed is not None:
        np.random.seed(seed)
    if obstacles is None:
        obstacles = []
    # Choose up to k points around each reference point as candidates for a new sample point
    k = 30
    borders = get_borders(polygon)
    width, height = borders[1] - borders[0], borders[3] - borders[2]
    # Cell side length
    c_s_l = agent_radius / np.sqrt(2)
    # Number of cells in the x- and y-directions of the grid
    nx, ny = int(width / c_s_l) + 1, int(height / c_s_l) + 1
    nxny = nx, ny
    # A list of coordinates in the grid of cells
    coords_list = [(ix, iy) for ix in range(nx) for iy in range(ny)]
    # Initialize the dictionary of cells: each key is a cell's coordinates, the
    # corresponding value is the index of that cell's point's coordinates in the
    # samples list (or None if the cell is empty).
    cells = {coords: None for coords in coords_list}

    # Pick a random point to start with.
    active = nsamples = samples = None
    i = 0
    while True:
        if i > FOREVER:
            raise AgentCount("No Agents could be placed inside the polygon")
        flag = False
        pt = np.random.uniform(borders[0], borders[1]), np.random.uniform(borders[2], borders[3])
        for obstacle in obstacles:
            if is_inside_polygon(obstacle, pt):
                flag = True
                break
        if flag:
            i += 1
            continue
        if is_inside_polygon(polygon, pt) and min_distance_to_polygon(pt, polygon) > wall_distance:
            samples = [pt]
            # Our first sample is indexed at 0 in the samples list...
            cells[get_cell_coords(pt, c_s_l, borders)] = 0
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
        pt = get_point(k, refpt, polygon, agent_radius, wall_distance, c_s_l, samples, nxny, cells, temp_seed,
                       obstacles)
        if pt:
            # Point pt is valid: add it to the samples list and mark it as active
            samples.append(pt)
            nsamples += 1
            active.append(len(samples) - 1)
            cells[get_cell_coords(pt, c_s_l, borders)] = len(samples) - 1
        else:
            # We had to give up looking for valid points near ref.pt, so remove it
            # from the list of "active" points.
            active.remove(idx)

    return samples


def get_cell_coords(pt, cell_side_length, borders):
    """ Get the coordinates of the cell that pt = (x,y) falls in.
        borders is list containing the minimal/maximal x and y values"""

    return int((pt[0] - borders[0]) // cell_side_length), int((pt[1] - borders[2]) // cell_side_length)


def get_point(k, refpt, polygon, agent_radius, wall_distance, c_s_l, samples, nxny, cells, seed=None, obstacles=None):
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
    borders = get_borders(polygon)

    i = 0
    while i < k:
        i += 1
        rho = np.sqrt(np.random.uniform(agent_radius ** 2, 4 * agent_radius ** 2))
        theta = np.random.uniform(0, 2 * np.pi)
        pt = refpt[0] + rho * np.cos(theta), refpt[1] + rho * np.sin(theta)
        if not (borders[0] <= pt[0] < borders[1] and borders[2] <= pt[1] < borders[3]):
            # This point falls outside the domain, so try again.
            continue
        if point_valid(pt, agent_radius, wall_distance, c_s_l, polygon, samples, nxny, cells, obstacles):
            return pt

    # We failed to find a suitable point in the vicinity of refpt. The Point will be declared as inactive
    return False


def point_valid(pt, agent_radius, wall_distance, cell_side_length, polygon, samples, nxny, cells, obstacles=None):
    """ Determines if a point is valid by using a Grid to determine neighbours
    :param pt: point that is being checked
    :param agent_radius:minimal distance between points
    :param wall_distance: minimal distance between point and the polygon
    :param cell_side_length: Cell side length of the Grid
    :param polygon: Polygon in which the points must lie
    :param samples: already placed points
    :param nxny: Number of cells in the x- and y-direction as Tuple: (nx, ny)
    :param cells: Dictionary with key: cell, value: point
    :param obstacles: list of polygons, point must not lay within a polygon
    :return: if valid: True else: False
    """
    if obstacles is None:
        obstacles = []
    cell_coords = get_cell_coords(pt, cell_side_length, get_borders(polygon))
    if not is_inside_polygon(polygon, pt):
        return False
    for obstacle in obstacles:
        if is_inside_polygon(obstacle, pt):
            return False
    if min_distance_to_polygon(pt, polygon) < wall_distance:
        return False
    for idx in get_neighbours(cell_coords, nxny, cells):
        nearby_pt = samples[idx]
        # Squared distance between or candidate point, pt, and this nearby_pt.
        distance2 = (nearby_pt[0] - pt[0]) ** 2 + (nearby_pt[1] - pt[1]) ** 2
        if distance2 < agent_radius ** 2:
            # The points are too close, so pt is not a candidate.
            return False
    # All points tested: if we're here, pt is valid
    return True


def get_neighbours(coords, nxny, cells):
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
    nx, ny = nxny[0], nxny[1]
    dxdy = [(-1, -2), (0, -2), (1, -2), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1),
            (-2, 0), (-1, 0), (1, 0), (2, 0), (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1),
            (-1, 2), (0, 2), (1, 2), (0, 0)]
    neighbours = []
    for dx, dy in dxdy:
        neighbour_coords = coords[0] + dx, coords[1] + dy
        if not (0 <= neighbour_coords[0] < nx and
                0 <= neighbour_coords[1] < ny):
            # We're off the grid: no neighbours here.
            continue
        neighbour_cell = cells[neighbour_coords]
        if neighbour_cell is not None:
            # This cell is occupied: store this index of the contained point.
            neighbours.append(neighbour_cell)
    return neighbours


def heatmap(all, agent_radius, wall_distance, polygon, iterations, max_persons=50):
    a_r = agent_radius
    w_d = wall_distance
    x = y = []
    # determines all x and y values to draw the polygon
    for point in polygon:
        x.append(point[0])
        y.append(point[1])

    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)

    total = iterations
    if all:
        alpha = (25 * (a_r ** 2)) / total

    else:
        area = shply.Polygon(polygon).area
        alpha = (25 * area) / (total * max_persons * np.pi)
    if alpha < 0.01:
        alpha = 0.01
    elif alpha > 1:
        alpha = 1

    for temp in range(total):
        print(f"{temp / total * 100}%")
        # creates the random points
        if all:
            samples = create_points_everywhere(polygon, a_r, w_d)
        else:
            samples = create_random_points(polygon, max_persons, a_r, w_d)

        for elem in samples:
            ax.add_patch(plt.Circle(radius=0.1, xy=elem, color="r", alpha=alpha))
    n = len(polygon)
    i = 0
    while True:
        following = (i + 1) % n
        x_value = [polygon[i][0], polygon[following][0]]
        y_value = [polygon[i][1], polygon[following][1]]
        plt.plot(x_value, y_value, color='blue')

        i += 1
        if following == 0:
            break
    plt.xlim(min(x) - 1, max(x) + 1)
    plt.ylim(min(y) - 1, max(y) + 1)
    plt.axis('on')
    plt.show()


def show_points(polygon, points, radius, obstacles=None):
    if obstacles is None:
        obstacles = []
    samples = points
    borders = get_borders(polygon)
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    for elem in samples:
        ax.add_patch(plt.Circle(radius=radius / 2, xy=elem, fill=False))
        ax.add_patch(plt.Circle(radius=0.1, xy=elem, color="r"))

    n = len(polygon)
    i = 0
    while True:
        following = (i + 1) % n
        x_value = [polygon[i][0], polygon[following][0]]
        y_value = [polygon[i][1], polygon[following][1]]
        plt.plot(x_value, y_value, color='blue')

        i += 1
        if following == 0:
            break

    for obstacle in obstacles:
        n = len(obstacle)
        i = 0
        while True:
            following = (i + 1) % n
            x_value = [obstacle[i][0], obstacle[following][0]]
            y_value = [obstacle[i][1], obstacle[following][1]]
            plt.plot(x_value, y_value, color='black')

            i += 1
            if following == 0:
                break

    plt.xlim(borders[0], borders[1])
    plt.ylim(borders[2], borders[3])
    plt.axis('equal')
    plt.show()
