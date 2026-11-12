# Draft (sketch, not runnable: names like model and x1 are placeholders)
# ruff: noqa: F821
# Scope: 2.0 only.
# - Regions, targets and routes are returned as handles.
# - Routes are built and validated in one call: either a valid route or an
#   error.
# - Ownership, scope and state changes are explicit, and so are keywords.
# - Errors are informative and say how to fix the call.

import math

import jupedsim as jps
import shapely

# --- Concepts ---------------------------------------------------------
# Geometry   the complete walkable surface, built once by a GeometryBuilder
# Region     an identified patch of that surface
# Connector  a region joining two other regions (staircase, ramp)
# Location   coordinates qualified by a region: region.at(x, y)
# Target     what an agent heads for: a waypoint or an exit on a route, or
#            a location in direct steering
# Route      a directed graph of targets, given by its transitions
#
# Regions, targets, routes and agents are handles with .id and .name.
# `name` is optional, used in reprs, error messages and lookups, and unique
# in its scope: region names in a geometry, the others in a simulation.

# --- Geometry and regions ---------------------------------------------
# add_region and connect_regions return Region handles.
builder = jps.GeometryBuilder()
floor0 = builder.add_region(polygon=floor0_polygon, height=0.0, name="floor0")
floor1 = builder.add_region(polygon=floor1_polygon, height=3.0, name="floor1")
# Instead of polygon=, a region can be given as boundary= (a sequence of
# points) with optional holes= (sequences of points).
# A connector (staircase, ramp) is a region of its own. edge0 must be an
# edge of floor0's boundary and edge1 an edge of floor1's. A connector
# cannot be connected again: a staircase with a turn uses two connectors
# and a landing region.
staircase = builder.connect_regions(
    from_region=floor0,
    from_edge=edge0,
    to_region=floor1,
    to_edge=edge1,
    name="staircase",
)

# build() creates the immutable geometry. A builder is built once; for
# variants of a building, fill a new builder. All region handles of the
# builder refer to this geometry from now on.
geometry = builder.build()

# Locations: region.at(x, y) returns a location on the built geometry. It
# is checked right away and knows its height. Before build(), at() raises.
pos1 = floor0.at(x1, y1)
pos1.xy  # (x1, y1)
pos1.z  # 0.0
pos1.region  # floor0
# The region is always given explicitly: no call infers it from
# coordinates, so stacked floors cannot make a call ambiguous.

# Looking regions up, e.g. for a geometry loaded from a file:
geometry.region("floor0")  # by name
geometry.region(0)  # by id

# A geometry can be shared by several simulations; a location can be used
# in every simulation built on its geometry.
sim = jps.Simulation(model=model, geometry=geometry)

# --- Targets ----------------------------------------------------------
# sim.add_target() adds a waypoint or an exit and returns a handle, a
# RouteTarget. jps.Waypoint and jps.Exit describe the target; the handle
# is the target in the simulation.
#
# A target is a list of (area, region) pairs. Reaching any one of its
# areas completes the target. Areas may lie in different regions.
# An area is a polygon without holes, as a sequence of >= 3 (x, y) points
# or a shapely Polygon. Only the part of an area inside its region belongs
# to the target; an area that does not overlap its region is an error.
# A disc around a point: jps.circle_to_polygon(center=(x, y), radius=0.5).
wp1 = sim.add_target(jps.Waypoint([(wp1_polygon, floor0)]), name="wp1")
wp2 = sim.add_target(jps.Waypoint([(wp2_polygon, floor0)]), name="wp2")
wp3 = sim.add_target(
    jps.Waypoint(
        [(jps.circle_to_polygon(center=(x3, y3), radius=0.5), floor0)]
    ),
    name="wp3",
)
doors = sim.add_target(
    jps.Waypoint([(door_a, floor0), (door_b, floor1)]), name="doors"
)  # either door completes it
exit1 = sim.add_target(jps.Exit([(exit_polygon1, floor0)]), name="exit1")
exit2 = sim.add_target(jps.Exit([(exit_polygon2, floor0)]), name="exit2")
main_exits = sim.add_target(
    jps.Exit([(exit_polygon3, floor0), (exit_polygon4, floor0)]),
    name="main_exits",
)  # one exit with two doors

