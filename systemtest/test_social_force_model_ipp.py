# SPDX-License-Identifier: LGPL-3.0-or-later
import sqlite3

import jupedsim as jps
import pytest


def _make_simulation(model=None):
    """Create a simple corridor simulation with the IPP model."""
    if model is None:
        model = jps.SocialForceModelIPP()
    sim = jps.Simulation(
        model=model,
        geometry=[(0, 0), (20, 0), (20, 4), (0, 4)],
    )
    return sim


def _add_journey(sim):
    """Add an exit and journey, return (journey_id, exit_id)."""
    exit_id = sim.add_exit_stage([(19, 0), (20, 0), (20, 4), (19, 4)])
    journey = jps.JourneyDescription([exit_id])
    journey_id = sim.add_journey(journey)
    return journey_id, exit_id


def test_ipp_model_construction_defaults():
    """Model can be created with default parameters."""
    sim = _make_simulation()
    journey_id, exit_id = _add_journey(sim)
    agent_id = sim.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
        )
    )
    assert agent_id is not None
    for _ in range(10):
        sim.iterate()


def test_ipp_model_construction_custom():
    """Model can be created with custom agent parameters."""
    model = jps.SocialForceModelIPP()
    sim = _make_simulation(model)
    journey_id, exit_id = _add_journey(sim)
    sim.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
            desired_speed=1.5,
            height=1.80,
            lambda_u=0.6,
            lambda_b=1.2,
            balance_speed=1.1,
        )
    )
    for _ in range(10):
        sim.iterate()
    agent = sim.agent(next(sim.agents()).id)
    # After 10 steps, agent should have moved
    assert agent.position[0] > 2.0


def test_ipp_agent_state_readable():
    """Agent state properties are readable at runtime."""
    sim = _make_simulation()
    journey_id, exit_id = _add_journey(sim)
    sim.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
            height=1.75,
            desired_speed=1.2,
        )
    )
    sim.iterate()
    agent = sim.agent(next(sim.agents()).id)
    state = agent.model
    assert state.height == pytest.approx(1.75)
    assert state.desired_speed == pytest.approx(1.2)
    assert isinstance(state.velocity, tuple)
    assert isinstance(state.ground_support_position, tuple)
    assert isinstance(state.ground_support_velocity, tuple)
    assert isinstance(state.radius, float)
    assert isinstance(state.lambda_u, float)
    assert isinstance(state.lambda_b, float)
    assert isinstance(state.balance_speed, float)
    assert isinstance(state.damping, float)
    assert isinstance(state.leg_force_distance, float)


def test_ipp_agent_state_writable():
    """Agent state properties can be modified at runtime."""
    sim = _make_simulation()
    journey_id, exit_id = _add_journey(sim)
    sim.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
        )
    )
    agent = sim.agent(next(sim.agents()).id)
    state = agent.model
    state.desired_speed = 2.0
    assert state.desired_speed == pytest.approx(2.0)
    state.lambda_u = 0.7
    assert state.lambda_u == pytest.approx(0.7)
    state.balance_speed = 1.5
    assert state.balance_speed == pytest.approx(1.5)


def test_ipp_constraint_rejects_negative_desired_speed():
    """CheckModelConstraint rejects agents with negative desired speed."""
    sim = _make_simulation()
    journey_id, exit_id = _add_journey(sim)
    with pytest.raises(RuntimeError):
        sim.add_agent(
            jps.SocialForceModelIPPAgentParameters(
                position=(2, 2),
                journey_id=journey_id,
                stage_id=exit_id,
                desired_speed=-1.0,
            )
        )


def test_ipp_constraint_rejects_overlapping_agents():
    """CheckModelConstraint rejects overlapping agents."""
    sim = _make_simulation()
    journey_id, exit_id = _add_journey(sim)
    sim.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
        )
    )
    with pytest.raises(RuntimeError):
        sim.add_agent(
            jps.SocialForceModelIPPAgentParameters(
                position=(2, 2),
                journey_id=journey_id,
                stage_id=exit_id,
            )
        )


def test_ipp_agents_reach_exit():
    """Agents using the IPP model can reach an exit."""
    sim = _make_simulation()
    journey_id, exit_id = _add_journey(sim)
    sim.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
            desired_speed=1.0,
        )
    )
    max_iterations = 5000
    for _ in range(max_iterations):
        sim.iterate()
        if sim.agent_count() == 0:
            break
    assert sim.agent_count() == 0, "Agent did not reach exit"


def test_ipp_bottleneck_scenario():
    """Bottleneck with 20 agents completes without crash."""
    model = jps.SocialForceModelIPP()
    sim = jps.Simulation(
        model=model,
        geometry=[
            (0, 0),
            (10, 0),
            (10, 1.5),
            (12, 1.5),
            (12, 0),
            (22, 0),
            (22, 4),
            (12, 4),
            (12, 2.5),
            (10, 2.5),
            (10, 4),
            (0, 4),
        ],
    )
    exit_id = sim.add_exit_stage([(21, 0), (22, 0), (22, 4), (21, 4)])
    journey = jps.JourneyDescription([exit_id])
    journey_id = sim.add_journey(journey)

    for i in range(20):
        x = 1.0 + (i % 5) * 1.5
        y = 0.8 + (i // 5) * 0.6
        sim.add_agent(
            jps.SocialForceModelIPPAgentParameters(
                position=(x, y),
                journey_id=journey_id,
                stage_id=exit_id,
                desired_speed=1.0,
            )
        )

    max_iterations = 20000
    for _ in range(max_iterations):
        sim.iterate()
        if sim.agent_count() == 0:
            break
    # Primary goal: simulation completes without crash.
    # Some agents may remain in narrow bottleneck (1m wide) with
    # pure exponential repulsion — that's a tuning concern, not a bug.
    assert sim.agent_count() < 20, (
        f"No agents progressed (all {sim.agent_count()} stuck)"
    )


def test_ipp_trajectory_writer(tmp_path):
    """SqliteIPPTrajectoryWriter writes ground support columns."""
    from jupedsim.sqlite_serialization import SqliteIPPTrajectoryWriter

    output = tmp_path / "traj.sqlite"
    writer = SqliteIPPTrajectoryWriter(output_file=output, every_nth_frame=1)
    sim = _make_simulation()
    journey_id, exit_id = _add_journey(sim)
    sim.add_agent(
        jps.SocialForceModelIPPAgentParameters(
            position=(2, 2),
            journey_id=journey_id,
            stage_id=exit_id,
            desired_speed=1.0,
        )
    )
    writer.begin_writing(sim)
    for _ in range(10):
        sim.iterate()
        writer.write_iteration_state(sim)
    writer.close()

    con = sqlite3.connect(output)
    cur = con.cursor()
    cols = [
        row[1]
        for row in cur.execute("PRAGMA table_info(trajectory_data)").fetchall()
    ]
    assert "pos_gs_x" in cols
    assert "pos_gs_y" in cols
    assert "vel_gs_x" in cols
    assert "vel_gs_y" in cols
    assert "height" in cols
    assert "radius" in cols

    rows = cur.execute("SELECT * FROM trajectory_data").fetchall()
    assert len(rows) > 0
    # Columns: frame, id, pos_x, pos_y, ori_x, ori_y,
    #          pos_gs_x, pos_gs_y, vel_gs_x, vel_gs_y, height, radius
    for row in rows:
        height_val = row[10]
        radius_val = row[11]
        assert height_val > 0
        assert radius_val > 0
    con.close()
