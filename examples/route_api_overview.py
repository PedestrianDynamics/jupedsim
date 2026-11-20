#! /usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""The route API of branch route-api, as implemented: a runnable tour.

It follows the structure of examples/route_api_draft.py in PR 1717. Unlike the
draft it runs; the error messages under "Errors" are the real ones. See
route_api_vs_master.md for the comparison with master and the deviations from
the draft.
"""

import math

import jupedsim as jps
import shapely

# --- Concepts ---------------------------------------------------------
# Geometry   the complete walkable surface, built once by a GeometryBuilder
# Region     an identified patch of that surface: a floor or a connector
# Connector  a region joining two other regions (staircase, ramp)
# Location   coordinates qualified by a region: region.at(x, y)
# Target     what an agent heads for: a waypoint or an exit on a route, or
#            a location in direct steering
# Route      a directed graph of targets, given by its transitions
#
# Regions, targets, routes and agents are handles with .id and .name.
# `name` is optional, used in reprs, error messages and lookups, and unique
# in its scope: region names in a geometry, the others in a simulation.
# Events are reported as immutable records, see "Reporting".

# --- Geometry and regions ---------------------------------------------
# add_region and connect_regions return Region handles. Extra boundary
# vertices mark the edges the staircase is attached to.
builder = jps.GeometryBuilder()
floor0 = builder.add_region(
    boundary=[(0, 0), (20, 0), (20, 3), (20, 7), (20, 10), (0, 10)],
    height=0.0,
    name="floor0",
)
floor1 = builder.add_region(
    boundary=[(26, 0), (46, 0), (46, 10), (26, 10), (26, 7), (26, 3)],
    height=2.0,
    name="floor1",
)
# A connector (staircase, ramp) is a region of its own and cannot be
# connected again: a staircase with a turn uses two connectors and a landing.
staircase = builder.connect_regions(
    from_region=floor0,
    from_edge=((20, 3), (20, 7)),
    to_region=floor1,
    to_edge=((26, 3), (26, 7)),
    name="staircase",
)
# floor0.at(1, 1) here: SimulationError, floor0 is not built yet

# build() creates the immutable geometry. A builder is built once; all its
# region handles refer to this geometry from now on.
geometry = builder.build()

# region.at(x, y) returns a location, checked right away; it knows its
# height. The region is always given explicitly: no call infers it from
# coordinates, so stacked floors cannot make a call ambiguous.
pos1 = floor0.at(2, 5)
assert pos1.xy == (2, 5) and pos1.z == 0.0 and pos1.region == floor0

# Lookup by name or id, e.g. for a geometry loaded from a file. Handles of
# the same region are equal, wherever they come from.
assert geometry.region("floor1") == floor1 == geometry.region(1)

# A geometry can be shared by several simulations.
sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=geometry)
assert sim.geometry.region("staircase") == staircase

# --- Targets ----------------------------------------------------------
# sim.add_target() adds a waypoint or an exit and returns a RouteTarget.
# jps.Waypoint and jps.Exit describe it: a list of (area, region) pairs.
# An agent reaches the target in any one of its areas; areas may lie in
# different regions. Only the part of an area inside its region counts.
# An area is a shapely Polygon or a sequence of points. A disc around a
# point: jps.circle_to_polygon(center=(x, y), radius=0.5).
wp1 = sim.add_target(
    jps.Waypoint([(jps.circle_to_polygon(center=(8, 5), radius=1), floor0)]),
    name="wp1",
)
wp2 = sim.add_target(
    jps.Waypoint([(jps.circle_to_polygon(center=(16, 5), radius=1), floor0)]),
    name="wp2",
)
wp3 = sim.add_target(
    jps.Waypoint([(jps.circle_to_polygon(center=(36, 5), radius=1), floor1)]),
    name="wp3",
)
exit1 = sim.add_target(
    jps.Exit([(shapely.box(45, 0, 46, 4), floor1)]), name="exit1"
)
exit2 = sim.add_target(
    jps.Exit([(shapely.box(45, 6, 46, 10), floor1)]), name="exit2"
)
# One exit with two doors, on different floors
main_exits = sim.add_target(
    jps.Exit(
        [(shapely.box(0, 0, 1, 2), floor0), (shapely.box(45, 4, 46, 6), floor1)]
    ),
    name="main_exits",
)
# The description is kept as given, without clipping.
assert isinstance(exit1.description, jps.Exit)
assert exit1.description.areas[0][1] == floor1

# Agents move toward an area along paths of least travel time (floor
# fields, see "Routing backend"). A target is reached on the floor field
# grid (0.2 m cells by default): arrival is off by up to half a cell.

# --- Routes -----------------------------------------------------------
# A route is a directed graph of targets, given as a list of transitions:
#   jps.FixedTransition(source, target)
#   jps.RoundRobinTransition(source, {target: weight, ...})
#   jps.LeastTargetedTransition(source, [target, ...])
# jps.AGENT_LOCATION is the source of the transition that chooses an
# agent's first target, once, when it joins the route (add_agent,
# reroute(route=)).
#
# add_route checks:
# - Exactly one transition leaves AGENT_LOCATION.
# - At most one transition leaves each target.
# - No transition leaves an exit, and none leads back to its own source.
# - The graph is connected when the directions are ignored. A target that
#   cannot be reached from AGENT_LOCATION but is connected to the rest is
#   allowed: a side entry, see first_target= below.
# Loops are allowed. A target without an outgoing transition ends the
# route, and the agent stays in its area.

# Linear: wp1, wp2, up the stairs to exit1
route_a = sim.add_route(
    [
        jps.FixedTransition(jps.AGENT_LOCATION, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.FixedTransition(wp2, exit1),
    ],
    name="route_a",
)

# Branching after a waypoint
route_c = sim.add_route(
    [
        jps.FixedTransition(jps.AGENT_LOCATION, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.RoundRobinTransition(wp2, {wp3: 1, exit1: 2, exit2: 1}),
        jps.FixedTransition(wp3, exit2),
    ],
    name="route_c",
)

# Split right at the start, 2:1, no helper waypoint needed
split = sim.add_route(
    [jps.RoundRobinTransition(jps.AGENT_LOCATION, {exit1: 2, exit2: 1})],
    name="split",
)

# Ending at an assembly point: no exit needed
assembly = sim.add_route(
    [
        jps.FixedTransition(jps.AGENT_LOCATION, wp1),
        jps.LeastTargetedTransition(wp1, [wp2, wp3]),
    ],
    name="assembly",
)

# A loop, e.g. for single-file experiments
ring = sim.add_route(
    [
        jps.FixedTransition(jps.AGENT_LOCATION, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.FixedTransition(wp2, wp1),
    ],
    name="ring",
)

# RoundRobin: weights are positive integers; one counter per (route,
# source), in blocks as on master (2:1 -> e1 e1 e2). LeastTargeted: the
# target the fewest agents currently head for, counted over all routes.

# --- Agents -----------------------------------------------------------
# add_agent returns an Agent handle. Give either route= or steer_to=.
# first_target= selects the first target, instead of the transition
# leaving AGENT_LOCATION: any target of the route; later targets follow
# the transitions. An agent added inside its first target has reached it.
# state= defaults to the model's state for built-in models; custom models
# must pass it.
agent1 = sim.add_agent(pos1, route=route_a)  # starts with wp1
agent2 = sim.add_agent(
    floor0.at(2, 3),
    route=route_c,
    first_target=wp2,
    state=jps.CollisionFreeSpeedModelState(desired_speed=1.2),
    name="anna",
)
agent3 = sim.add_agent(floor0.at(2, 7), route=split)  # exit1 or exit2, 2:1
agent4 = sim.add_agent(floor0.at(4, 2), route=assembly)

# Choosing the first target by travel time, by hand
start = floor0.at(4, 8)
best = min((exit1, exit2), key=lambda e: sim.travel_time(e, start))
assert not math.isinf(sim.travel_time(best, start))
agent5 = sim.add_agent(start, route=split, first_target=best)

# Direct steering: the agent walks to a location; route is None. The way
# follows the walkable surface over the shortest path (no floor field, so
# speed fields do not apply), up the staircase to another floor.
agent6 = sim.add_agent(floor0.at(4, 5), steer_to=floor1.at(30, 5))
assert agent6.route is None and agent6.current_target.region == floor1

# --- Runtime changes --------------------------------------------------
# agent.reroute() takes the parameters of add_agent():
#   agent.reroute(route=r)                  join r, AGENT_LOCATION decides
#   agent.reroute(route=r, first_target=t)  join r at t
#   agent.reroute(first_target=t)           jump to t in the current route
#   agent.reroute(steer_to=location)        direct steering
# It takes effect in the next iteration; current_target changes at once.
agent4.reroute(route=route_a, first_target=wp2)
assert agent4.current_target == wp2

# --- Reporting --------------------------------------------------------
# Current state is read from handles; events come as immutable records
# with ids, so a record does not suggest that its agent still exists.
# sim.reached_targets() -> tuple of TargetReachedRecord
#   .agent_id, .target (RouteTarget, or the Location in direct steering),
#   .time, .location (where the agent stood)
#   A target is reached after a step, or by add_agent() inside it; the agent
#   moves on at the beginning of the next iteration, one target per step.
# sim.removed_agents() -> tuple of RemovalRecord
#   .agent_id, .exit (None after mark_agent_for_removal()), .time
#   An agent reaching an exit, or marked, is agent.leaving until it is removed
#   at the beginning of the next iteration.
# Both cover the most recent iterate(count=n) call and are replaced by the
# next one. Scripts that react to events call iterate() with count=1.
reached = {wp1: 0, wp2: 0, wp3: 0}
left_by = {exit1: 0, exit2: 0, main_exits: 0, None: 0}
steered_arrived = False
while sim.agent_count() > 0 and sim.elapsed_time() < 120:
    sim.iterate()
    for record in sim.reached_targets():
        if record.target in reached:
            reached[record.target] += 1
        elif isinstance(record.target, jps.Location):
            steered_arrived = True
            # Arrival is reported once; send the agent on.
            sim.agent(record.agent_id).reroute(route=split)
    for removal in sim.removed_agents():
        left_by[removal.exit] += 1
    if sim.iteration_count() == 500:
        sim.mark_agent_for_removal(agent2)

# --- State (read-only) ------------------------------------------------
# agent.id, agent.name, agent.route (None in direct steering),
# agent.current_target (RouteTarget or Location), agent.location,
# agent.route_orientation, agent.state (model state, writable fields)
print(f"reached: { {t.name: n for t, n in reached.items()} }")
print(f"removed: { {(e.name if e else None): n for e, n in left_by.items()} }")
print(f"steered agent arrived: {steered_arrived}")
print(f"left after {sim.elapsed_time():.1f} s: {sim.agent_count()} agents")

# --- Routing backend: floor fields ------------------------------------
# Floor fields for targets on routes, the navigation mesh for direct
# steering. Configure with jps.Simulation(..., floorfield_options=
# jps.FloorfieldOptions(cell_size=..., wall_influence_radius=...)).
grids = sim.travel_times(exit1)  # {Region: TravelTimeGrid}
assert set(grids) == {floor0, floor1, staircase}
# Travel time in seconds from a location: the value at the nearest
# walkable cell; 0 inside the target, inf if it cannot be reached.
assert sim.travel_time(exit1, floor1.at(45.5, 2)) == 0.0
speeds = sim.default_speed_fields()  # {Region: SpeedGrid}
# Speed fields change routes, not walking speed:
sim.set_custom_speed_field(exit1, floor1, speeds[floor1].values * 0.5)
assert sim.has_custom_speed_field(exit1, floor1)
sim.clear_custom_speed_field(exit1, floor1)
sim.set_default_speed_field(floor0, speeds[floor0].values)

# --- Lookup and removal -----------------------------------------------
assert sim.target("exit1") == exit1 == sim.target(exit1.id)
assert sim.route("route_a") == route_a == sim.route(route_a.id)
assert sim.targets()[0] == wp1 and sim.routes()[0] == route_a  # creation order
# sim.agent("anna"), sim.agent(17): by name or id
# sim.agents(filter=lambda a: a.current_target == exit1): selected agents
# Agents near a location (in plain sight, as the models see them) or
# inside a polygon clipped to one region, like a target area:
#   sim.agents_in_range(floor0.at(x, y), distance)
#   sim.agents_in_polygon(polygon, region=floor0)
# sim.mark_agent_for_removal(agent): removed at the start of the next
# iterate(), before agents interact.


# --- Errors -----------------------------------------------------------
# Checks done in Python raise TypeError, ValueError and KeyError. Errors
# found in C++ are SimulationError for now; mapping them to Python types
# is planned as a separate PR.
def error_of(call):
    try:
        call()
    except Exception as error:
        return f"{type(error).__name__}: {error}"
    raise AssertionError("no error")


errors = {
    "region.at before build()": lambda: (
        jps.GeometryBuilder()
        .add_region(
            boundary=[(0, 0), (1, 0), (1, 1), (0, 1)], height=0.0, name="f"
        )
        .at(0.5, 0.5)
    ),
    "location outside its region": lambda: floor1.at(5, 5),
    "unknown region": lambda: geometry.region("floor9"),
    "unknown target": lambda: sim.target("exit9"),
    "unknown agent": lambda: sim.agent(999),
    "location is not a Location": lambda: sim.add_agent((2, 2), route=route_a),
    "route= and steer_to=": lambda: sim.add_agent(
        floor0.at(3, 3), route=route_a, steer_to=floor0.at(4, 4)
    ),
    "empty route": lambda: sim.add_route([]),
    "no entry": lambda: sim.add_route([jps.FixedTransition(wp1, exit1)]),
    "two outgoing transitions": lambda: sim.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, wp1),
            jps.FixedTransition(wp1, wp2),
            jps.FixedTransition(wp1, wp3),
        ]
    ),
    "disconnected part": lambda: sim.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, wp1),
            jps.FixedTransition(wp2, wp3),
        ]
    ),
    "transition leaving an exit": lambda: sim.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, exit1),
            jps.FixedTransition(exit1, wp1),
        ]
    ),
    "self-transition": lambda: sim.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, wp1),
            jps.FixedTransition(wp1, wp1),
        ]
    ),
    "weights": lambda: jps.RoundRobinTransition(wp2, {exit1: 0.6, exit2: 0.4}),
    "first_target outside the route": lambda: sim.add_agent(
        floor0.at(3, 3), route=route_a, first_target=main_exits
    ),
    "name used twice": lambda: sim.add_target(
        jps.Waypoint([(shapely.box(2, 2, 3, 3), floor0)]), name="wp1"
    ),
}
for what, call in errors.items():
    print(f"{what}: {error_of(call)}")


# Output:
#   reached: {'wp1': 1, 'wp2': 2, 'wp3': 0}
#   removed: {'exit1': 4, 'exit2': 1, 'main_exits': 0, None: 1}
#   steered agent arrived: True
#   left after 36.0 s: 0 agents
#   region.at before build(): SimulationError: Region 0 (f) is not built yet; call build() first.
#   location outside its region: SimulationError: Point (5, 5) is not in region 1 (floor1)
#   unknown region: KeyError: "no region named 'floor9'"
#   unknown target: KeyError: "no target named 'exit9'"
#   unknown agent: KeyError: 'no agent with id 999'
#   location is not a Location: TypeError: location must be a Location, e.g. region.at(x, y), got tuple
#   route= and steer_to=: ValueError: use route= or steer_to=, one of them
#   empty route: SimulationError: The route is empty.
#   no entry: SimulationError: No transition leaves AGENT_LOCATION; add one, e.g. jps.FixedTransition(jps.AGENT_LOCATION, ...).
#   two outgoing transitions: SimulationError: Target 1 (wp1) has 2 outgoing transitions; combine them into one, e.g. a jps.RoundRobinTransition.
#   disconnected part: SimulationError: Targets 2 (wp2), 3 (wp3) are not connected to the rest of the route; define them as a route of their own.
#   transition leaving an exit: SimulationError: Target 4 (exit1) is an exit: agents leave the simulation there, so no transition can leave it.
#   self-transition: SimulationError: Target 1 (wp1) cannot follow itself; remove the self-transition to keep agents there.
#   weights: ValueError: weights must be positive integers, got 0.6 for target 4 (exit1)
#   first_target outside the route: SimulationError: Target 6 (main_exits) is not part of route 1 (route_a).
#   name used twice: SimulationError: The target name 'wp1' is already used.
