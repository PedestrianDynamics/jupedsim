import pytest
import numpy as np
from distributions import point_is_valid
from distributions import get_borders
import matplotlib.pyplot as plt

INFINITY = 10000


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

    def create_circle(self, min_radius, max_radius, number):
        if min_radius < 0 or max_radius < 0:
            raise NegativeNumber("a new created Circle contained a negative radius")
        for circle in self.circles:
            if min_radius < max_radius <= circle[0] or circle[1] <= min_radius < max_radius:
                continue
            else:
                raise Overlapping(f"the new Circle would overlap with"
                                  f" the existing circle from {circle[0]} to {circle[1]}")
        self.circles.append((min_radius, max_radius, number))

    def remove_circle(self, min_radius, max_radius, number):
        self.circles.remove((min_radius, max_radius, number))

    def place_in_Polygon(self, polygon, agent_radius, wall_distance):
        samples = []
        for circle in self.circles:
            for max_persons in range(circle[2]):
                i = 0
                while i < INFINITY:
                    i += 1
                    rho = np.sqrt(np.random.uniform(circle[0] ** 2, circle[1] ** 2))
                    theta = np.random.uniform(0, 2 * np.pi)
                    pt = self.mid_point[0] + rho * np.cos(theta), self.mid_point[1] + rho * np.sin(theta)
                    if point_is_valid(pt, polygon, agent_radius, wall_distance, samples):
                        samples.append(pt)
                        break
                if i >= INFINITY:
                    break
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


def test_placing_Circles():
    distribution = Distribution((0, 1))
    distribution.create_circle(0, 1, 2)
    assert distribution.mid_point == (0, 1)
    assert distribution.circles == [(0, 1, 2)]
    with pytest.raises(Overlapping):
        distribution.create_circle(0.5, 3, 5)
    with pytest.raises(NegativeNumber):
        distribution.create_circle(-1, -2, 5)
    distribution.create_circle(2, 3, 2)
    assert distribution.circles == [(0, 1, 2), (2, 3, 2)]
    with pytest.raises(Overlapping):
        distribution.create_circle(1.5, 2.5, 5)
    distribution.create_circle(1, 2, 2)
    assert distribution.circles == [(0, 1, 2), (2, 3, 2), (1, 2, 2)]


def test_removing_Circles():
    distribution = Distribution("no mid point")
    distribution.circles = [(0, 1, 2), (2, 3, 2)]
    distribution.remove_circle(0, 1, 2)
    assert distribution.circles == [(2, 3, 2)]


def demonstration():
    distribution = Distribution((8, 6))
    polygon = [(0, 0), (8, 2), (10, 6), (8, 10), (4, 10), (0, 6)]
    distribution.create_circle(0, 2, 8)
    distribution.create_circle(5, 7, 8)
    a_r, w_d = 1, 0.5
    samples = distribution.place_in_Polygon(polygon, a_r, w_d)
    distribution.show_points(polygon, samples, a_r)


def main():
    distribution = Distribution((15, 25))
    polygon = [(0, 0), (30, 0), (25, 5), (20, 5), (17.5, 15), (25, 15), (15, 25), (5, 15), (12.5, 15), (10, 5), (5, 5)]
    a_r, w_d = 2, 1
    distribution.create_circle(0, 9.5, 7)
    distribution.create_circle(20, 25, 10)
    samples = distribution.place_in_Polygon(polygon, a_r, w_d)
    distribution.show_points(polygon, samples, a_r)


if __name__ == "__main__":
    main()
