import pathlib
import platform

import numpy
import pytest
from driver.driver import JpsCoreDriver
from driver.environment import Platform
from driver.events import read_starting_times
from fixtures import env
from driver.flow import check_flow
from driver.geometry import get_intersetions_path_segment
from driver.inifile import (
    instanciate_tempalte,
    parse_traffic_constraints,
    parse_waiting_areas,
)
from driver.trajectories import load_trajectory
from driver.utils import (
    copy_all_files,
    copy_files,
    get_file_text_diff,
    pairwise,
    setup_jpscore_driver,
)
from sympy.geometry import Point, Segment


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="No reference data for Windows available",
)
@pytest.mark.parametrize(
    "test_directory",
    [
        (pathlib.Path("reference_tests/RT01_corridor_GCFM_global-shortest/")),
        (
            pathlib.Path(
                "reference_tests/RT02_corridor_velocity_global-shortest/"
            )
        ),
    ],
)
def test_reference_data(tmp_path, env, test_directory: pathlib.Path):
    """
    Ensures that the output of jpscore has not changed.

    For this purpose reference trajectory files for Linux/Mac are compared with the actual generated trajectory files.
    The tests passes if there is no difference in the expected and actual trajectory files.
    The test is skipped for Windows.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param test_directory: directory of the test
    """
    jpscore_driver = setup_jpscore_driver(
        env=env, working_directory=tmp_path, test_directory=test_directory
    )
    jpscore_driver.run()

    # check if there is no diff between expected and new output
    # use different files for different OS
    expected = None
    if env.operating_system == Platform.LINUX:
        expected = (
            env.systemtest_path / test_directory / "expected_linux/traj.txt"
        )
    elif env.operating_system == Platform.MACOS:
        expected = (
            env.systemtest_path / test_directory / "expected_mac/traj.txt"
        )
    assert expected

    actual = jpscore_driver.traj_file
    diff = get_file_text_diff(
        expected=expected,
        actual=actual,
    )
    # diff must be empty
    assert diff == ""


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "test_directory, expected_evac_time, tolerance",
    [
        (
            pathlib.Path("juelich_tests/evacuation_time/single_ped_corridor"),
            10.0,
            0.5,
        )
    ],
)
def test_evac_time_single_ped(
    tmp_path,
    env,
    test_directory: pathlib.Path,
    expected_evac_time: float,
    tolerance: float,
):
    """
    Ensures that the evacuation for a single pedestrian in free flow lies within an accepted range.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param test_directory: directory of the test
    :param expected_evac_time: expected evacuation time
    :param tolerance: tolerated difference to actual evacuation time
    """
    jpscore_driver = setup_jpscore_driver(
        env=env, working_directory=tmp_path, test_directory=test_directory
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)
    actual_evac_time = (
        max(trajectories.data[:, 1]) - min(trajectories.data[:, 1])
    ) / trajectories.framerate
    diff_evac_times = abs(actual_evac_time - expected_evac_time)

    # difference in evac times must be in tolerance range
    assert diff_evac_times < tolerance


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "test_directory",
    [
        pathlib.Path("waiting_area_tests/routing_room_test"),
        pathlib.Path("waiting_area_tests/routing_subroom_test"),
    ],
)
def test_waiting_area_routing_room(tmp_path, env, test_directory):
    """
    In this test case 2 pedestrians walk to the exit. The first pedestrian
    is taking the direct route, bypassing all waiting areas. The second
    pedestrian is waiting at each waiting area for a moment.

    This test is parametrized with the geometry type used, room bases vs
    sub-room based construction.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param test_directory: directory of the test
    """
    jpscore_driver = setup_jpscore_driver(
        env=env,
        working_directory=tmp_path,
        test_directory=test_directory,
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)
    waiting_areas = parse_waiting_areas(
        env.systemtest_path / test_directory / "input/inifile.xml"
    )

    direct_path = trajectories.path(3)
    indirect_path = trajectories.path(4)

    for area in waiting_areas:
        if any([area.encloses_point(Point(e[2], e[3])) for e in direct_path]):
            pytest.fail(
                "Pedestrian ID=3 did pass trough a waiting area. This should not have happened."
            )
        if not any(
            [area.encloses_point(Point(e[2], e[3])) for e in indirect_path]
        ):
            pytest.fail(
                "Pedestrian ID=4 did not pass trough a waiting area. This should not have happened."
            )


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_train_feature_basic_functionality(tmp_path, env):
    """
    Simple scenario with pedestrians heading for the train. It is expected that
    all agents have left the simulation within 50 seconds.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "train_tests" / "simple_train_test"
    copy_all_files(src=input_location, dest=tmp_path)
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)
    assert trajectories.runtime() <= 50.0


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_train_capacity_feature(tmp_path, env):
    """
    The first train has a capacity of 15, the second train of 20. At the end of
    the simulation only 15 agents should remain.

    BUG: Currently the capacity can be exceeded if pedestrians enter in the
    same frame, hence the remaining pedestrians are treated as an upper bound.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = (
        env.systemtest_path / "train_tests" / "train_test_max_agents"
    )
    copy_all_files(src=input_location, dest=tmp_path)
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)

    # Check agent count after the first train left
    first_train_leaving_index = int(trajectories.framerate * 15)
    assert trajectories.agent_count_in_frame(first_train_leaving_index) <= 35

    # Check agent count after the second train left
    second_train_leaving_index = int(trajectories.framerate * 30)
    assert trajectories.agent_count_in_frame(second_train_leaving_index) <= 15

    # Check agent count at the end of the simulation
    last_frame_index = trajectories.frame_count() - 1
    assert trajectories.agent_count_in_frame(last_frame_index) <= 15


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_juelich_1_free_flow_movement_in_corridor(tmp_path, env):
    """
    A pedestrian that starts in the middle of a corridor (i.e. is not
    influenced by the walls) should move with its free flow velocity towards
    the exit.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_1"
    copy_all_files(src=input_location, dest=tmp_path)
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    trajectories = load_trajectory(jpscore_driver.traj_file)
    expected_evac_time = 10.0
    assert trajectories.runtime() <= expected_evac_time


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "operational_model_id",
    [
        1,
        3,
    ],
)
def test_juelich_2_single_pedestrian_moving_in_a_corridor(
    tmp_path, env, operational_model_id
):
    """
    Rotating the same geometry as in test 1 around the z−axis by an arbitrary angle e.g.
    45deg should lead to the evacuation time of 10 s.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param operational_model_id this test is parametrized for two models, the
           ids have to match the model ids in the template file
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_2"
    template_path = input_location / "inifile.template"
    inifile_path = tmp_path / "inifile.xml"
    instanciate_tempalte(
        src=template_path,
        args={"operational_model_id": operational_model_id},
        dest=inifile_path,
    )

    copy_files(sources=[input_location / "geometry.xml"], dest=tmp_path)
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    trajectories = load_trajectory(jpscore_driver.traj_file)
    agent_path = trajectories.path(2)

    d_x = max(agent_path[:, 2]) - min(agent_path[:, 2])
    d_y = max(agent_path[:, 3]) - min(agent_path[:, 3])
    beeline_distance = numpy.sqrt(d_x ** 2 + d_y ** 2)
    v_expected = 1.0
    time_limit = (beeline_distance / v_expected) + 0.1
    assert trajectories.runtime() <= time_limit


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "exit_crossing_strategy",
    [
        1,
        3,
    ],
)
@pytest.mark.parametrize(
    "seed",
    [
        1.0,
        527.2631578947369,
        1053.5263157894738,
        1579.7894736842106,
        2106.0526315789475,
        2632.315789473684,
        3158.5789473684213,
        3684.8421052631584,
        4211.105263157895,
        4737.368421052632,
        5263.631578947368,
        5789.894736842106,
        6316.1578947368425,
        6842.421052631579,
        7368.684210526317,
        7894.947368421053,
        8421.21052631579,
        8947.473684210527,
        9473.736842105263,
        10000.0,
    ],
)
def test_juelich_3_single_pedestrian_moving_in_a_corridor_with_a_desired_direction(
    tmp_path, env, seed, exit_crossing_strategy
):
    """
    A pedestrian is started from a random position in a holding area. This test
    should be repeated with different initial positions. Expected result: The
    pedestrians should be able to reach the marked goal in all repetitions of
    the test.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param seed: value to use for the simulation
    :param exit_crossing_strategy: exit strategy id to use
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_3"
    template_path = input_location / "inifile.template"
    inifile_path = tmp_path / "inifile.xml"
    instanciate_tempalte(
        src=template_path,
        args={"seed": seed, "exit_crossing_strategy": exit_crossing_strategy},
        dest=inifile_path,
    )

    copy_files(sources=[input_location / "geometry.xml"], dest=tmp_path)
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    trajectories = load_trajectory(jpscore_driver.traj_file)
    agent_path = trajectories.path(2)

    assert trajectories.runtime() < 10.0
    assert numpy.any(agent_path[:, 2] > 8.5)


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "operational_model_id",
    [
        1,
        3,
    ],
)
def test_juelich_4_single_pedestrian_moving_in_a_corridor_with_obstacle(
    tmp_path, env, operational_model_id
):
    """
    Two pedestrians are aligned in the same room. The second pedestrian from left is standing and will not move during the test.
    Expected result: Pedestrian left should be able to overtake the standing pedestrian.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    :param operational_model_id this test is parametrized for two models, the
           ids have to match the model ids in the template file
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_4"
    template_path = input_location / "inifile.template"
    inifile_path = tmp_path / "inifile.xml"
    instanciate_tempalte(
        src=template_path,
        args={"operational_model_id": operational_model_id},
        dest=inifile_path,
    )

    copy_files(sources=[input_location / "geometry.xml"], dest=tmp_path)
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    trajectories = load_trajectory(jpscore_driver.traj_file)

    agent_path3 = trajectories.path(3)
    agent_path4 = trajectories.path(4)

    x_1 = agent_path3[:, 2]
    y_1 = agent_path3[:, 3]

    x_2 = agent_path4[:, 2]
    y_2 = agent_path4[:, 3]

    eps = 0.3  # 10 cm
    x_min = x_2[0] - eps
    x_max = x_2[0] + eps
    y_min = y_2[0] - eps
    y_max = y_2[0] + eps

    lx = numpy.logical_and(x_1 > x_min, x_1 < x_max)
    ly = numpy.logical_and(y_1 > y_min, y_1 < y_max)

    overlap = (lx * ly).any()
    assert not overlap


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_juelich_5_single_pedestrian_moving_in_a_very_narrow_corridor_with_an_obstacle(
    tmp_path, env
):
    """
    This test is Similar to test 4. Two pedestrians are aligned in the same
    room. The second pedestrian from left is standing and will not move during
    the test. The corridor is narrow and does not allow passing of two
    pedestians without serious overlapping.

    Expected result: Pedestrian left should stop without overlapping with the
    standing pedestrian.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_5"
    copy_files(
        sources=[
            input_location / "geometry.xml",
            input_location / "inifile.xml",
        ],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    trajectories = load_trajectory(jpscore_driver.traj_file)
    immobile_agent_path = trajectories.path(4)
    agent_path = trajectories.path(3)

    distances = numpy.sqrt(
        (agent_path[:, 2] - immobile_agent_path[:, 2]) ** 2
        + (agent_path[:, 3] - immobile_agent_path[:, 3]) ** 2
    )

    assert numpy.all(distances >= 0.4)


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_juelich_6_single_pedestrian_moving_in_a_corridor_with_more_than_one_target(
    tmp_path, env
):
    """
    A pedestrian is moving in a corridor with several intermediate goals.

    Expected result: The pedestrian should move through the different targets
    without a substantial change in its velocity i.e. with a desired speed of 1
    m/s the distance of 10 m should be covered in 10 s.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_6"
    copy_files(
        sources=[
            input_location / "geometry.xml",
            input_location / "inifile.xml",
        ],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    trajectories = load_trajectory(jpscore_driver.traj_file)
    assert trajectories.runtime() <= 10.1
    assert trajectories.runtime() >= 9.0


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "operational_model_id",
    [
        1,
        3,
    ],
)
def test_juelich_8_obstacle_avoidance(tmp_path, env, operational_model_id):
    """
    Expected result: The pedestrian should avoid the obstacle and exit the room
    without overlapping with the obstacle.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_8"
    template_path = input_location / "inifile.template"
    inifile_path = tmp_path / "inifile.xml"
    instanciate_tempalte(
        src=template_path,
        args={"operational_model_id": operational_model_id},
        dest=inifile_path,
    )
    copy_files(
        sources=[input_location / "geometry.xml"],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()
    trajectories = load_trajectory(jpscore_driver.traj_file)
    assert trajectories.runtime() <= 45.0


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "operational_model_id",
    [
        1,
        3,
    ],
)
def test_juelich_11_geo_room_subroom_structure(
    tmp_path, env, operational_model_id
):
    """
    The same geometry is constructed differently The whole geometry is designed
    as a rooms (i.e. utility space) The geometry is designed by dividing the
    utility space in connected subrooms. Distribute randomly pedestrians in all
    sub-rooms of the geometry and repeat the simulation to get a certain
    statistical significance.
    Expected results: The mean value of the evacuation times calculated from
    both cases should not differ.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_11"
    template_path_a = input_location / "inifile_a.template"
    template_path_b = input_location / "inifile_b.template"
    tmp_path_a = tmp_path / "a"
    tmp_path_b = tmp_path / "b"
    tmp_path_a.mkdir()
    tmp_path_b.mkdir()

    inifile_path_a = tmp_path_a / "inifile.xml"
    instanciate_tempalte(
        src=template_path_a,
        args={"operational_model_id": operational_model_id},
        dest=inifile_path_a,
    )
    copy_files(
        sources=[input_location / "geometry_a.xml"],
        dest=tmp_path_a,
    )

    inifile_path_b = tmp_path_b / "inifile.xml"
    instanciate_tempalte(
        src=template_path_b,
        args={"operational_model_id": operational_model_id},
        dest=inifile_path_b,
    )
    copy_files(
        sources=[input_location / "geometry_b.xml"],
        dest=tmp_path_b,
    )

    jpscore_driver_a = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path_a
    )
    jpscore_driver_a.run()

    jpscore_driver_b = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path_b
    )
    jpscore_driver_b.run()

    trajectories_a = load_trajectory(jpscore_driver_a.traj_file)
    trajectories_b = load_trajectory(jpscore_driver_b.traj_file)

    assert numpy.allclose(trajectories_a.data, trajectories_b.data, 0.001)


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "operational_model_id",
    [
        1,
        3,
    ],
)
def test_juelich_12_obstructed_visibility(tmp_path, env, operational_model_id):
    """
    Four pedestrians being simulated in a bottleneck. Pedestrians 0 and 1 have
    zero desired speed i.e. they will not move during the simulation whereas
    pedestrians 2 and 3 are heading towards the exit.
    The visibility between pedestrians 2 resp. 3 and 0 resp. 2 is obstructed
    by a wall resp. an obstacle.
    Expected results: Pedestrians 2 and 3 should not deviate from
    the horizontal dashed line.

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "juelich_tests" / "test_12"
    template_path = input_location / "inifile.template"
    inifile_path = tmp_path / "inifile.xml"
    instanciate_tempalte(
        src=template_path,
        args={"operational_model_id": operational_model_id},
        dest=inifile_path,
    )
    copy_files(
        sources=[input_location / "geometry.xml"],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)
    agent_path = trajectories.path(2)
    y = agent_path[:, 3]
    dy = numpy.sum(numpy.abs(numpy.diff(y)))
    tolerance = 0.005
    assert dy < tolerance


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
@pytest.mark.parametrize(
    "router_id",
    [
        1,
        2,
    ],
)
def test_router_corridor_close(tmp_path, env, router_id):
    """

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = (
        env.systemtest_path / "router_tests" / "test_corridor_close"
    )
    template_path = input_location / "inifile.template"
    inifile_path = tmp_path / "inifile.xml"
    instanciate_tempalte(
        src=template_path,
        args={"router_id": router_id},
        dest=inifile_path,
    )
    copy_files(
        sources=[input_location / "geometry.xml"],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)
    agent_path = trajectories.path(2)
    assert get_intersetions_path_segment(
        agent_path, Segment(Point(9.5, -5), Point(9.5, 5))
    )


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_router_10(tmp_path, env):
    """

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "router_tests" / "test_router_10"
    copy_files(
        sources=[
            input_location / "geometry.xml",
            input_location / "inifile.xml",
        ],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()

    trajectories = load_trajectory(jpscore_driver.traj_file)
    agent_path = trajectories.path(2)
    assert get_intersetions_path_segment(
        agent_path, Segment(Point(90.1, -104), Point(90.1, -102))
    )


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_door_closes_after_max_agents(tmp_path, env):
    """

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = env.systemtest_path / "door_tests" / "closed_doors"
    copy_files(
        sources=[
            input_location / "geometry.xml",
            input_location / "inifile.xml",
        ],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()

    flow_dict = {1: 0, 2: 0}
    # here the flow through the doors is computed.
    # The two doors are parallel to the X-axis, therefore the computation of the door taken by each pedestrian is simplyfied.
    trajectories = load_trajectory(jpscore_driver.traj_file)
    for ped_id in numpy.unique(trajectories.data[:, 0]):
        path = trajectories.path(ped_id)
        for p1, p2 in pairwise(path):
            if p1[3] < 30 and p2[3] >= 30:
                if 20 <= p1[2] <= 22:
                    flow_dict[1] += 1
                if 28 <= p1[2] <= 30:
                    flow_dict[2] += 1

    assert flow_dict[1] >= 80
    assert flow_dict[2] == 20


@pytest.mark.skip(reason="Not yet migrated to new exit / room handling")
def test_door_flow_regulation(tmp_path, env):
    """

    :param tmp_path: working directory of test execution
    :param env: global environment object
    """
    input_location = (
        env.systemtest_path / "door_tests" / "test_flow_regulation"
    )
    copy_files(
        sources=[
            input_location / "geometry.xml",
            input_location / "inifile.xml",
            input_location / "events.xml",
        ],
        dest=tmp_path,
    )
    jpscore_driver = JpsCoreDriver(
        jpscore_path=env.jpscore_path, working_directory=tmp_path
    )
    jpscore_driver.run()

    starting_times_dict = read_starting_times(tmp_path / "events.xml")
    traffic_constraints = parse_traffic_constraints(tmp_path / "inifile.xml")
    check_flow(
        load_trajectory(jpscore_driver.traj_file),
        starting_times_dict,
        traffic_constraints,
    )
