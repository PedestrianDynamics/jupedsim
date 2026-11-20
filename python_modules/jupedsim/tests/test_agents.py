# SPDX-License-Identifier: LGPL-3.0-or-later
import jupedsim as jps
import pytest


class _Room:
    """A 10 x 10 room with a waypoint left and an exit right."""

    def __init__(self, model=None):
        surface = jps.GeometryBuilder()
        self.room = surface.add_region(
            boundary=[(0, 0), (10, 0), (10, 10), (0, 10)], height=0.0
        )
        self.sim = jps.Simulation(
            model=model or jps.CollisionFreeSpeedModel(),
            geometry=surface.build(),
            dt=0.01,
        )
        self.waypoint = self.sim.add_target(
            jps.Waypoint([([(1, 4), (3, 4), (3, 6), (1, 6)], self.room)]),
            name="waypoint",
        )
        self.exit = self.sim.add_target(
            jps.Exit([([(9, 4), (10, 4), (10, 6), (9, 6)], self.room)]),
            name="exit",
        )
        self.route = self.sim.add_route(
            [
                jps.FixedTransition(jps.AGENT_LOCATION, self.waypoint),
                jps.FixedTransition(self.waypoint, self.exit),
            ],
            name="through",
        )

    def at(self, x, y):
        return self.room.at(x, y)


def test_an_agent_follows_its_route_and_is_recorded_on_the_way():
    r = _Room()
    agent = r.sim.add_agent(r.at(2, 5), route=r.route, name="anna")
    assert (agent.route, agent.current_target) == (r.route, r.waypoint)
    assert r.sim.agent("anna") == agent
    assert repr(agent) == f"Agent(id={agent.id}, name='anna')"

    (reached,) = r.sim.reached_targets()
    assert (reached.agent_id, reached.target) == (agent.id, r.waypoint)
    assert reached.location.xy == (2, 5)
    r.sim.iterate()
    assert agent.current_target == r.exit

    while not agent.leaving and r.sim.iteration_count() < 2000:
        r.sim.iterate()
    assert r.sim.reached_targets()[-1].target == r.exit
    r.sim.iterate()
    (removal,) = r.sim.removed_agents()
    assert (removal.agent_id, removal.exit) == (agent.id, r.exit)
    with pytest.raises(KeyError, match="no agent named 'anna'"):
        r.sim.agent("anna")


def test_records_cover_all_iterations_of_one_iterate_call():
    r = _Room()
    for x, y in ((3.1, 4.5), (3.3, 5.5)):
        r.sim.add_agent(r.at(x, y), route=r.route)
    r.sim.iterate(40)
    assert len({record.time for record in r.sim.reached_targets()}) == 2
    r.sim.iterate()
    assert r.sim.reached_targets() == ()


def test_agents_added_inside_their_target_are_recorded_until_the_next_iterate():
    r = _Room()
    for y in (4.5, 5.5):
        r.sim.add_agent(r.at(2, y), route=r.route)
    assert len(r.sim.reached_targets()) == 2
    r.sim.iterate()
    assert r.sim.reached_targets() == ()


def test_a_marked_agent_is_recorded_without_exit():
    r = _Room()
    agent = r.sim.add_agent(r.at(5, 5), route=r.route)
    r.sim.mark_agent_for_removal(agent)
    r.sim.iterate()
    (removal,) = r.sim.removed_agents()
    assert (removal.agent_id, removal.exit) == (agent.id, None)


def test_reroute_changes_where_the_agent_goes():
    r = _Room()
    other = r.sim.add_route([jps.FixedTransition(jps.AGENT_LOCATION, r.exit)])
    agent = r.sim.add_agent(r.at(5, 5), route=r.route, first_target=r.exit)
    assert agent.current_target == r.exit

    agent.reroute(first_target=r.waypoint)
    assert (agent.route, agent.current_target) == (r.route, r.waypoint)
    agent.reroute(route=other)
    assert (agent.route, agent.current_target) == (other, r.exit)
    spot = r.at(5, 8)
    agent.reroute(steer_to=spot)
    assert agent.route is None
    assert agent.current_target.xy == spot.xy


