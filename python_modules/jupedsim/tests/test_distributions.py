# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import pytest

from jupedsim import distributions


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
    distance_to_agents = 0.3
    grid = distributions.Grid(box, distance_to_agents)
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
    mock.coords_list = [
        (ix, iy) for ix in range(mock.nx) for iy in range(mock.ny)
    ]
    mock.cells = {coords: None for coords in mock.coords_list}
    for i in range(15):
        mock.cells[(i, i)] = (i + 0.5, i + 0.5)
    assert mock.has_neighbour_in_distance((7, 7), (7, 7)) is True
    assert mock.has_neighbour_in_distance((10, 0), (10, 0)) is False


def test_point_in_circle():
    mid = (0, 0)
    min_radius, max_radius = 5, 10
    test_point = (2, 2)
    assert (
        distributions.__is_inside_circle(
            test_point, mid, min_radius, max_radius
        )
        is False
    )
    test_point = (10, 10)
    assert (
        distributions.__is_inside_circle(
            test_point, mid, min_radius, max_radius
        )
        is False
    )
    test_point = (7, 7)
    assert (
        distributions.__is_inside_circle(
            test_point, mid, min_radius, max_radius
        )
        is True
    )


def test_bounding_box_determination():
    polygon = [
        (6, 0),
        (9, 2),
        (11, 4),
        (12, 7),
        (11.5, 9.5),
        (9.5, 10.5),
        (7.5, 10),
        (6, 9),
        (4.5, 10),
        (2.5, 10.5),
        (0.6, 9.5),
        (0, 7),
        (1, 4),
        (3, 2),
    ]
    s_polygon = distributions.shply.Polygon(polygon)
    expected_box = [(0, 0), (12, 10.5)]
    assert distributions.__get_bounding_box(s_polygon) == expected_box


def test_minimal_distance_to_polygon():
    polygon = distributions.shply.Polygon(
        [(0, 0), (4, 1), (5, 3), (4, 5), (2, 5), (0, 3)]
    )
    pt = (3, 3)
    expected_result = min(
        [
            2.182820625326997,
            1.7888543819998317,
            1.7888543819998317,
            2.0,
            2.1213203435596424,
            3.0,
        ]
    )
    acceptance_rate = 0.01
    actual_result = distributions.__min_distance_to_polygon(pt, polygon)
    difference = actual_result - expected_result
    assert abs(difference) < acceptance_rate


def test_seed_works_correct_for_determination_by_number():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    polygon = distributions.shply.Polygon(polygon)
    distance_to_agents, distance_to_polygon = 0.3, 0.3
    number_of_agents = 100
    set_seed = 1337
    samples1 = distributions.distribute_by_number(
        polygon=polygon,
        number_of_agents=number_of_agents,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
    )
    samples2 = distributions.distribute_by_number(
        polygon=polygon,
        number_of_agents=number_of_agents,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
    )
    assert samples1 == samples2


def test_seed_works_correct_for_determination_in_circles_by_number():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    polygon = distributions.shply.Polygon(polygon)
    number_of_agents = [75]
    distance_to_agents = 0.3
    distance_to_polygon = 0.3
    set_seed = 1337
    center_point = (5, 5)
    circle_segment_radii = [(0, 5)]

    samples1 = distributions.distribute_in_circles_by_number(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        center_point=center_point,
        circle_segment_radii=circle_segment_radii,
        numbers_of_agents=number_of_agents,
        seed=set_seed,
        max_iterations=10_000,
    )
    samples2 = distributions.distribute_in_circles_by_number(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        center_point=center_point,
        circle_segment_radii=circle_segment_radii,
        numbers_of_agents=number_of_agents,
        seed=set_seed,
        max_iterations=10_000,
    )
    assert samples1 == samples2


