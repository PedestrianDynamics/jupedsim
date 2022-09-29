from jupedsim import distributions
import pytest


class GridMock(distributions.Grid):
    def __init__(self):
        pass


def test_cell_coord_determination():
    mock = GridMock()
    pt = (5, 5)
    mock.box = [(0, 0), (10, 10)]
    mock.c_s_l = 0.7071067811865475  # 1/√2
    assert (7, 7) == mock.get_cell_coords(pt)


def test_grid_creation():
    box = [(0, 0), (3, 3)]
    agent_radius = 0.3
    grid = distributions.Grid(box, agent_radius)
    acceptance_rate = 0.01
    assert grid.box == box
    assert abs(grid.c_s_l - 0.21213203435596423) < acceptance_rate
    assert grid.nx == 15
    assert grid.ny == 15
    assert grid.coords_list
    assert grid.cells


def test_neighbour_determination():
    mock = GridMock()
    mock.a_r = 1.414213562
    mock.nx, mock.ny = 15, 15
    mock.coords_list = [(ix, iy) for ix in range(mock.nx) for iy in range(mock.ny)]
    mock.cells = {coords: None for coords in mock.coords_list}
    for i in range(15):
        mock.cells[(i, i)] = (i+0.5, i+0.5)
    assert mock.has_neighbour_in_distance((7, 7), (7, 7)) is True
    assert mock.has_neighbour_in_distance((10, 0), (10, 0)) is False


def test_point_in_circle():
    mid = (0, 0)
    min_radius, max_radius = 5, 10
    test_point = (2, 2)
    assert distributions.is_inside_circle(test_point, mid, min_radius, max_radius) is False
    test_point = (10, 10)
    assert distributions.is_inside_circle(test_point, mid, min_radius, max_radius) is False
    test_point = (7, 7)
    assert distributions.is_inside_circle(test_point, mid, min_radius, max_radius) is True


def test_bounding_box_determination():
    polygon = [(6, 0), (9, 2), (11, 4), (12, 7), (11.5, 9.5), (9.5, 10.5), (7.5, 10),
               (6, 9), (4.5, 10), (2.5, 10.5), (0.6, 9.5), (0, 7), (1, 4), (3, 2)]
    s_polygon = distributions.shply.Polygon(polygon)
    expected_box = [(0, 0), (12, 10.5)]
    assert distributions.get_bounding_box(s_polygon) == expected_box


def test_minimal_distance_to_polygon():
    polygon = distributions.shply.Polygon([(0, 0), (4, 1), (5, 3), (4, 5), (2, 5), (0, 3)])
    pt = (3, 3)
    expected_result = min([2.182820625326997, 1.7888543819998317, 1.7888543819998317, 2.0, 2.1213203435596424, 3.0])
    acceptance_rate = 0.01
    actual_result = distributions.min_distance_to_polygon(pt, polygon)
    difference = actual_result - expected_result
    assert abs(difference) < acceptance_rate


def test_seed_works_correct_for_determination_by_number():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    polygon = distributions.shply.Polygon(polygon)
    agent_radius, wall_distance = 0.3, 0.3
    set_seed = 1337
    samples1 = distributions.distribute_by_number(polygon, 100, agent_radius, wall_distance, seed=set_seed)
    samples2 = distributions.distribute_by_number(polygon, 100, agent_radius, wall_distance, seed=set_seed)
    assert samples1 == samples2


def test_seed_works_correct_for_distribution_in_circle_by_number():
    polygon = distributions.shply.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    number_of_agents = [5]
    agent_distance = 0.3
    distace_to_polygon = 0.3
    set_seed = 1337
    center_point = (5, 5)
    circle_segment_radii = [(0, 5)]

    samples1 = distributions.distribute_in_circles_by_number(polygon=polygon, agent_distance=agent_distance,
                                                             distance_to_polygon=distace_to_polygon,
                                                             center_point=center_point,
                                                             circle_segment_radii=circle_segment_radii,
                                                             numbers_of_agents=number_of_agents,
                                                             seed=set_seed, max_iterations=10_000)
    samples2 = distributions.distribute_in_circles_by_number(polygon=polygon, agent_distance=agent_distance,
                                                             distance_to_polygon=distace_to_polygon,
                                                             center_point=center_point,
                                                             circle_segment_radii=circle_segment_radii,
                                                             numbers_of_agents=number_of_agents,
                                                             seed=set_seed, max_iterations=10_000)

    assert samples1 == samples2