# Agents move toward an area along paths of least travel time (floor
# fields, see "Routing backend" below), which need not be the
# geometrically nearest part. A target is reached on the floor field grid
# (0.2 m cells by default), not on its polygon: arrival is off by up to
# half a cell.

# --- Routes -----------------------------------------------------------
# A route is a directed graph of targets, given as a list of transitions.
# The nodes are the targets that appear in the transitions. Each
# transition is one pair of a source and what follows it:
#   jps.FixedTransition(source, target)
#   jps.RoundRobinTransition(source, {target: weight, ...})
#   jps.LeastTargetedTransition(source, [target, ...])
# jps.START_TARGET is a virtual source: "from where the agent currently
# is". The transition leaving it chooses an agent's first target, once,
# when the agent joins the route (add_agent, reroute(route=)).
#
# The list of transitions is the only positional parameter; `name` is
# keyword-only. sim.add_route() returns a Route handle. A route belongs to
# its simulation.
#
# add_route checks:
# - Exactly one transition leaves START_TARGET.
# - At most one transition leaves each target.
# - START_TARGET is never what a transition leads to.
# - The graph, START_TARGET included, is connected when the directions of
#   the transitions are ignored. Separate parts belong in a route of their
#   own. A target that cannot be reached from START_TARGET but is
#   connected to the rest is allowed: a side entry, see first_target=
#   below.
# Loops are allowed. A route need not end at an exit: a target without an
# outgoing transition is the end, and the agent stays in its area.

# Linear: wp1, wp2, then exit1
route_a = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.FixedTransition(wp2, exit1),
    ],
    name="route_a",
)

# Minimal evacuation route: walk to exit1
route_x = sim.add_route([jps.FixedTransition(jps.START_TARGET, exit1)])

# Branching after a waypoint
route_c = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.RoundRobinTransition(wp2, {wp3: 1, exit1: 2, exit2: 1}),
        jps.FixedTransition(wp3, exit2),
    ],
    name="route_c",
)

# Split right at the start, 2:1, no helper waypoint needed
split = sim.add_route(
    [
        jps.RoundRobinTransition(jps.START_TARGET, {exit1: 2, exit2: 1}),
    ],
    name="split",
)

# Ending at an assembly point: no exit needed
assembly = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.LeastTargetedTransition(wp1, [wp2, wp3]),
    ],
)

# A loop, e.g. for single-file experiments
ring = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.FixedTransition(wp2, wp3),
        jps.FixedTransition(wp3, wp1),
    ],
    name="ring",
)

# RoundRobin: weights are positive integers; agents are distributed in
# their ratio. One counter per (route, source), shared by all agents
# deciding there. A transition object used in two routes gives each route
# its own counter.
# LeastTargeted: picks the target that the fewest agents are currently
# heading for, counted over all routes, as on master.

# Choosing the first target by travel time, by hand: sim.travel_time()
# reads the floor field of a target (see "Routing backend").
best = min((exit1, exit2), key=lambda e: sim.travel_time(e, pos1))
if math.isinf(sim.travel_time(best, pos1)):
    raise ValueError(f"none of exit1, exit2 is reachable from {pos1}")
agent9 = sim.add_agent(pos1, route=split, first_target=best)

