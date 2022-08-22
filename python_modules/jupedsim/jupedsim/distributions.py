import argparse
import random
from math import sqrt
import matplotlib.pyplot as plt
import numpy as np

# Define Infinite
INT_MAX = 10000
FOREVER = 10000


# Given three collinear points p, q, r,
# the function checks if point q lies on line segment 'p->r'
def on_segment(p: tuple, q: tuple, r: tuple) -> bool:
    if ((q[0] <= max(p[0], r[0])) &
            (q[0] >= min(p[0], r[0])) &
            (q[1] <= max(p[1], r[1])) &
            (q[1] >= min(p[1], r[1]))):
        return True

    return False


# To find orientation of ordered triplet (p, q, r).
# The function returns following values
# 0 --> p, q and r are collinear
# 1 --> Clockwise
# 2 --> Counterclockwise
def orientation(p: tuple, q: tuple, r: tuple) -> int:
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


def do_intersect(p1, q1, p2, q2):
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


# Returns true if the point p lies
# inside the polygon[] with n vertices
def is_inside_polygon(points: list, p: tuple) -> bool:
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
        next = (i + 1) % n

        if points[i][1] == p[1]:
            decrease += 1

        # Check if the line segment from 'p' to
        # 'extreme' intersects with the line
        # segment from 'polygon[i]' to 'polygon[next]'
        if (do_intersect(points[i],
                         points[next],
                         p, extreme)):

            # If the point 'p' is collinear with line
            # segment 'i-next', then check if it lies
            # on segment. If it lies, return true, otherwise false
            if orientation(points[i], p,
                           points[next]) == 0:
                return on_segment(points[i], p,
                                  points[next])

            count += 1

        i = next

        if i == 0:
            # if i is back at 0 every segment has been checked
            break

    # Reduce the count by decrease amount
    # as these points would have been added twice
    count -= decrease

    # Return true if count is odd, false otherwise
    return count % 2 == 1


def test_if_inside():
    polygon1 = [(0, 0), (10, 0), (10, 10), (0, 10)]

    p = (20, 20)
    assert is_inside_polygon(points=polygon1, p=p) is False

    p = (5, 5)
    assert is_inside_polygon(points=polygon1, p=p) is True

    polygon2 = [(0, 0), (5, 0), (5, 5), (3, 3)]

    p = (3, 3)
    assert is_inside_polygon(points=polygon2, p=p) is True

    p = (5, 1)
    assert is_inside_polygon(points=polygon2, p=p) is True

    p = (8, 1)
    assert is_inside_polygon(points=polygon2, p=p) is False

    p = (-1, 10)
    assert is_inside_polygon(points=polygon1, p=p) is False


def distance_between(pt1, pt2):
    dx = pt2[0] - pt1[0]
    dy = pt2[1] - pt1[1]
    return sqrt(dx ** 2 + dy ** 2)


def test_distance_determination():
    pt1 = (0, 0)
    pt2 = (2, 0)
    expected_result = 2.0
    exception_rate = 0.01
    actual_result = distance_between(pt1, pt2)
    difference = expected_result - actual_result
    assert difference == 0 or exception_rate > difference > 0 or -exception_rate < difference < 0
    pt1 = (0, 0)
    pt2 = (1, 1)
    expected_result = 1.41421
    exception_rate = 0.01
    actual_result = distance_between(pt1, pt2)
    difference = expected_result - actual_result
    assert difference == 0 or exception_rate > difference > 0 or -exception_rate < difference < 0


def pt_has_distance_to_others(n_point, existing_points, radius):
    i = 0
    for current_point in existing_points:
        distance = distance_between(n_point, current_point)
        i += 1
        if distance < radius:
            return False
    return True


def test_has_enough_distance():
    n_point = (0, 0)
    existing_points = [(2, 0), (0, 2), (5, 3)]
    radius = 1.5
    assert pt_has_distance_to_others(n_point, existing_points, radius) is True
    existing_points = [(1, 0), (0, 1)]
    assert pt_has_distance_to_others(n_point, existing_points, radius) is False


def point_is_valid(pt, polygon, agent_radius, wall_distance, other_points):
    return is_inside_polygon(polygon, pt) \
           and min_distance_to_polygon(pt, polygon) > wall_distance \
           and pt_has_distance_to_others(pt, other_points, agent_radius)


def create_random_points(polygon, count, agent_radius, wall_distance):
    borders = get_borders(polygon)
    samples = []
    created_points = 0
    iterations = 0
    while created_points < count:
        if iterations > FOREVER:
            return samples
        temp_point = (random.uniform(borders[0], borders[1]), random.uniform(borders[2], borders[3]))
        if point_is_valid(temp_point, polygon, agent_radius, wall_distance, samples):
            samples.append(temp_point)
            iterations = 0
            created_points += 1
        else:
            iterations += 1
    return samples


def min_distance_to_polygon(pt, polygon):
    distances = []
    n = len(polygon)
    i = 0
    while True:
        # find the number of the next line segment
        next = (i + 1) % n
        distances.append(distance_to_segment(polygon[i], polygon[next], pt))
        i = next
        if i == 0:
            break
    return min(distances)