def test_determination_by_number_creates_correct_points():
    polygon = [
        (0, 2),
        (3, 3),
        (3, 5),
        (6, 5),
        (6, 0),
        (14, 0),
        (15, 15),
        (9, 15),
        (6, 10),
        (4, 12.5),
        (0, 7.5),
    ]
    holes = [
        [(8.5, 4), (10, 3), (12, 6.5), (10, 7)],
        [(10, 10.5), (11, 9.5), (14, 12.5), (11, 13)],
    ]
    polygon = distributions.shply.Polygon(polygon, holes)
    distance_to_agents, distance_to_polygon = 0.3, 0.3
    number_of_agents = 450
    set_seed = 1337
    samples = distributions.distribute_by_number(
        polygon=polygon,
        number_of_agents=number_of_agents,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
    )
    # as many points created as intended
    assert len(samples) == number_of_agents

    # all created points contained inside polygon
    for sample in samples:
        assert polygon.contains(distributions.shply.Point(sample))

    # all Points have enough distance to another
    for i, sample in enumerate(samples):
        j = 0
        while j < len(samples):
            if i == j:
                j = j + 1
                continue
            dif_x = sample[0] - samples[j][0]
            dif_y = sample[1] - samples[j][1]
            distance = math.sqrt(dif_x**2 + dif_y**2)
            assert distance >= distance_to_agents
            j = j + 1


def test_determination_by_density_creates_correct_amount():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    # polygon 10 x 10 square with 100m²
    polygon = distributions.shply.Polygon(polygon)
    distance_to_agents, distance_to_polygon = 0.3, 0.3
    density = 2.5
    # 2.5 persons per m²
    set_seed = 1337
    samples = distributions.distribute_by_density(
        polygon=polygon,
        density=density,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
    )
    # as many points created as intended
    assert len(samples) == 250


def test_distribution_in_circle_by_number_creates_correct_points():
    polygon = [
        (0, 2),
        (3, 3),
        (3, 5),
        (6, 5),
        (6, 0),
        (14, 0),
        (15, 15),
        (9, 15),
        (6, 10),
        (4, 12.5),
        (0, 7.5),
    ]
    holes = [
        [(8.5, 4), (10, 3), (12, 6.5), (10, 7)],
        [(10, 10.5), (11, 9.5), (14, 12.5), (11, 13)],
    ]
    polygon = distributions.shply.Polygon(polygon, holes)
    number_of_agents = [200, 150]
    distance_to_agents = 0.3
    distance_to_polygon = 0.3
    set_seed = 1337
    center_point = (7.5, 7.5)
    circle_segment_radii = [(0, 5), (6, 7.5)]

    samples = distributions.distribute_in_circles_by_number(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        center_point=center_point,
        circle_segment_radii=circle_segment_radii,
        numbers_of_agents=number_of_agents,
        seed=set_seed,
        max_iterations=10_000,
    )
    # as many points created as intended
    assert len(samples) == sum(number_of_agents)
    i = 0

    # all created points inside their corresponding circle segment
    while i < number_of_agents[0]:
        assert distributions.__is_inside_circle(
            samples[i],
            center_point,
            circle_segment_radii[0][0],
            circle_segment_radii[0][1],
        )
        i = i + 1
    while i < number_of_agents[0] + number_of_agents[1]:
        assert distributions.__is_inside_circle(
            samples[i],
            center_point,
            circle_segment_radii[1][0],
            circle_segment_radii[1][1],
        )
        i = i + 1

    # all created points contained inside polygon
    for sample in samples:
        assert polygon.contains(distributions.shply.Point(sample))

    # all Points have enough distance to another
    for i, sample in enumerate(samples):
        j = 0
        while j < len(samples):
            if i == j:
                j = j + 1
                continue
            dif_x = sample[0] - samples[j][0]
            dif_y = sample[1] - samples[j][1]
            distance = math.sqrt(dif_x**2 + dif_y**2)
            assert distance >= distance_to_agents
            j = j + 1


def test_distribution_in_circle_by_density_creates_correct_amount():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    # polygon 10 x 10 square with 100m²
    polygon = distributions.shply.Polygon(polygon)
    densities = [1]
    distance_to_agents = 0.3
    distance_to_polygon = 0.3
    set_seed = 1337
    center_point = (5, 5)
    circle_segment_radii = [(0, 5)]

    samples = distributions.distribute_in_circles_by_density(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        center_point=center_point,
        circle_segment_radii=circle_segment_radii,
        densities=densities,
        seed=set_seed,
        max_iterations=10_000,
    )
    # as many points created as intended
    # int ( π * 5² ) = 78
    assert len(samples) == 78