def test_a_steered_agent_reports_its_arrival_once():
    r = _Room()
    spot = r.at(5, 6)
    agent = r.sim.add_agent(r.at(5, 5), steer_to=spot)
    arrivals = []
    for _ in range(300):
        r.sim.iterate()
        arrivals += r.sim.reached_targets()
    (arrival,) = arrivals
    assert arrival.agent_id == agent.id
    assert isinstance(arrival.target, jps.Location)
    assert arrival.target.xy == spot.xy


def test_agents_can_be_filtered_by_any_condition():
    r = _Room()
    to_exit = r.sim.add_agent(r.at(5, 5), route=r.route, first_target=r.exit)
    r.sim.add_agent(r.at(5, 7), route=r.route)
    assert list(r.sim.agents(filter=lambda a: a.current_target == r.exit)) == [
        to_exit
    ]


@pytest.mark.parametrize(
    "kwargs, error, message",
    [
        ({}, ValueError, "use route= or steer_to="),
        (
            {"steer_to": "spot", "route": "route"},
            ValueError,
            "use route= or steer_to=",
        ),
        (
            {"steer_to": "spot", "first_target": "exit"},
            ValueError,
            "does not apply",
        ),
        (
            {"route": "route", "first_target": "other_exit"},
            jps.SimulationError,
            "is not part of",
        ),
    ],
)
def test_add_agent_rejects_invalid_combinations(kwargs, error, message):
    r = _Room()
    other_exit = r.sim.add_target(
        jps.Exit([([(0, 0), (1, 0), (1, 1), (0, 1)], r.room)])
    )
    values = {
        "spot": r.at(5, 8),
        "route": r.route,
        "exit": r.exit,
        "other_exit": other_exit,
    }
    with pytest.raises(error, match=message):
        r.sim.add_agent(r.at(5, 5), **{k: values[v] for k, v in kwargs.items()})
    assert r.sim.agent_count() == 0


def test_add_agent_needs_a_location():
    r = _Room()
    with pytest.raises(TypeError, match="location must be a Location"):
        r.sim.add_agent((5, 5), route=r.route)


def test_reroute_rejects_invalid_calls():
    r = _Room()
    agent = r.sim.add_agent(r.at(5, 5), steer_to=r.at(5, 8))
    with pytest.raises(jps.SimulationError, match="The agent has no route"):
        agent.reroute(first_target=r.exit)
    with pytest.raises(
        ValueError, match="give route=, first_target= or steer_to="
    ):
        agent.reroute()
    with pytest.raises(ValueError, match="not both"):
        agent.reroute(route=r.route, steer_to=r.at(5, 8))


def test_a_location_of_another_geometry_is_rejected():
    r = _Room()
    other = _Room()
    with pytest.raises(
        jps.SimulationError, match="location .* belongs to another geometry"
    ):
        r.sim.add_agent(other.at(2, 5), route=r.route)

    shared = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=r.sim.geometry
    )
    shared.add_agent(r.at(2, 5), steer_to=r.at(8, 5))
    assert shared.agent_count() == 1


def test_custom_models_need_a_state():
    class Model(jps.CustomOperationalModel):
        def compute_next_state(self, current, next, step):
            return (0.0, 0.0)

    r = _Room(Model())
    with pytest.raises(ValueError, match="custom models need state="):
        r.sim.add_agent(r.at(5, 5), route=r.route)


def test_lookups_take_no_bools():
    r = _Room()
    r.sim.add_agent(r.at(5, 5), route=r.route)
    for lookup in (r.sim.agent, r.sim.target, r.sim.route):
        with pytest.raises(TypeError, match="id or name"):
            lookup(True)
