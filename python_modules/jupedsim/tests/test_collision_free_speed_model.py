# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps

_CORRIDOR = [(0, 0), (20, 0), (20, 2), (0, 2)]


def _simulation_with_agents(agents):
    """Corridor with a waypoint at the far end; `agents` is a list of (position, state)."""
    builder = jps.GeometryBuilder()
    room = builder.add_region(boundary=_CORRIDOR, height=0.0)
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=builder.build()
    )
    wp = sim.add_waypoint_stage(
        [(jps.circle_to_polygon(center=(19, 1), radius=0.5), room)]
    )
    journey_id = sim.add_journey(jps.JourneyDescription([wp]))
    ids = [
        sim.add_agent(
            journey_id=journey_id,
            stage_id=wp,
            position=position,
            state=state,
            region_id=room,
        )
        for position, state in agents
    ]
    return sim, ids


def _front_agent_lateral_heading(anisotropy):
    """Lateral heading of an agent with a neighbor behind it and to its left."""
    front = jps.CollisionFreeSpeedModelState(
        neighbor_repulsion_anisotropy=anisotropy
    )
    sim, (front_id, _) = _simulation_with_agents(
        [
            ((5.0, 1.0), front),
            ((4.6, 1.2), jps.CollisionFreeSpeedModelState()),
        ]
    )
    sim.iterate()
    return sim.agent(front_id).state.orientation[1]


def test_full_anisotropy_ignores_neighbor_behind():
    assert _front_agent_lateral_heading(anisotropy=0.0) < -0.01
    assert abs(_front_agent_lateral_heading(anisotropy=1.0)) < 1e-6
