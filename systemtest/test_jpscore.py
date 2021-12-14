import pathlib
import platform
import pytest
import numpy
from driver.fixtures import env
from driver.utils import setup_jpscore_driver, get_file_text_diff
from driver.trajectories import load_trajectory
from driver.environment import Platform
from driver.inifile import parse_waiting_areas, instanciate_tempalte
from sympy.geometry import Point
from driver.driver import JpsCoreDriver
from driver.utils import copy_all_files, copy_files


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
    print(f"HELLO:{last_frame_index}")
    assert trajectories.agent_count_in_frame(last_frame_index) <= 15


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