# --- Agents -----------------------------------------------------------
# add_agent returns an Agent handle. Give either route= or steer_to=.
# first_target= overrides the transition leaving START_TARGET: it can be
# any target of the route, and the agent then does not take part in a
# round robin at the start. Targets are completed in iterate(), not in
# add_agent(): an agent placed inside its first target still has it right
# after add_agent() and completes it in the first iteration.
# state=: the agent's model state. Built-in models default it; custom
# models must pass it.
agent1 = sim.add_agent(pos1, route=route_a)  # starts with wp1
agent2 = sim.add_agent(
    pos2,
    route=route_c,
    first_target=wp2,
    state=jps.CollisionFreeSpeedModelState(desired_speed=1.2),
    name="anna",
)
agent3 = sim.add_agent(pos3, route=split)  # exit1 or exit2, 2:1
# Direct steering: the agent walks to a location; route is None. The way
# there still follows the walkable surface, including connectors
# (shortest path over the navigation mesh, no floor field), so speed
# fields do not apply. A location on another floor is reached over the
# staircase.
agent5 = sim.add_agent(pos5, steer_to=floor1.at(x, y))

# --- Runtime changes --------------------------------------------------
# agent.reroute() changes where an agent goes, with the same parameters as
# add_agent():
#   agent.reroute(route=r)                  join r, START_TARGET decides
#   agent.reroute(route=r, first_target=t)  join r at t
#   agent.reroute(first_target=t)           jump to t in the current route
#   agent.reroute(steer_to=location)        direct steering
# It takes effect in the next iteration and cannot be called while the
# simulation iterates.
# The calls in this loop illustrate the API; together they are not a
# meaningful scenario.
while sim.agent_count() > 0 and sim.elapsed_time() < 600:
    sim.iterate()

    # Targets completed in the most recent iteration, as {agent: target}.
    # An agent is reported in every iteration in which it is inside the
    # target, e.g. while it stays at the end of its route. An agent at an
    # exit is still in the simulation until the start of the next
    # iterate(), so evaluate the result right after iterate().
    completed = sim.agents_completed_target()

    # React: agents reaching wp1 walk on to a free spot of their own
    for agent, target in completed.items():
        if target == wp1:
            agent.reroute(steer_to=next_free_spot())

    # Count: which door of main_exits an agent leaves through
    for agent, target in completed.items():
        if target == main_exits:
            point = shapely.Point(agent.location.xy)
            door = "3" if exit_polygon3.covers(point) else "4"
            leaving_through[door] += 1

    # Filter agents with any condition: agents on route_c that head for
    # wp2 skip it
    for agent in sim.filter_agents(
        lambda a: a.route == route_c and a.target == wp2
    ):
        agent.reroute(first_target=wp3)
    # filter_agents() yields the agents; to count them, make a list
    heading_for_exit1 = len(
        list(sim.filter_agents(lambda a: a.target == exit1))
    )

    agent5.reroute(steer_to=floor0.at(x_new, y_new))
    agent1.reroute(route=route_c)

# --- State (read-only) ------------------------------------------------
print(agent1.route)  # Route(id=0, name="route_a"), or None in steering
# The target the agent heads for: a RouteTarget on a route, a location in
# direct steering
print(agent1.target)  # RouteTarget(id=1, name="wp2")
print(agent5.target)  # Location(x_new, y_new, region=floor0)
# Where the agent stands, with its region and height; a snapshot
print(agent1.location)  # Location(x, y, region=floor0)
# Unit vector toward the current target, (0, 0) inside the area of the
# target, as on master
print(agent1.route_orientation)

