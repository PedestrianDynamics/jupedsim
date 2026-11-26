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
    wp = sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=(19, 1), radius=0.5), room)]
        )
    )
    route = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])
    agents = [
        sim.add_agent(room.at(*position), route=route, state=state)
        for position, state in agents
    ]
    return sim, agents


def _front_agent_lateral_heading(anisotropy):
    """Lateral heading of an agent with a neighbor behind it and to its left."""
    front = jps.CollisionFreeSpeedModelState(
        neighbor_repulsion_anisotropy=anisotropy
    )
    sim, (front_agent, _) = _simulation_with_agents(
        [
            ((5.0, 1.0), front),
            ((4.6, 1.2), jps.CollisionFreeSpeedModelState()),
        ]
    )
    sim.iterate()
    return front_agent.state.orientation[1]


def test_full_anisotropy_ignores_neighbor_behind():
    assert _front_agent_lateral_heading(anisotropy=0.0) < -0.01
    assert abs(_front_agent_lateral_heading(anisotropy=1.0)) < 1e-6


def test_agent_without_route_direction_still_feels_neighbors():
    """An agent standing on its target has no route direction; with full
    anisotropy its neighbors must still push it away."""
    builder = jps.GeometryBuilder()
    room = builder.add_region(boundary=_CORRIDOR, height=0.0)
    sim = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=builder.build()
    )
    standing = sim.add_agent(
        room.at(5.0, 1.0),
        steer_to=room.at(5.0, 1.0),
        state=jps.CollisionFreeSpeedModelState(
            neighbor_repulsion_anisotropy=1.0
        ),
    )
    sim.add_agent(
        room.at(5.35, 1.0),
        steer_to=room.at(5.35, 1.0),
        state=jps.CollisionFreeSpeedModelState(),
    )
    sim.iterate()
    assert standing.location.xy[0] < 5.0
