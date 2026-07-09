# SPDX-License-Identifier: LGPL-3.0-or-later
import math

import jupedsim as jps
import pytest

GEOMETRY = [(0, 0), (100, 0), (100, 100), (0, 100)]


def make_sim():
    return jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=GEOMETRY,
    )


def test_default_routing_is_astar():
    sim = make_sim()
    assert sim.routing_engine_name == "TAStar"


def test_construct_with_direct_path_engine():
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=GEOMETRY,
        routing_engine=jps.DirectPathRoutingEngine(),
    )
    assert sim.routing_engine_name == "DirectPath"


def test_construct_with_astar_engine():
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=GEOMETRY,
        routing_engine=jps.TAStarRoutingEngine(),
    )
    assert sim.routing_engine_name == "TAStar"


def test_switch_to_direct_path():
    sim = make_sim()
    sim.switch_routing_engine(jps.DirectPathRoutingEngine())
    assert sim.routing_engine_name == "DirectPath"


def test_switch_back_to_astar():
    sim = make_sim()
    sim.switch_routing_engine(jps.DirectPathRoutingEngine())
    sim.switch_routing_engine(jps.TAStarRoutingEngine())
    assert sim.routing_engine_name == "TAStar"


def test_astar_engine_name():
    navi = jps.TAStarRoutingEngine()
    assert navi.name == "TAStar"


def test_astar_waypoints_straight_line_in_open_space():
    navi = jps.TAStarRoutingEngine()
    navi.set_geometry(GEOMETRY)
    from_pt = (10.0, 10.0)
    to_pt = (90.0, 90.0)
    waypoints = navi.compute_waypoints(from_pt, to_pt)
    expected_dist = math.hypot(90 - 10, 90 - 10)
    path_dist = sum(
        math.hypot(b[0] - a[0], b[1] - a[1])
        for a, b in zip(waypoints[:-1], waypoints[1:])
    )
    assert math.isclose(path_dist, expected_dist, rel_tol=1e-6)


def test_non_engine_raises():
    sim = make_sim()
    with pytest.raises(Exception):
        sim.switch_routing_engine("NonExistent")


def test_direct_path_routing_engine_waypoints():
    """DirectPathRoutingEngine returns a straight line ignoring geometry."""
    engine = jps.DirectPathRoutingEngine()
    frm = (10.0, 10.0)
    to = (90.0, 90.0)
    assert engine.compute_waypoints(frm, to) == [frm, to]


def test_direct_path_routing_engine_is_routable():
    engine = jps.DirectPathRoutingEngine()
    assert engine.is_routable((50.0, 50.0)) is True


def test_custom_python_routing_engine():
    """Users can implement a custom routing engine in Python."""

    class MyEngine(jps.RoutingEngine):
        def name(self):
            return "MyEngine"

        def set_geometry(self, _):
            pass

        def compute_waypoints(self, frm, to):
            return [frm, (50.0, 50.0), to]

        def is_routable(self, _):
            return True

    sim = make_sim()
    sim.switch_routing_engine(MyEngine())
    assert sim.routing_engine_name == "MyEngine"


def test_python_direct_path_engine_drives_simulation():
    """End-to-end: an agent guided by the Python DirectPathRoutingEngine
    reaches its exit.

    Geometry: 100x100 square.  Exit: right edge (x ≈ 100, y ∈ [49, 51]).
    Agent starts at (10, 50) and must walk straight to the exit.
    The Python engine returns [current_pos, destination] on every call, so the
    agent moves in a straight horizontal line and is eventually removed by the
    exit stage — demonstrating that the C++ simulation loop is calling back
    into the Python engine on every iteration.
    """
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=GEOMETRY,
        dt=0.05,
    )
    sim.switch_routing_engine(jps.DirectPathRoutingEngine())
    assert sim.routing_engine_name == "DirectPath"

    exit_id = sim.add_exit_stage([(99, 49), (100, 49), (100, 51), (99, 51)])
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))

    agent_id = sim.add_agent(
        jps.CollisionFreeSpeedModelAgentParameters(
            position=(10.0, 50.0),
            journey_id=journey_id,
            stage_id=exit_id,
        )
    )

    # Run until the agent is removed (reached the exit) or a safety limit.
    max_iterations = 5000
    for _ in range(max_iterations):
        sim.iterate()
        if agent_id in sim.removed_agents():
            break
    else:
        pytest.fail("Agent did not reach the exit within the iteration limit")