def test_seed_works_correct_for_distribution_till_full():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    polygon = distributions.shply.Polygon(polygon)
    distance_to_agents, distance_to_polygon = 0.75, 0.75
    set_seed = 1337
    samples1 = distributions.distribute_until_filled(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
    )
    samples2 = distributions.distribute_until_filled(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
        max_iterations=750,
    )
    assert samples1 == samples2


def test_distribution_by_percentage_creates_correct_points():
    polygon = [
        (0, 2),
        (3, 3),
        (3, 5),
        (6, 5),
        (6, 0),
        (14, 0),
        (15, 15),
        (9, 15),
        (6, 10),
        (4, 12.5),
        (0, 7.5),
    ]
    holes = [
        [(8.5, 4), (10, 3), (12, 6.5), (10, 7)],
        [(10, 10.5), (11, 9.5), (14, 12.5), (11, 13)],
    ]
    polygon = distributions.shply.Polygon(polygon, holes)
    percent = 73
    distance_to_agents = 2.0
    distance_to_polygon = 1.0
    set_seed = 1337
    samples = distributions.distribute_by_percentage(
        polygon=polygon,
        percent=percent,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
        max_iterations=750,
    )
    # as many points created as intended
    assert len(samples) == round(16 * (percent / 100))

    # all created points contained inside polygon
    for sample in samples:
        assert polygon.contains(distributions.shply.Point(sample))

    # all Points have enough distance to another
    for i, sample in enumerate(samples):
        j = 0
        while j < len(samples):
            if i == j:
                j = j + 1
                continue
            dif_x = sample[0] - samples[j][0]
            dif_y = sample[1] - samples[j][1]
            distance = math.sqrt(dif_x**2 + dif_y**2)
            assert distance >= distance_to_agents
            j = j + 1


def test_seed_works_correct_for_distribution_by_percentage():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    polygon = distributions.shply.Polygon(polygon)
    distance_to_agents, distance_to_polygon = 0.75, 0.75
    set_seed = 1337
    percent = 73
    samples1 = distributions.distribute_by_percentage(
        polygon=polygon,
        percent=percent,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
        max_iterations=750,
    )
    samples2 = distributions.distribute_by_percentage(
        polygon=polygon,
        percent=percent,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
        max_iterations=750,
    )
    assert samples1 == samples2


def test_box_of_intersection():
    polygon = [
        (0, 0),
        (8, 0),
        (8, 2),
        (2, 2),
        (2, 4),
        (6, 4),
        (6, 6),
        (2, 6),
        (2, 8),
        (8, 8),
        (8, 10),
        (0, 10),
    ]
    s_polygon = distributions.shply.Polygon(polygon)
    center_point = (4, 7)
    outer_radius = 3.5
    expected_box = [(0.5, 4), (7.35329, 10)]
    actual_box = distributions.__box_of_intersection(
        s_polygon, center_point, outer_radius
    )
    exception_rate = 0.01

    assert abs(expected_box[0][0] - actual_box[0][0]) < exception_rate
    assert abs(expected_box[0][1] - actual_box[0][1]) < exception_rate
    assert abs(expected_box[1][0] - actual_box[1][0]) < exception_rate
    assert abs(expected_box[1][1] - actual_box[1][1]) < exception_rate


def test_catch_wrong_inputs_for_wrong_polygon_type():
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    center_point = (0, 0)
    circle_segment_radii, numbers_of_agents = [(1, 3)], [5]
    # polygon is no Shapely Polygon
    with pytest.raises(distributions.IncorrectParameterError):
        distributions.__catch_wrong_inputs(
            polygon=polygon,
            center_point=center_point,
            circle_segment_radii=circle_segment_radii,
            fill_parameters=numbers_of_agents,
        )


def test_catch_wrong_inputs_negativ_value():
    s_polygon = distributions.shply.Polygon(
        [(0, 0), (10, 0), (10, 10), (0, 10)]
    )
    center_point = (0, 0)
    circle_segment_radii = [(-1, 3), (3, 5)]
    numbers_of_agents = [5, 5]
    with pytest.raises(distributions.NegativeValueError):
        distributions.__catch_wrong_inputs(
            polygon=s_polygon,
            center_point=center_point,
            circle_segment_radii=circle_segment_radii,
            fill_parameters=numbers_of_agents,
        )


