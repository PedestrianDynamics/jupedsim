# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest


def _simulation():
    surface = jps.GeometryBuilder()
    room = surface.add_region(
        boundary=[(0, 0), (10, 0), (10, 10), (0, 10)], height=0.0
    )
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModelTordeux2016(),
        geometry=surface.build(),
        dt=0.01,
    )
    return simulation, room


def _waypoint(sim, room, center, name=None):
    return sim.add_target(
        jps.Waypoint(
            [(jps.circle_to_polygon(center=center, radius=0.5), room)]
        ),
        name=name,
    )


def test_a_route_with_every_transition_kind_leads_to_the_exit():
    sim, room = _simulation()
    first = _waypoint(sim, room, (2, 5))
    upper = _waypoint(sim, room, (5, 7))
    lower = _waypoint(sim, room, (5, 3))
    last = _waypoint(sim, room, (8, 5))
    exit_ = sim.add_target(
        jps.Exit([([(9, 4), (10, 4), (10, 6), (9, 6)], room)])
    )
    route = sim.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, first),
            jps.RoundRobinTransition(first, {upper: 1, lower: 1}),
            jps.LeastTargetedTransition(upper, [last]),
            jps.LeastTargetedTransition(lower, [last]),
            jps.FixedTransition(last, exit_),
        ]
    )
    for y in (4.0, 6.0):
        sim.add_agent(room.at(1.0, y), route=route)
    visited = {r.target for r in sim.reached_targets()}
    while sim.agent_count() > 0 and sim.iteration_count() < 5000:
        sim.iterate()
        visited.update(r.target for r in sim.reached_targets())
    assert sim.agent_count() == 0
    assert visited == {first, upper, lower, last, exit_}


@pytest.mark.parametrize("weight", [0, -1, 1.5, True])
def test_round_robin_weights_must_be_positive_integers(weight):
    sim, room = _simulation()
    wp = _waypoint(sim, room, (2, 5))
    with pytest.raises(ValueError, match="weights must be positive integers"):
        jps.RoundRobinTransition(jps.AGENT_LOCATION, {wp: weight})


def test_agent_location_is_no_target():
    sim, room = _simulation()
    wp = _waypoint(sim, room, (2, 5))
    with pytest.raises(ValueError, match="AGENT_LOCATION cannot be the target"):
        jps.FixedTransition(wp, jps.AGENT_LOCATION)
    with pytest.raises(TypeError, match="targets must be RouteTargets"):
        jps.LeastTargetedTransition(wp, [3])


def test_an_invalid_route_is_a_value_error():
    sim, room = _simulation()
    wp1 = _waypoint(sim, room, (2, 5), name="wp1")
    wp2 = _waypoint(sim, room, (5, 5))
    with pytest.raises(
        jps.SimulationError, match="No transition leaves AGENT_LOCATION"
    ):
        sim.add_route([jps.FixedTransition(wp1, wp2)])


def test_a_target_of_another_simulation_is_rejected():
    sim, room = _simulation()
    other, other_room = _simulation()
    foreign = _waypoint(other, other_room, (2, 5))
    with pytest.raises(
        ValueError, match=f"Target {foreign.id} belongs to another simulation"
    ):
        sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, foreign)])


def test_routes_are_looked_up_by_name_or_id():
    sim, room = _simulation()
    wp = _waypoint(sim, room, (2, 5))
    named = sim.add_route(
        [jps.FixedTransition(jps.AGENT_LOCATION, wp)], name="to wp"
    )
    unnamed = sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, wp)])
    assert sim.routes() == [named, unnamed]
    assert sim.route("to wp") == named
    assert sim.route(unnamed.id) == unnamed
    assert repr(named) == f"Route(id={named.id}, name='to wp')"
    assert repr(unnamed) == f"Route(id={unnamed.id})"
    with pytest.raises(KeyError, match="no route with id"):
        sim.route(10**6)
