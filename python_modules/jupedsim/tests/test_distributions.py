import distributions
import pytest


def test_seed_works_correct_for_poisson_disc():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    set_seed = 1337
    samples1 = distributions.create_points_everywhere(polygon, agent_radius=0.5, wall_distance=0.5, seed=set_seed)
    samples2 = distributions.create_points_everywhere(polygon, agent_radius=0.5, wall_distance=0.5, seed=set_seed)
    assert samples1 == samples2


def test_cell_coord_determination():
    pt = (5, 5)
    borders = [0, 10, 0, 10]
    c_s_l = 0.7071067811865475  # 1/√2
    assert (7, 7) == distributions.get_cell_coords(pt, c_s_l, borders)


def test_point_in_circle():
    mid = (0, 0)
    min_radius, max_radius = 5, 10
    test_point = (2, 2)
    assert distributions.is_inside_circle(test_point, mid, min_radius, max_radius) is False
    test_point = (10, 10)
    assert distributions.is_inside_circle(test_point, mid, min_radius, max_radius) is False
    test_point = (7, 7)
    assert distributions.is_inside_circle(test_point, mid, min_radius, max_radius) is True


def test_border_determination():
    polygon = [(6, 0), (9, 2), (11, 4), (12, 7), (11.5, 9.5), (9.5, 10.5), (7.5, 10),
               (6, 9), (4.5, 10), (2.5, 10.5), (0.6, 9.5), (0, 7), (1, 4), (3, 2)]
    expected_borders = [0, 12, 0, 10.5]
    assert distributions.get_borders(polygon) == expected_borders


def test_distance_determination_point_line_segment():
    pt = (3, 3)
    acceptation_rate = 0.01
    pt1, pt2 = (0, 0), (4, 1)
    actual, expected = distributions.distance_to_segment(pt1, pt2, pt), 2.182820625326997
    differance = actual - expected
    assert abs(differance) < acceptation_rate
    pt1, pt2 = (4, 1), (5, 3)
    actual, expected = distributions.distance_to_segment(pt1, pt2, pt), 1.7888543819998317
    differance = actual - expected
    assert abs(differance) < acceptation_rate
    pt1, pt2 = (5, 3), (4, 5)
    actual, expected = distributions.distance_to_segment(pt1, pt2, pt), 1.7888543819998317
    differance = actual - expected
    assert abs(differance) < acceptation_rate
    pt1, pt2 = (4, 5), (2, 5)
    actual, expected = distributions.distance_to_segment(pt1, pt2, pt), 2.0
    differance = actual - expected
    assert abs(differance) < acceptation_rate
    pt1, pt2 = (2, 5), (0, 3)
    actual, expected = distributions.distance_to_segment(pt1, pt2, pt), 2.1213203435596424
    differance = actual - expected
    assert abs(differance) < acceptation_rate
    pt1, pt2 = (0, 3), (0, 0)
    actual, expected = distributions.distance_to_segment(pt1, pt2, pt), 3.0
    differance = actual - expected
    assert abs(differance) < acceptation_rate


def test_minimal_distance_to_polygon():
    polygon = [(0, 0), (4, 1), (5, 3), (4, 5), (2, 5), (0, 3)]
    pt = (3, 3)
    expected_result = min([2.182820625326997, 1.7888543819998317, 1.7888543819998317, 2.0, 2.1213203435596424, 3.0])
    acceptance_rate = 0.01
    actual_result = distributions.min_distance_to_polygon(pt, polygon)
    difference = actual_result - expected_result
    assert abs(difference) < acceptance_rate


def test_seed_works_correct_for_random_points():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    set_seed = 1337
    samples1 = distributions.create_random_points(polygon, 100, agent_radius=0.3, wall_distance=0.3, seed=set_seed)
    samples2 = distributions.create_random_points(polygon, 100, agent_radius=0.3, wall_distance=0.3, seed=set_seed)
    assert samples1 == samples2


def test_distance_determination():
    pt1 = (0, 0)
    pt2 = (2, 0)
    expected_result = 2.0
    exception_rate = 0.01
    actual_result = distributions.distance_between(pt1, pt2)
    difference = expected_result - actual_result
    assert abs(difference) < exception_rate
    pt1 = (0, 0)
    pt2 = (1, 1)
    expected_result = 1.41421
    exception_rate = 0.01
    actual_result = distributions.distance_between(pt1, pt2)
    difference = expected_result - actual_result
    assert abs(difference) < exception_rate