test_parameters = [
    [[(0, 0), (10, 0), (10, 10), (0, 10)], 5, [(1, 3)], [5]],
    [[(0, 0), (10, 0), (10, 10), (0, 10)], [0], [(1, 3)], [5]],
    [[(0, 0), (10, 0), (10, 10), (0, 10)], (0, 0), [(1, 3), (3, 5)], [5]],
]


@pytest.mark.parametrize(
    "polygon, center_point, circle_segment_radii, numbers_of_agents",
    test_parameters,
)
def test_catch_wrong_inputs_incorrect_parameter(
    polygon, center_point, circle_segment_radii, numbers_of_agents
):
    s_polygon = distributions.shply.Polygon(polygon)
    with pytest.raises(distributions.IncorrectParameterError):
        distributions.__catch_wrong_inputs(
            polygon=s_polygon,
            center_point=center_point,
            circle_segment_radii=circle_segment_radii,
            fill_parameters=numbers_of_agents,
        )


test_parameters = [
    [[(0, 0), (10, 0), (10, 10), (0, 10)], (0, 0), [(1, 3), (2, 5)], [5, 5]],
    [
        [(0, 0), (10, 0), (10, 10), (0, 10)],
        (0, 0),
        [(0, 2), (3, 5), (2, 4)],
        [5, 5, 5],
    ],
    [[(0, 0), (10, 0), (10, 10), (0, 10)], (0, 0), [(0, 5), (8, 3)], [5, 5]],
]


@pytest.mark.parametrize(
    "polygon, center_point, circle_segment_radii, numbers_of_agents",
    test_parameters,
)
def test_catch_wrong_inputs_overlapping_circles(
    polygon, center_point, circle_segment_radii, numbers_of_agents
):
    s_polygon = distributions.shply.Polygon(polygon)
    with pytest.raises(distributions.OverlappingCirclesError):
        distributions.__catch_wrong_inputs(
            polygon=s_polygon,
            center_point=center_point,
            circle_segment_radii=circle_segment_radii,
            fill_parameters=numbers_of_agents,
        )


test_parameters = [
    [
        [
            (0, 2),
            (3, 3),
            (3, 5),
            (6, 5),
            (6, 0),
            (14, 0),
            (15, 15),
            (9, 15),
            (6, 10),
            (4, 12.5),
            (0, 7.5),
        ],
        [
            [(8.5, 4), (10, 3), (12, 6.5), (10, 7)],
            [(10, 10.5), (11, 9.5), (14, 12.5), (11, 13)],
        ],
        2.0,
        1.0,
        16,
    ],
    [[(0, 0), (1, 0), (1, 1), (0, 1)], [], 2.0, 0.1, 1],
    [
        [
            (1, 0),
            (9, 0),
            (6.5, 5),
            (8.5, 5),
            (6.5, 10),
            (7.5, 10),
            (5, 15),
            (2.5, 10),
            (3.5, 10),
            (1.5, 5),
            (3.5, 5),
        ],
        [],
        1.0,
        0.2,
        44,
    ],
]


@pytest.mark.parametrize(
    "polygon, holes, distance_to_agents, distance_to_polygon, expected_size",
    test_parameters,
)
def test_distribution_till_full_creates_correct_points(
    polygon, holes, distance_to_agents, distance_to_polygon, expected_size
):
    polygon = distributions.shply.Polygon(polygon, holes)
    set_seed = 1337
    samples = distributions.distribute_until_filled(
        polygon=polygon,
        distance_to_agents=distance_to_agents,
        distance_to_polygon=distance_to_polygon,
        seed=set_seed,
        max_iterations=750,
    )
    # as many points created as intended
    assert len(samples) == expected_size

    # all created points contained inside polygon
    for sample in samples:
        assert polygon.contains(distributions.shply.Point(sample))

    # all Points have enough distance to another
    for i, sample in enumerate(samples):
        j = 0
        while j < len(samples):
            if i == j:
                j = j + 1
                continue
            dif_x = sample[0] - samples[j][0]
            dif_y = sample[1] - samples[j][1]
            distance = math.sqrt(dif_x**2 + dif_y**2)
            assert distance >= distance_to_agents
            j = j + 1
