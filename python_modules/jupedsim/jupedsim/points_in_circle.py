import pytest
import numpy as np
from distributions import point_is_valid
from distributions import get_borders
import matplotlib.pyplot as plt
import shapely.geometry as sply

INFINITY = 10000


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
        if number is None and density is None:
            raise AgentCount(f"no number of agents and no density given when"
                             f" creating a Circle from {min_radius} to {max_radius} with center at {self.mid_point}")
        if min_radius < 0 or max_radius < 0:
            raise NegativeNumber("a new created Circle contained a negative radius")
        for circle in self.circles:
            if min_radius < max_radius <= circle[0] or circle[1] <= min_radius < max_radius:
                continue
            else:
                raise Overlapping(f"the new Circle would overlap with"
                                  f" the existing circle from {circle[0]} to {circle[1]}")
        self.circles.append((min_radius, max_radius, number, density))

    def remove_circle(self, min_radius, max_radius):
        for circle in self.circles:
            if circle[0] == min_radius and circle[1] == max_radius:
                self.circles.remove(circle)
                break

    def place_in_Polygon(self, polygon, agent_radius, wall_distance):
        samples = []
        for circle in self.circles:
            # if for the circle no exact number of agent is set it will be determined with the density
            if circle[2] is None:
                density = circle[3]
                big_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[1], polygon)
                small_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[0], polygon)
                possible_area = big_circle_area - small_circle_area
                targeted_count = round(density * possible_area)
            else:
                targeted_count = circle[2]
            for max_persons in range(targeted_count):
                i = 0
                while i < INFINITY:
                    i += 1
                    rho = np.sqrt(np.random.uniform(circle[0] ** 2, circle[1] ** 2))
                    theta = np.random.uniform(0, 2 * np.pi)
                    pt = self.mid_point[0] + rho * np.cos(theta), self.mid_point[1] + rho * np.sin(theta)
                    if point_is_valid(pt, polygon, agent_radius, wall_distance, samples):
                        samples.append(pt)
                        break
                    # debug
                    currently_placed = max_persons
                    # debug
                if i >= INFINITY:
                    break
            # DEBUG PRINTS START
            big_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[1], polygon)
            small_circle_area = intersecting_area_polygon_circle(self.mid_point, circle[0], polygon)
            area = big_circle_area - small_circle_area
            if circle[3]:
                print(f"targeted density: {circle[3]}")
            print(f"actual density: {currently_placed / area}")
            # DEBUG PRINTS END
        return samples

    def show_points(self, polygon, points, radius):
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


def intersecting_area_polygon_circle(mid_point, radius, polygon):
    # creates a point
    point = sply.Point(mid_point)
    # creates a layer with the size of the radius all around this point
    circle = point.buffer(radius)
    # creates a polygon in shapely
    poly = sply.Polygon(polygon)
    # returns the size of the intersecting area
    return poly.intersection(circle).area


# TESTING BEGINN
def test_placing_Circles():
    distribution = Distribution((0, 1))
    distribution.create_circle(0, 1, 2)
    assert distribution.mid_point == (0, 1)
    assert distribution.circles == [(0, 1, 2, None)]
    with pytest.raises(Overlapping):
        distribution.create_circle(0.5, 3, 5)
    with pytest.raises(NegativeNumber):
        distribution.create_circle(-1, -2, 5)
    distribution.create_circle(2, 3, 2)
    assert distribution.circles == [(0, 1, 2, None), (2, 3, 2, None)]
    with pytest.raises(Overlapping):
        distribution.create_circle(1.5, 2.5, 5)
    distribution.create_circle(1, 2, 2)
    assert distribution.circles == [(0, 1, 2, None), (2, 3, 2, None), (1, 2, 2, None)]
    distribution = Distribution((0, 1))
    distribution.create_circle(0, 1, density=1)
    assert distribution.circles == [(0, 1, None, 1)]


def test_removing_Circles():
    distribution = Distribution("no mid point")
    distribution.circles = [(0, 1, 2, None), (2, 3, 2, None)]
    distribution.remove_circle(0, 1)
    assert distribution.circles == [(2, 3, 2, None)]
# TESTING END


# currently unused
def area_of_polygon(polygon):
    n = len(polygon)
    area = 0
    i = 0
    while True:
        next = (i + 1) % n
        area += polygon[i][0] * polygon[next][1]
        area -= polygon[i][1] * polygon[next][0]

        i += 1
        if next == 0:
            break
    return abs(area) / 2


# every thing below is part of main functions and soon to be deleted
def demonstration():
    distribution = Distribution((8, 6))
    polygon = [(0, 0), (8, 2), (10, 6), (8, 10), (4, 10), (0, 6)]
    distribution.create_circle(0, 2, 8)
    distribution.create_circle(5, 7, 8)
    a_r, w_d = 1, 0.5
    samples = distribution.place_in_Polygon(polygon, a_r, w_d)
    distribution.show_points(polygon, samples, a_r)


def demonstration2():
    distribution = Distribution((15, 25))
    polygon = [(0, 0), (30, 0), (25, 5), (20, 5), (17.5, 15), (25, 15), (15, 25), (5, 15), (12.5, 15), (10, 5), (5, 5)]
    a_r, w_d = 2, 1
    distribution.create_circle(0, 9.5, 7)
    distribution.create_circle(20, 25, 10)
    samples = distribution.place_in_Polygon(polygon, a_r, w_d)
    distribution.show_points(polygon, samples, a_r)


def main():
    distribution = Distribution((9, 12))
    polygon = [(0, 0), (15, 0), (15, 10), (10, 10)]
    a_r, w_d = 0.75, 0.5
    distribution.create_circle(0, 5, density=1)
    distribution.create_circle(5, 8.5, density=0.2)
    distribution.create_circle(8.5, 10, density=0.05)
    samples = distribution.place_in_Polygon(polygon, a_r, w_d)
    distribution.show_points(polygon, samples, a_r)


if __name__ == "__main__":
    main()