# --- Routing backend: floor fields ------------------------------------
# Two fixed routing engines: floor fields for targets on routes, the
# navigation mesh for direct steering.
# Configuration: jps.Simulation(model=model, geometry=geometry,
#     floorfield_options=jps.FloorfieldOptions(cell_size=0.2,
#                                              wall_influence_radius=0.5))
sim.travel_times(exit1)  # {floor0: TravelTimeGrid, staircase: ..., ...}
# Travel time in seconds from a location to a target: the value at the
# nearest walkable floor-field cell; 0 inside the target, inf if it cannot
# be reached.
sim.travel_time(exit1, floor0.at(x, y))
sim.default_speed_fields()  # {floor0: SpeedGrid, staircase: ..., ...}
sim.set_default_speed_field(floor0, values)  # targets without their own
sim.set_custom_speed_field(exit1, floor0, values)  # one target only
sim.custom_speed_fields(exit1)  # {floor0: SpeedGrid, staircase: None, ...}
sim.clear_custom_speed_field(exit1, floor0)
# Speed fields change routes, not walking speed, e.g. to make agents avoid
# a corridor. Floor fields are solved when first needed and cost memory
# per target and region, so targets are created once and shared by routes.

# --- Lookup and removal -----------------------------------------------
# By name or by id:
sim.agent("anna")
sim.agent(17)
sim.target("exit1")
sim.target(5)
sim.route("route_a")
sim.route(0)
sim.agents()  # all agents, as on master
sim.targets  # in creation order
sim.routes  # in creation order
# Agents near a location or inside a polygon. Both need a region, so that
# agents on a floor above or below are not included.
sim.agents_in_range(floor0.at(x, y), distance)  # list of agent handles
sim.agents_in_polygon(polygon, region=floor0)  # list of agent handles
# Removal is scheduled, as on master: the agent leaves at the start of the
# next iterate(), before agents interact.
sim.mark_agent_for_removal(agent1)
# Ids of the agents removed by the most recent iterate(), as on master
sim.removed_agents()

# --- Errors -----------------------------------------------------------
floor1.at(100, 100)
# ValueError: (100, 100) is not in floor1
other_floor = new_builder.add_region(polygon=other_polygon)
other_floor.at(x, y)  # before new_builder.build()
# ValueError: other_floor is not built yet; call build() first
builder.build()  # a second time
# ValueError: the builder was already built; create a new one
sim.add_target(jps.Waypoint([(door_polygon, floor1)]))  # not in floor1
# ValueError: area does not overlap floor1
sim.add_target(jps.Waypoint([(polygon_with_hole, floor0)]))
# ValueError: waypoint areas must be simple and without holes
sim.add_target(jps.Waypoint([(door_polygon, floor0)]), name="wp1")
# ValueError: target name 'wp1' is already used by RouteTarget(id=0, ...)
sim.add_route([])
# ValueError: route is empty
sim.add_route([jps.FixedTransition(wp1, exit1)])
# ValueError: no transition leaves START_TARGET; add one, e.g.
#   jps.FixedTransition(jps.START_TARGET, wp1)
sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.FixedTransition(wp1, wp3),
    ]
)
# ValueError: wp1 has 2 outgoing transitions; combine them into one, e.g.
#   jps.RoundRobinTransition(wp1, {wp2: 1, wp3: 1})
sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.FixedTransition(wp2, wp3),
    ]
)
# ValueError: wp2, wp3 are not connected to the rest of the route; define
#   them as a route of their own
sim.add_route([jps.FixedTransition(jps.START_TARGET, other_sim_wp)])
# ValueError: other_sim_wp belongs to another simulation
jps.RoundRobinTransition(wp2, {exit1: 0.6, exit2: 0.4})
# ValueError: weights must be positive integers
sim.add_agent(pos, route=route_a, steer_to=floor0.at(x, y))
# ValueError: use route= or steer_to=, not both
sim.add_agent(pos, route=route_c, first_target=main_exits)
# ValueError: main_exits is not part of route_c; targets are wp1, wp2, wp3,
#   exit1, exit2
sim.add_agent((x, y), route=route_a)
# TypeError: location must be a Location, e.g. floor0.at(x, y)
agent5.reroute(first_target=wp2)  # agent5 is in direct steering
# ValueError: the agent has no route; pass route=
sim.add_agent(floor0.at(x_closed_room, y_closed_room), route=route_a)
# ValueError: wp1 cannot be reached from Location(..., region=floor0)
custom_sim.add_agent(pos, route=route_a)  # simulation with a custom model
# ValueError: custom models need state=; pass your model's agent state