def distance_to_segment(a, b, e):
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
        width = max(x) - min(x)
        height = max(y) - min(y)
        alpha = (25 * width * height) / (total * max_persons * np.pi)
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
    print(alpha)
    n = len(polygon)
    i = 0
    while True:
        next = (i + 1) % n
        x_value = [polygon[i][0], polygon[next][0]]
        y_value = [polygon[i][1], polygon[next][1]]
        plt.plot(x_value, y_value, color='blue')

        i += 1
        if next == 0:
            break
    plt.xlim(min(x) - 1, max(x) + 1)
    plt.ylim(min(y) - 1, max(y) + 1)
    plt.axis('on')
    plt.show()


def make_polygon(string_list):
    points = []
    for elem in string_list:
        elem = elem.strip("(")
        elem = elem.strip(")")
        numbers = elem.split(",")
        x = int(numbers[0])
        y = int(numbers[1])
        points.append((x, y))
    return points


def get_borders(polygon):
    """returns a list containing the minimal/maximal x and y values
    formatted like : [min(x_values), max(x_values), min(y_values), max(y_values)]"""
    x_values = y_values = []
    for point in polygon:
        x_values.append(point[0])
        y_values.append(point[1])
    return [min(x_values), max(x_values), min(y_values), max(y_values)]


def get_cell_coords(pt, cell_side_length, borders):
    """Get the coordinates of the cell that pt = (x,y) falls in."""

    return int((pt[0] - borders[0]) // cell_side_length), int((pt[1] - borders[2]) // cell_side_length)


def get_point(k, refpt, polygon, agent_radius, wall_distance, c_s_l, samples, nxny, cells):
    """Try to find a candidate point relative to refpt to emit in the sample.

    We draw up to k points from the annulus of inner radius r, outer radius 2r
    around the reference point, refpt. If none of them are suitable (because
    they're too close to existing points in the sample), return False.
    Otherwise, return the pt.

    """
    borders = get_borders(polygon)

    width, height = borders[1] - borders[0], borders[3] - borders[2]

    i = 0
    while i < k:
        i += 1
        rho = np.sqrt(np.random.uniform(agent_radius ** 2, 4 * agent_radius ** 2))
        theta = np.random.uniform(0, 2 * np.pi)
        pt = refpt[0] + rho * np.cos(theta), refpt[1] + rho * np.sin(theta)
        if not (0 <= pt[0] < width and 0 <= pt[1] < height):
            # This point falls outside the domain, so try again.
            continue
        if point_valid(pt, agent_radius, wall_distance, c_s_l, polygon, samples, nxny, cells):
            return pt

    # We failed to find a suitable point in the vicinity of refpt.
    return False


def get_neighbours(coords, nxny, cells):
    """Return the indexes of points in cells neighbouring cell at coords.

    For the cell at coords = (x,y), return the indexes of points in the cells
    with neighbouring coordinates illustrated below: ie those cells that could
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


def point_valid(pt, agent_radius, wall_distance, cell_side_length, polygon, samples, nxny, cells):
    """Is pt a valid point to emit as a sample?

    It must be no closer than r from any other point: check the cells in its
    immediate neighbourhood.

    """

    cell_coords = get_cell_coords(pt, cell_side_length, get_borders(polygon))
    if not is_inside_polygon(polygon, pt):
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


def create_points_everywhere(polygon, agent_radius, wall_distance):
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
    while True:
        pt = np.random.uniform(borders[0], borders[1]), np.random.uniform(borders[2], borders[3])
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
            continue

    while active:
        # choose a random "reference" point from the active list.
        idx = np.random.choice(active)
        refpt = samples[idx]
        # Try to pick a new point relative to the reference point.
        pt = get_point(k, refpt, polygon, agent_radius, wall_distance, c_s_l, samples, nxny, cells)
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


def show_points(polygon, points, radius):
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
        next = (i + 1) % n
        x_value = [polygon[i][0], polygon[next][0]]
        y_value = [polygon[i][1], polygon[next][1]]
        plt.plot(x_value, y_value, color='blue')

        i += 1
        if next == 0:
            break

    plt.xlim(borders[0], borders[1])
    plt.ylim(borders[2], borders[3])
    plt.axis('equal')
    plt.show()


def set_up():
    parser = argparse.ArgumentParser()
    parser.add_argument('a_r', type=float, help="first argument: minimal radius among agents")
    parser.add_argument('w_d', type=float, help="second argument: minimal radius between agents and polygon segments")
    parser.add_argument('-agents', type=int, help="number of agents placed, default value=10", default=10)
    parser.add_argument('-point', '-p', type=str, nargs="+",
                        help="define the Polygon with it´s corner Points, formatted like '(0,0)' '(0,1)'")
    parser.add_argument('-a', action="store_true",
                        help="-a for as many agents as possible with bridson´s poisson-disk algorithm", dest="all")
    parser.add_argument('-heatmap', type=int, default=None,
                        help="creates a heatmap regarding as many iterations as selected")
    # parser.add_argument("--self-test", action="store_true", help="Will run self tests, print the results and exit.")
    return parser.parse_args()


def main():
    args = set_up()
    polygon = make_polygon(args.point)
    samples = None
    if args.heatmap is not None:
        heatmap(args.all, args.a_r, args.w_d, polygon, args.heatmap, args.agents)
        exit(0)
    elif args.all:
        samples = create_points_everywhere(polygon, args.a_r, args.w_d)
    else:
        samples = create_random_points(polygon, args.agents, args.a_r, args.w_d)

    show_points(polygon, samples, args.a_r)


if __name__ == '__main__':
    main()