def test_if_inside():
    polygon1 = [(0, 0), (10, 0), (10, 10), (0, 10)]

    p = (20, 20)
    assert distributions.is_inside_polygon(points=polygon1, p=p) is False

    p = (5, 5)
    assert distributions.is_inside_polygon(points=polygon1, p=p) is True

    polygon2 = [(0, 0), (5, 0), (5, 5), (3, 3)]

    p = (3, 3)
    assert distributions.is_inside_polygon(points=polygon2, p=p) is True

    p = (5, 1)
    assert distributions.is_inside_polygon(points=polygon2, p=p) is True

    p = (8, 1)
    assert distributions.is_inside_polygon(points=polygon2, p=p) is False

    p = (-1, 10)
    assert distributions.is_inside_polygon(points=polygon1, p=p) is False

    polygon3 = [(1, 0), (2, 1)]
    p = (1.5, 0.5)
    assert distributions.is_inside_polygon(polygon3, p) is False


def test_if_intersect_correctly():
    p1, p2, p3, p4 = (0, 0), (2, 3), (0, 1), (3, 1)
    assert distributions.do_intersect(p1, p2, p3, p4) is True
    p1, p2, p3, p4 = (0, 0), (3, 0), (0, 1), (3, 1)
    assert distributions.do_intersect(p1, p2, p3, p4) is False
    p1, p2, p3, p4 = (1, 1), (2, 1), (0, 1), (3, 1)
    assert distributions.do_intersect(p1, p2, p3, p4) is True
    p1, p2, p3, p4 = (0, 1), (8, 1), (5, 5), (5, 5)
    assert distributions.do_intersect(p1, p2, p3, p4) is False


def test_orientation():
    p1, p2, p3 = (15, 0), (20, 10), (25, 20)
    assert distributions.orientation(p1, p2, p3) == 0  # collinear
    p1, p2, p3 = (0, 0), (20, 5), (10, 10)
    assert distributions.orientation(p1, p2, p3) == 2  # Counterclockwise
    p1, p2, p3 = (10, 10), (20, 5), (0, 0)
    assert distributions.orientation(p1, p2, p3) == 1  # Clockwise


def test_on_segment():
    p1, p2, p3 = (15, 0), (20, 10), (25, 20)
    assert distributions.on_segment(p1, p2, p3) is True
    p1, p2, p3 = (0, 0), (20, 5), (10, 10)
    assert distributions.on_segment(p1, p2, p3) is False


def test_placing_Circles():
    distribution = distributions.Distribution((0, 1))
    distribution.create_circle(0, 1, 2)
    assert distribution.mid_point == (0, 1)
    assert distribution.circles == [(0, 1, 2, None)]
    with pytest.raises(distributions.Overlapping):
        distribution.create_circle(0.5, 3, 5)
    with pytest.raises(distributions.NegativeNumber):
        distribution.create_circle(-1, -2, 5)
    distribution.create_circle(2, 3, 2)
    assert distribution.circles == [(0, 1, 2, None), (2, 3, 2, None)]
    with pytest.raises(distributions.Overlapping):
        distribution.create_circle(1.5, 2.5, 5)
    distribution.create_circle(1, 2, 2)
    assert distribution.circles == [(0, 1, 2, None), (2, 3, 2, None), (1, 2, 2, None)]
    distribution = distributions.Distribution((0, 1))
    distribution.create_circle(0, 1, density=1)
    assert distribution.circles == [(0, 1, None, 1)]
    with pytest.raises(distributions.Overlapping):
        distribution.create_circle(5, 3, 6)


def test_seed_works_correct_for_Circles():
    distibution = distributions.Distribution((5, 5))
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    set_seed = 1337
    distibution.create_circle(0, 5, number=5)
    samples1 = distibution.place_in_Polygon(polygon, agent_radius=0.3, wall_distance=0.3, seed=set_seed)
    samples2 = distibution.place_in_Polygon(polygon, agent_radius=0.3, wall_distance=0.3, seed=set_seed)
    assert samples2 is not []
    assert samples1 == samples2


def test_removing_Circles():
    distribution = distributions.Distribution("no mid point")
    distribution.circles = [(0, 1, 2, None), (2, 3, 2, None)]
    distribution.remove_circle(0, 1)
    assert distribution.circles == [(2, 3, 2, None)]