# --- Unchanged from master --------------------------------------------
# sim.iterate(count=1), sim.agent_count(), sim.elapsed_time(),
# sim.delta_time(), sim.iteration_count(), sim.close()
# sim.get_geometry(), sim.set_tracing(), sim.timer
# agent.id, agent.route_orientation, agent.state
# Operational models, their agent states and the trajectory writers

# --- From the current API on master to 2.0 ----------------------------
# Handles are used for object references; ids remain available for lookup
# and serialization (agent.id, target.id, region.id match trajectory
# files).
#
# Current API on master                     2.0
# WalkableSurface()                         GeometryBuilder()
# add_region(...) -> int                    builder.add_region(...) -> Region
# connect_regions(...) -> int               builder.connect_regions(...)
#                                               -> Region
# Simulation(geometry=surface)              Simulation(geometry=
#                                               builder.build())
# get_location(x, y, region_id=)            region.at(x, y)
# add_waypoint_stage(...) -> int            sim.add_target(jps.Waypoint(...))
#                                               -> RouteTarget
# add_exit_stage(...) -> int                sim.add_target(jps.Exit(...))
#                                               -> RouteTarget
# add_direct_steering_stage() -> int        removed: steer_to=
# add_journey(JourneyDescription) -> int    sim.add_route([...]) -> Route
# Transition.create_fixed_transition(t)     jps.FixedTransition(source, t)
# Transition.create_round_robin_            jps.RoundRobinTransition(source,
#     transition([(t, w), ...])                 {t: w, ...})
# Transition.create_least_targeted_         jps.LeastTargetedTransition(
#     transition([t, ...])                      source, [t, ...])
# add_agent(journey_id, stage_id,           sim.add_agent(location, route=,
#     position, region_id, ...) -> int          first_target=) -> Agent
# switch_agent_journey(agent_id,            agent.reroute(route=,
#     journey_id, stage_id)                     first_target=)
# agent.final_target = (x, y)               agent.reroute(steer_to=location)
# agent.journey_id, agent.stage_id          agent.route, agent.target
# agent.position                            agent.location
# mark_agent_for_removal(agent_id)          sim.mark_agent_for_removal(agent)
# agents_in_range((x, y), distance)         sim.agents_in_range(location,
#                                               distance)
# agents_in_polygon(polygon)                sim.agents_in_polygon(polygon,
#                                               region=floor0)
# get_stage(stage_id)                       sim.target("exit1") or
#                                               sim.target(5)
# agent(agent_id)                           sim.agent(17) or sim.agent("anna")
# travel_times(stage_id) -> {int: grid}     sim.travel_times(exit1)
#                                               -> {Region: grid}
# set_default_speed_field(region_id, ...)   sim.set_default_speed_field(
#                                               floor0, ...)
# set_custom_speed_field(stage_id,          sim.set_custom_speed_field(exit1,
#     region_id, ...)                           floor0, ...)
# custom_speed_fields(stage_id)             sim.custom_speed_fields(exit1)
# clear_custom_speed_field(stage_id,        sim.clear_custom_speed_field(
#     region_id)                                exit1, floor0)

# --- Open questions ---------------------------------------------------
# 1. Kind of a target: how does a script tell a waypoint from an exit,
#    given a RouteTarget handle?
# 2. Reporting: which information is exposed all the time, and what users
#    compute in their scripts, e.g. whether agents_completed_target()
#    returns agent handles or ids.
# 3. Direct steering is never completed. Should reaching the location be
#    reported?
# 4. Errors raised in C++: ValueError and TypeError, as above, or
#    SimulationError?
# 5. Looking regions up: on the geometry, as above, or on the simulation
#    like targets, routes and agents (sim.region("floor0"))?
