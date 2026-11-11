# Draft (sketch, not runnable: names like model and x1 are placeholders)
# ruff: noqa: F821
# - Stages and routes are owned by the simulation and returned as handles.
# - Routes are built and validated in one call: either a valid route or an error.
# - Common cases are concise; equivalent forms have the same semantics.
# - Ownership, scope and state changes are explicit, and so are keywords.
# - Errors are informative and say how to fix the call.


import math

import jupedsim as jps

# --- Concepts ---------------------------------------------------------
# Surface    the complete walkable geometry
# Region     an identified patch of that surface
# Floor      a building concept; one floor may consist of several regions
# Connector  a region joining two other regions (staircase, ramp)
# Staircase  one or more inclined connector regions, with landing regions
#            in between where the staircase turns
# Stage      a waypoint or an exit of a simulation: destination areas
#            attached to regions
# Route      the order in which an agent visits stages
# Location   coordinates qualified by a region: region.at(x, y)
# Target     what an agent is currently trying to reach: a stage on a
#            route, a location in direct steering. A descriptive word, not
#            a type: agent.stage is the stage, agent.target the
#            direct-steering location.

# --- Regions ----------------------------------------------------------
# Regions are returned as handles, like stages, not as bare ints.
# `name` is optional and used in reprs and error messages. Region handles
# expose .id, .name (None if unnamed) and .height.
surface = jps.WalkableSurface()
floor0 = surface.add_region(polygon=floor0_polygon, height=0.0, name="floor0")
floor1 = surface.add_region(polygon=floor1_polygon, height=3.0, name="floor1")
# Instead of polygon=, a region can be given as boundary= (a sequence of
# points) with optional holes= (sequences of points). Region names are
# unique on a surface.
# A connector (staircase, ramp) is a region of its own: connect_regions returns
# its handle. edge0 must be an edge of floor0's boundary and edge1 an edge of
# floor1's; each is lifted to its region's height. The four endpoints must
# lie in one plane inclined at most 50 degrees, as on master. The
# orientation of the edges is fixed automatically. A connector cannot
# itself be connected again: a staircase with a turn uses two connectors
# and a landing region.
staircase = surface.connect_regions(
    from_region=floor0,
    from_edge=edge0,
    to_region=floor1,
    to_edge=edge1,
    name="staircase",
)

# Looking regions up, e.g. for a geometry loaded from a file:
surface.regions  # (Region(id=0, name="floor0", height=0.0), ..., Region(id=2, name="staircase"))
surface.region("floor0")  # by name
surface.region(id=0)  # by id, e.g. from saved results
surface.regions_at((x, y))  # every region covering a point (discovery)
# surface.regions, regions_at() and sim.stages are in creation order.

# Locations: region.at(x, y) returns an immutable Location: coordinates plus
# the region's identity. It is checked against the surface right away.
# Locations compare and hash by (x, y, region): floor0.at(5, 2) equals
# floor0.at(5, 2) but not floor1.at(5, 2).
# A simulation resolves it against its own copy of the geometry when the
# location is used (add_agent, steer_to), so the same Location works in
# every simulation built from this surface, provided that simulation's
# snapshot contains the region and the position. Otherwise add_agent and
# steer_to raise ValueError: for a region from another surface, or one
# added to `surface` after the simulation was built (see Errors).
# Regions compare and hash by (surface, id), so a region handle from
# `surface` is the same dict key in every simulation built from it.
# The region is always supplied explicitly: no call infers it from
# coordinates, so adding a region (e.g. an upper floor) cannot make
# existing calls ambiguous. regions_at is a lookup for discovery and
# interactive tools (which regions cover this point?); at() builds the
# location in the region you pass:
surface.regions_at((5, 2))  # (floor0, floor1): stacked floors
floor1.at(5, 2)  # Location(5.0, 2.0, region=floor1)
floor1.at(5, 2).xy  # (5.0, 2.0) when only the coordinates are needed
floor1.at(5, 2).region  # floor1

# The surface is copied when the simulation is constructed: later edits to
# `surface` do not affect `sim`. No builder is needed for this.
sim = jps.Simulation(model, surface)

# --- Stages -----------------------------------------------------------
# - Regions describe where agents can walk.
# - A stage's areas are alternative places that complete the stage:
#   reaching any one of them is enough.
# - A route describes the order of stages.
#
# Waypoints and exits are areas. General form: a list of (area, region)
# pairs, so one stage can cover several areas (e.g. two doors) and regions.
# Area: a polygon without holes, as a sequence of >= 3 (x, y) vertices or a
# shapely Polygon. Only the part of an area inside its region belongs to
# the stage; an area whose clipped part has no area is an error.
# Shorthands: a single polygon, or a point with `radius` (a disc-shaped
# area), each with region=. The area must overlap its region with
# positive area; touching along an edge does not count.
# Agent positions and target points are locations: region.at(x, y).
# Coordinates are in metres, in the same frame as the geometry.
#
# Stage grouping sets what close() and hold() act on together: one exit
# with two doors closes both doors. Create separate exits for doors that
# must close individually.
#
# Routing to areas uses floor fields (see "Routing backend" below): agents
# move toward the area along paths of least travel time, which need not be
# the geometrically nearest part. A stage is reached on the floor field
# grid (0.2 m cells by default), not on its polygon: arrival is off by up
# to half a cell, and by up to a cell for areas thinner than one.
# Direct steering (steer_to= / steer_to()) replaces the route with a single
# point destination. Navigation to that point still follows the walkable
# surface, including connectors (point-to-point shortest path over the
# navigation mesh, no floor field); it is not a straight walk toward the
# point. Speed fields therefore do not apply: a route agent switched to
# steer_to() no longer avoids areas made costly by speed fields (e.g.
# smoke).
# The point counts as reached when the agent's last step passed within
# 0.05 m of it (horizontal distance, same region), whatever the step
# length. The agent is then arrived, as with STAY: arrived == True, it
# stays at the point (steers back if pushed away) and is not removed, until
# follow() or steer_to(). route_orientation is (0, 0) only exactly on the
# point, so use arrived to tell arrival.
# To pass a point and go on, make it a waypoint of the route instead, e.g.
# sim.add_route(
#     [(wp_wait, wp_edge), (wp_edge, jps.RoundRobin({exit1: 1, exit2: 1}))],
#     start=wp_wait,
# ).
#
# `region`, `radius` and `name` are keyword-only.
# `name` is optional and used in reprs and error messages; unnamed stages
# appear as Waypoint(id=3). The messages below assume named stages. Stage
# names are unique in a simulation.
# Stage handles expose .id and .name (None if unnamed).
wp1 = sim.add_waypoint((x1, y1), radius=0.5, region=floor0, name="wp1")
wp2 = sim.add_waypoint((x2, y2), radius=0.5, region=floor0, name="wp2")
wp3 = sim.add_waypoint((x3, y3), radius=0.5, region=floor0, name="wp3")
door_polygon = [(), (), (), ()]
wp4 = sim.add_waypoint(door_polygon, region=floor0, name="wp4")
doors = sim.add_waypoint(
    [(door_a, floor0), (door_b, floor1)], name="doors"
)  # either door completes it; areas may lie in different regions
exit1 = sim.add_exit(exit_polygon1, region=floor0, name="exit1")
exit2 = sim.add_exit(exit_polygon2, region=floor0, name="exit2")
main_exits = sim.add_exit(
    [(exit_polygon3, floor0), (exit_polygon4, floor0)], name="main_exits"
)  # main_exits.close() closes both doors; exit1, exit2 close one at a time

# --- Routes -----------------------------------------------------------
# A route is a directed graph of stages. sim.add_route() creates it and
# returns its handle: the simulation owns the route, as it owns its stages,
# and the route cannot be used in another simulation.
# `start`, `endless` and `name` are keyword-only. `name` is optional and
# used in reprs and error messages; route names are unique in a simulation.
# Route handles expose .name (None if unnamed).
# Rules (jps.RoundRobin, jps.LeastTravelTime, jps.LeastTargeted) are
# independent values: one rule object may be used in several routes. Their
# runtime state, such as the round-robin counter, belongs to the route's
# decision stage, not to the rule.
#
# Validation: from every stage of the route, an exit or STAY must be
# reachable via weights > 0 (agents may enter at any stage via start=).
# This is checked at construction and on every update_weights(); exit
# closures are exempt because they are temporary by intent.
# The check is structural: it does not guarantee that every agent
# eventually leaves a loop.
# Route validity is a property of the graph, not of the geometry: it does
# not check that a stage can be reached from where an agent stands. That
# is checked when a stage becomes an agent's target: add_agent(), follow()
# and steer_to() raise if the first target cannot be reached. Later targets
# are always reachable: Simulation() rejects disconnected surfaces, regions
# connect both ways and speeds have a minimum. Agents are stranded only
# through closed exits.

# Linear (shorthand): a DIRECTED list of stages, walked in order.
# The last element must be an exit or jps.STAY, unless endless=True.
route_a = sim.add_route([wp1, wp2, exit1], name="route_a")
route_b = sim.add_route([wp2, wp3, exit2])
route_x = sim.add_route([exit1])  # minimal evacuation route: walk to exit1

# Branching (general form): a list of (from, to) pairs; order does not matter.
# Each stage has exactly one outgoing transition: a stage or a rule.
# `start` is the default start stage for agents on this route.
route_c = sim.add_route(
    [
        (wp1, wp2),
        (wp2, jps.RoundRobin({wp3: 1, exit1: 2, exit2: 1})),
        (wp3, exit2),
    ],
    start=wp1,
)

# Ending at an assembly point (Sammelstelle) must be explicit.
# STAY is a terminal action, not a stage: on completing wp3 the agent is
# arrived, agent.stage stays wp3, and it actively stays inside wp3's
# arrival region (steers back if pushed out).
route_e = sim.add_route([wp1, wp2, wp3, jps.STAY])

# Loops are allowed if an exit or STAY stays reachable from every stage.
route_f = sim.add_route(
    [
        (wp1, wp2),
        (wp2, jps.RoundRobin({exit1: 2, exit2: 1, wp3: 1, wp4: 4})),
        (wp3, jps.STAY),
        (wp4, wp1),
    ],
    start=wp1,
)

# Endless loops (tours, single-file) must be explicit.
# endless=True waives the need to reach an exit or STAY: the route may run
# indefinitely. Exits and STAY remain allowed, so a crowd can circulate and
# be sent out later with update_weights().
# With endless=True, repeating the first stage at the end of a linear list
# closes the loop; no other stage may appear twice.
patrol = sim.add_route([wp1, wp2, wp3, wp4, wp1], endless=True)
tour = sim.add_route(
    [
        (wp1, wp2),
        (wp2, jps.RoundRobin({wp3: 1, wp4: 1})),
        (wp3, wp1),
        (wp4, wp1),
    ],
    start=wp1,
    endless=True,
)

# Weights: integers from 0 to 1 000 000 000, at least one > 0.
# RoundRobin uses smooth weighted round-robin; ties go to the earlier target
# in insertion order. With this algorithm only the ratio matters:
# {exit1: 3, exit2: 2} and {exit1: 60, exit2: 40} behave the same.
# - 3:2 gives e1 e2 e1 e2 e1 / repeat
# - 60:40 gives exactly the same sequence
# One counter per (route, stage), shared by all agents deciding there.
# Agents arriving in the same step are served in agent-id order.

# LeastTravelTime: each agent picks the target with the least travel time
# from its position, read from the targets' floor fields. The choice is made
# once, when the agent completes the stage, and kept on the way. No weights.
# Ties go to the earlier target in the list, as with min().
route_n = sim.add_route(
    [
        (wp1, jps.LeastTravelTime([exit1, exit2])),
    ],
    start=wp1,
)

# LeastTargeted: picks the target that the fewest agents are currently
# heading for, as on master. No weights.
# A LeastTravelTime or LeastTargeted rule that lists its own stage as a
# candidate gives a UserWarning: agents may never leave it.

# `start` is always a stage: rules fire only on completing a stage. To
# choose the first target from where the agent stands, choose it in the
# script and give each choice its own route. If no candidate is reachable,
# min() returns one with travel time inf; add_agent() would then raise for
# that exit alone, so check first to report all candidates.
best = min((exit1, exit2), key=lambda e: sim.travel_time(e, pos1))
if math.isinf(sim.travel_time(best, pos1)):
    raise ValueError(f"none of exit1, exit2 is reachable from {pos1}")
agent9 = sim.add_agent(pos1, route=sim.add_route([best]))

# --- Agents -----------------------------------------------------------
# `start` is the agent's first target; agent.stage is always the stage the
# agent is heading for. Stages are completed in iterate(), not in
# add_agent(): an agent spawned inside its start stage's arrival region
# still has that stage right after add_agent(), reaches it in the first
# iteration and goes on to the next stage.
# `start` may be an exit: the agent walks to it and is removed on reaching it.
# state=: the agent's model state. For built-in models it may be omitted;
# the agent then gets the model's default state. Custom models must pass it.
# pos1, pos2, ... are locations, e.g. pos1 = floor0.at(xa, ya). The first
# parameter is named `location`, like agent.location; agent.position is
# the plain (x, y) tuple.
agent1 = sim.add_agent(pos1, route=route_a)  # starts at wp1
agent8 = sim.add_agent(
    pos8,
    route=route_a,
    state=jps.CollisionFreeSpeedModelState(desired_speed=1.2),
)
agent2 = sim.add_agent(pos2, route=route_b)  # starts at wp2
agent3 = sim.add_agent(
    pos3, route=route_c
)  # starts at the route's default (wp1)
agent4 = sim.add_agent(pos4, route=route_c, start=wp2)  # overrides the default
agent5 = sim.add_agent(pos5, steer_to=floor0.at(x, y))  # direct steering
# Steering to a location on another floor walks there over the connecting
# staircase.
agent7 = sim.add_agent(floor0.at(x0, y0), steer_to=floor1.at(x, y))
agent6 = sim.add_agent(pos6, route=route_x)  # walks to exit1

# --- Runtime changes --------------------------------------------------
# Bound the loop by time: with holds or closures, agents may wait for a
# later release or reopening, so "no agent is moving" is not a stop
# condition. agent_count() includes held, stranded and arrived agents.
while sim.agent_count() > 0 and sim.elapsed_time() < 600:
    sim.iterate()

    # Direct steering: change the target with steer_to(); also switches a
    # route agent to direct steering.
    agent5.steer_to(floor0.at(x_new, y_new))
    agent7.steer_to(floor1.at(x_new, y_new))  # location on floor1
    # route -> direct steering; route and stage become None
    agent2.steer_to(floor0.at(x_new, y_new))

    # Routes are changed with follow() only (route and start in one call).
    agent1.follow(route_b)  # starts at the first stage, wp2
    agent1.follow(route_b, start=wp3)  # or an explicit start
    agent3.follow(route_c, start=wp2)
    # direct steering -> route; route == route_a, stage == wp1
    agent5.follow(route_a)
    # follow() to a route whose start is a closed exit does not raise: the
    # agent is stranded at once, as if the exit had closed on its way.
    # follow() always restarts, also on the agent's current route: calling
    # it every iteration sends the agent back to the start each time.

    # Weights: a patch, like dict.update; targets not named keep their weight.
    # Validated against the whole route and applied atomically: on error the
    # old weights and the old counter are kept. On success the counter of
    # (route_c, wp2) resets if any weight changed; unchanged weights keep
    # the sequence, so a controller may apply its weights at every step.
    # Takes effect at the next decision; agents already assigned keep their
    # target.
    route_c.update_weights(wp2, {wp3: 0})  # wp3: 0, exit1: 2, exit2: 1
    print(route_c.weights(wp2))  # {wp3: 0, exit1: 2, exit2: 1}

    # Hold agents at a stage (e.g. phased evacuation), weights are kept.
    # The hold is scoped to (route_c, wp2) and lasts until release():
    # every agent on route_c whose target is wp2, now or later (including
    # agents added after the call), keeps walking to wp2 and stops there,
    # held == True on arrival. Agents on other routes that use wp2 are not
    # affected, nor are agents that completed wp2 before the call.
    # A held agent that follow()s another route is no longer held.
    # hold() on a stage that is already held does nothing.
    route_c.hold(wp2)
    # Held agents go on in arrival order (ties by agent id), current rule.
    # release() returns the agents whose hold was lifted, in arrival order,
    # as open() returns the agents that resumed. Released does not mean
    # moving: an agent whose rule has no open target is stranded at once
    # and is still returned. Agents still walking to wp2 pass it without
    # stopping and are not returned. release() on a stage that is not held
    # does nothing and returns an empty tuple.
    released = route_c.release(wp2)
    # Two ways to make agents wait: hold()/release() frees a group at one
    # moment, in arrival order; a route ending in STAY plus follow() on
    # agents with arrived == True decides agent by agent (e.g. by time or
    # for a subset). For a group released at once, both behave the same,
    # except that follow() goes in the order of the calls.

    # Close an exit for all routes (a method on the exit handle;
    # sim.close() frees the whole simulation). Rules skip closed targets.
    # Closing decides which stages may be targets. To make agents prefer
    # paths around costly areas (e.g. smoke), change the speed fields used
    # to compute travel times instead. Speed fields change routes, not how
    # fast agents walk: physical slowing is set in the movement model.
    # informed=True:  agents assigned to it by a rule are reassigned now by
    #                 that rule and walk directly to the new target.
    # informed=False: agents walk to the exit and are reassigned there.
    # An agent is stranded once it cannot continue: its next target is the
    # closed exit via a fixed transition, or its rule has no open target
    # with weight > 0 left. Stranded agents stop where they are (informed),
    # at the closed exit (not informed), or inside the waypoint whose rule
    # has no open target.
    # Rules only skip closed immediate targets: in route_c, wp3 (which leads
    # only to exit2) can still be chosen at wp2 after exit2 closes; agents
    # sent there are stranded at wp3.
    # close() on a closed exit and open() on an open one do nothing and
    # return an empty tuple.
    stranded = exit2.close(informed=True)  # agents stranded now
    # Valid target again. Agents stranded by this exit resume: agents
    # assigned by a rule re-run the decision at their last decision stage;
    # agents on a fixed transition keep exit2 as their stage and walk on
    # to it from where they stopped. Agents already rerouted are not
    # reassigned.
    resumed = exit2.open()  # agents that resumed

# --- State (read-only) ------------------------------------------------
# After the runtime block: agent2 is in direct steering, agent5 on route_a.
print(agent2.route, agent2.stage)  # None, None in direct steering
# Route(start=wp1, ...), Waypoint(id=1, name="wp1")
print(agent5.route, agent5.stage)
# agent.target is the direct-steering target, a location.
print(agent2.target)  # Location(x_new, y_new, region=floor0)
# agent.position is the (x, y) tuple, as on master. agent.location is the
# same point with its region, so it can be passed on directly, e.g.
# agent5.steer_to(agent2.location). Both are snapshots: a saved
# agent.location does not follow the agent. Location.region, Stage.name
# and the other handle attributes are read-only.
print(agent2.position)  # (x, y)
print(agent2.location)  # Location(..., region=floor0)
print(agent5.target)  # None on a route: use agent5.stage
# Navigation direction (unit vector toward the current target, or (0, 0)
# inside the area of the stage the agent heads for), as on master. Not the
# velocity (model state) and not the body orientation. It is computed in
# iterate(), and set at once by add_agent(), follow() and steer_to(), and
# by close(informed=True), open() and release() for the agents they
# reassign.
print(agent5.route_orientation)
print(agent3.held)  # True while held at a stage
# True after completing a waypoint whose transition is STAY, or on
# reaching a steer_to() point, until follow() or steer_to()
print(agent3.arrived)
print(agent3.stranded)  # True once the agent cannot continue (closed exit)
print(agent3.alive)  # False once the agent has left the simulation
# Agents stopped at wp2 (held == True); agents still walking to a held
# wp2 are not included, but release() frees them as well.
print(route_c.held_at(wp2))
# Whether a hold or a closure is in force (read-only; change them with
# hold()/release() and close()/open()).
print(route_c.is_held(wp2))  # True between hold(wp2) and release(wp2)
print(exit2.closed)  # True between exit2.close() and exit2.open()
# Agents whose current target is wp2, on any route or only on route_c.
# Membership is by assignment: agent.stage == wp2 (and agent.route ==
# route_c if given), so held, stranded and arrived agents are included
# when their stage matches. Agents in direct steering have no stage and
# are never returned. Returns a snapshot (tuple of handles): calling
# follow() while iterating does not change which agents are visited.
sim.agents_targeting(wp2)  # (Agent(id=3), Agent(id=4), ...)
sim.agents_targeting(wp2, route=route_c)

# Inspecting a route
# next() returns the rule as an immutable value and weights() returns a copy:
# changing them has no effect on the route; use update_weights().
route_c.next(wp1)  # wp2 (fixed transition)
route_c.next(wp2)  # RoundRobin({wp3: 0, exit1: 2, exit2: 1})
route_c.next(exit1)  # None: exits have no next stage
route_c.weights(wp2)  # {wp3: 0, exit1: 2, exit2: 1}
route_c.stages  # (wp1, wp2, wp3, exit1, exit2)
print(route_c)
# Route(start=wp1)
#   wp1 -> wp2
#   wp2 -> RoundRobin({wp3: 0, exit1: 2, exit2: 1})
#   wp3 -> exit2

# --- Rerouting --------------------------------------------------------
# Smoke at wp2: agents on route1 that still target wp2 take route2
# from wp3 on. Agents that already passed wp2 are not affected.
route1 = sim.add_route([wp1, wp2, exit1])
route2 = sim.add_route([wp1, wp3, exit1])
for agent in sim.agents_targeting(wp2, route=route1):
    agent.follow(route2, start=wp3)
# Alternatives, depending on what the smoke does:
# - wp2 is one branch of a rule: route.update_weights(..., {wp2: 0})
# - paths near the smoke should cost more: speed fields (Routing backend)
# - the smoke blocks an exit: exit.close()
#
# The four controls change different things:
#   agent.follow(route, start=)   one agent's route
#   route.update_weights(...)     how future decisions distribute agents
#   exit.close() / exit.open()    whether an exit is available
#   speed fields                  the travel costs used for navigation
# A high travel cost is not an unavailable exit, and choosing an exit
# (LeastTravelTime) is a different decision from navigating to it.
#
# hold() and close() differ in scope and effect:
#   route.hold(wp)   one route, waypoints only. Agents stop at wp until
#                    release(); nobody is reassigned or stranded.
#   exit.close()     all routes, exits only. Rules skip the exit; agents
#                    are reassigned or stranded until open().

# --- Two floors -------------------------------------------------------
# An agent upstairs leaves through an exit on the ground floor. The staircase
# is a region (from connect_regions), not a stage: the exit's floor field
# already routes over it.
exit_ground = sim.add_exit(
    ground_exit_polygon, region=floor0, name="exit_ground"
)
evacuate = sim.add_route([exit_ground])
upstairs = sim.add_agent(floor1.at(xu, yu), route=evacuate)
print(upstairs.stage)  # Exit(id=8, name="exit_ground")
grid = sim.travel_times(exit_ground)[floor1]  # floor1's grid
grid.values  # seconds to exit_ground, shape (height, width); inf = not walkable
grid.directions  # walking directions, shape (height, width, 2)
# The staircase is a region, so it can also hold a stage, e.g. a waypoint
# halfway up that agents must pass:
mid_stairs = sim.add_waypoint(mid_stairs_polygon, region=staircase, name="mid")
# To make agents pass the lower landing (e.g. a counting line), make it a
# stage of the route:
landing0 = sim.add_waypoint(
    lower_landing_polygon, region=floor0, name="landing0"
)
via_landing = sim.add_route([landing0, exit_ground])

# --- Routing backend: floor fields ------------------------------------
# Two fixed routing engines: floor fields for stages (routes), the
# navigation mesh for point targets (direct steering). There is no option
# to switch engines.
# Everything floor-field-specific is in this section; the route API above
# only needs "is the agent inside the stage's area" and "which
# direction leads there", plus travel times for LeastTravelTime.
# Configuration: jps.Simulation(model, surface,
#     floorfield_options=jps.FloorfieldOptions(cell_size=0.2,
#                                              wall_influence_radius=0.5))
sim.travel_times(exit1)  # {floor0: TravelTimeGrid, staircase: ..., floor1: ...}
# Travel time in seconds from a location to a stage, as on master: the
# value at the nearest walkable floor-field cell, not interpolated, so its
# resolution is cell_size; 0 inside the stage, inf if the stage cannot be
# reached from there.
sim.travel_time(exit1, floor0.at(x, y))
sim.default_speed_fields()  # {floor0: SpeedGrid, staircase: ..., floor1: ...}
sim.set_default_speed_field(floor0, values)  # all stages without their own
sim.set_custom_speed_field(exit1, floor0, values)  # one stage only
sim.custom_speed_fields(exit1)  # {floor0: SpeedGrid, staircase: None, ...}
sim.clear_custom_speed_field(exit1, floor0)
# Speed fields change routes, not walking speed. Floor fields are solved
# when first needed and cost memory per stage and region, so stages are
# created once and shared by routes.

# --- Errors -----------------------------------------------------------
sim.add_waypoint(door_polygon, radius=0.5, region=floor0)
# ValueError: radius applies to point waypoints only
sim.add_waypoint((x1, y1), radius=0, region=floor0)
# ValueError: radius must be > 0
sim.add_waypoint(door_polygon)
# TypeError: region= is required, e.g. sim.add_waypoint(area, region=floor0)
sim.add_waypoint(door_polygon, region=floor1)  # door_polygon is not in floor1
# ValueError: area does not overlap floor1
sim.add_waypoint(door_polygon, region=1)
# TypeError: region must be a Region handle, e.g. surface.region("floor1")
surface.region("floor2")
# KeyError: no region named floor2; regions are floor0, floor1, staircase
surface.connect_regions(
    from_region=floor0,
    from_edge=edge_inside_floor0,
    to_region=floor1,
    to_edge=edge1,
)
# ValueError: from_edge is not an edge of floor0's boundary
surface.connect_regions(
    from_region=staircase, from_edge=edge2, to_region=floor2, to_edge=edge3
)
# ValueError: staircase is a connector and cannot be connected again; add a
#   landing region between two connectors
sim.add_agent(new_region.at(x, y), route=route_a)  # region added after sim
# ValueError: new_region is not part of this simulation's geometry
sim.add_agent((x, y), route=route_a)
# TypeError: location must be a Location, e.g. floor0.at(x, y)
sim.add_agent(pos, steer_to=(x, y))
# TypeError: steer_to must be a Location, e.g. floor0.at(x, y)
agent5.steer_to((x, y))
# TypeError: location must be a Location, e.g. floor0.at(x, y)
floor1.at(100, 100)
# ValueError: (100, 100) is not in floor1
sim.add_waypoint(polygon_with_hole, region=floor0)
# ValueError: waypoint polygons must be simple and without holes
sim.add_waypoint((x1, y1), radius=0.5, region=floor0, name="wp1")
# ValueError: stage name 'wp1' is already used by Waypoint(id=0, name="wp1")
surface.add_region(polygon=other_polygon, name="floor0")
# ValueError: region name 'floor0' is already used by floor0
sim.add_route([])
# ValueError: route is empty
sim.add_route([(wp1, wp2), (wp2, wp3)])
# ValueError: wp3 has no next stage; end with an exit or add (wp3, jps.STAY)
sim.add_route(
    [(wp1, wp2), (wp2, wp3), (wp2, jps.RoundRobin({exit1: 2, exit2: 1}))]
)
# ValueError: wp2 has 2 outgoing transitions; combine them into one rule
sim.add_route([(wp1, wp2), (wp2, wp1)])
# ValueError: no exit or STAY reachable; to loop on purpose, use endless=True
sim.add_route([wp1, wp2, wp1, exit1])
# ValueError: wp1 appears twice; only repeating the first stage at the end closes a loop
sim.add_route([exit1, wp1])
# ValueError: exits are terminal; remove (exit1, wp1)
sim.add_route([(wp1, (15, 6.5)), ((15, 6.5), exit1)], start=wp1)
# TypeError: (15, 6.5) is not a stage; use sim.add_waypoint((15, 6.5), ...)
sim.add_route([wp1, other_sim_wp, exit1])
# ValueError: other_sim_wp belongs to another simulation
sim.add_route([wp2, exit1], name="route_a")
# ValueError: route name 'route_a' is already used
sim.route("route_z")
# KeyError: no route named route_z
jps.RoundRobin({exit1: 0.6, exit2: 0.4})
# ValueError: weights must be integers
jps.RoundRobin({exit1: 0, exit2: 0})
# ValueError: at least one weight must be > 0
jps.RoundRobin({exit1: 2_000_000_000, exit2: 1})
# ValueError: weights must be <= 1000000000
route_c.update_weights(wp1, {wp2: 3})
# ValueError: wp1 has a fixed transition, not a weighted rule
route_n.update_weights(wp1, {exit1: 2})
# ValueError: wp1 uses LeastTravelTime, which has no weights
route_c.update_weights(wp2, {wp4: 1})
# ValueError: wp4 is not a target of wp2; targets are wp3, exit1, exit2 (see route.weights(wp2))
route_c.weights(wp1)
# ValueError: wp1 has a fixed transition; use route.next(wp1)
route_c.next(wp4)
# ValueError: wp4 is not part of this route; stages are wp1, wp2, wp3, exit1, exit2
route_c.update_weights(wp2, {wp3: 0, exit1: 0, exit2: 0})
# ValueError: at least one target at wp2 needs weight > 0; to stop agents, use route.hold(wp2)
route_g = sim.add_route(
    [
        (wp1, jps.RoundRobin({wp2: 1, exit1: 1})),
        (wp2, wp1),
    ],
    start=wp1,
)
route_g.update_weights(wp1, {exit1: 0})
# ValueError: with these weights no exit or STAY is reachable from wp1, wp2;
#   weights unchanged
route_c.hold(exit1)
# ValueError: exits cannot be held; use exit1.close()
route_c.is_held(wp4)
# ValueError: wp4 is not part of this route; stages are wp1, wp2, wp3, exit1, exit2
exit2.closed = True
# AttributeError: use exit2.close()
sim.agents_targeting(wp4, route=route_c)
# ValueError: wp4 is not part of this route; stages are wp1, wp2, wp3, exit1, exit2
sim.add_agent(floor0.at(x_closed_room, y_closed_room), route=route_a)
# ValueError: wp1 cannot be reached from Location(..., region=floor0)
sim.add_agent(pos, route=route_a, steer_to=floor0.at(x, y))
# ValueError: use route= or steer_to=, not both
sim.add_agent(pos, route=sim.add_route([(wp1, wp2), (wp2, exit1)]))
# ValueError: route given as pairs has no default start; pass start= here or to add_route
sim.add_agent(pos, route=route_c, start=wp4)
# ValueError: wp4 is not part of this route; stages are wp1, wp2, wp3, exit1, exit2
other_sim.add_agent(pos, route=route_c)
# ValueError: route belongs to another simulation
sim.add_agent(other_surface_region.at(x, y), route=route_a)
# ValueError: Region(id=0, name="hall") belongs to another surface
agent1.route = route_c
# AttributeError: use agent.follow(route, start=...)
agent5.target = (x_new, y_new)
# AttributeError: use agent.steer_to(location)
agent1.stage  # after agent1 left through an exit
# RuntimeError: Agent(id=1) has left the simulation (agent.alive is False)
custom_sim.add_agent(pos, route=route_a)  # simulation with a custom model
# ValueError: custom models need state=; pass your model's agent state

# --- Lookup and removal -----------------------------------------------
# Removal is scheduled, as on master: the agent leaves at the start of the
# next iterate(), before agents interact. Until then it is still alive.
# A removed agent's handle keeps .id and .alive readable; everything else
# raises.
agent1.remove()
# removed_agents() returns the records of the most recent iterate(), as
# on master. Calls do not consume them: repeated calls return the same
# records until the next iterate().
# As on master, removal runs at the start of iterate(): an agent that
# reaches an exit in one iteration is removed at the start of the next.
# Its record is written when it reaches the exit, so it appears one
# iterate() before agent.alive turns False. An agent removed with
# agent.remove() gets its record in the iterate() in which it leaves.
# A record holds the agent's id, the exit it left through (None for
# agent.remove()) and the time it reached that exit (for agent.remove(),
# the time of removal), e.g. for exit counts and flow over time.
for removal in sim.removed_agents():
    removal.id  # 17
    removal.exit  # Exit(id=5, name="exit1"), or None
    removal.time  # simulation time in seconds, see above
sim.stages  # (Waypoint(id=0, name="wp1"), ..., Exit(id=8, name="exit_ground"))
sim.stage("exit1")  # Exit(id=5, name="exit1")
sim.stage(id=5)  # the same exit, by id
sim.routes  # every route of the simulation, in creation order
sim.route("route_a")  # by name
sim.agent(17)  # Agent(id=17)
# All agents still in the simulation, in id order: held, stranded and
# arrived agents included. A snapshot (tuple of handles), like
# agents_targeting(): agents added or removed while iterating do not
# change it.
sim.agents()  # (Agent(id=2), Agent(id=3), ...)

# --- Lifecycle of one agent -------------------------------------------
# 1. Spawned inside its first waypoint:
#    sim.add_agent(pos_in_wp1, route=route_c)
#    stage == wp1 right after add_agent() -> first iteration: wp1
#    reached -> stage == wp2.
# 2. Weighted branch:
#    reaches wp2 -> counter of (route_c, wp2) picks the next slot, e.g.
#    exit1 -> stage == exit1; the assignment is kept until exit1 or a
#    change (follow, close with informed=True).
# 3. Held:
#    route_c.hold(wp2) while or before stage == wp2 -> agent stops inside
#    wp2, held == True. route_c.release(wp2) -> served
#    in arrival order by the current rule -> held == False.
# 4. Exit closed on the way:
#    stage == exit2, assigned by the rule at wp2:
#      exit2.close(informed=True)  -> reassigned now to exit1.
#      exit2.close(informed=False) -> walks to exit2, reassigned there.
#    stage == exit2 via the fixed transition wp3 -> exit2:
#      informed=True  -> stranded now, stops where it is.
#      informed=False -> walks to exit2, stranded on arrival.
#      Returned by exit2.close (informed=True); on exit2.open() it walks
#      on to exit2 from where it stopped, or the user calls
#      agent.follow(...).
# 5. No open target left:
#    rule at wp2 is RoundRobin({exit1: 1, exit2: 1}); both exits closed ->
#    agents deciding at wp2 are stranded inside wp2; agents walking to
#    either exit are stranded where they are (informed) or at the exit.
#    exit1.open() -> they re-run the decision at wp2 and go to exit1.
# 6. Switching routes or steering directly:
#    agent.follow(route_b) -> stage == wp2 (first stage of route_b);
#    agent.steer_to(location) -> target == location, no route.
#    Both clear held, stranded and arrived, so an agent leaves STAY this
#    way; counters of route_c are not changed.

# --- Unchanged from master --------------------------------------------
# sim.iterate(count=1), sim.agent_count(), sim.elapsed_time(),
# sim.delta_time(), sim.iteration_count(), sim.close()
# sim.agents_in_range(pos, distance), sim.agents_in_polygon(polygon): take
#   plain (x, y) coordinates and return agent handles
# sim.get_geometry(), sim.set_tracing(), sim.timer
# agent.id, agent.position, agent.location, agent.route_orientation,
#   agent.state
# Operational models, their agent states and the trajectory writers

# --- From the current API on master to 2.0 ----------------------------
# Handles are used for object references; IDs remain available for lookup
# and serialization (agent.id, stage.id, region.id match trajectory files).
#
# Current API on master                     2.0
# add_waypoint_stage(...) -> int            sim.add_waypoint(...) -> Waypoint
# add_exit_stage(...) -> int                sim.add_exit(...) -> Exit
# add_direct_steering_stage() -> int        removed: steer_to= / agent.steer_to()
# add_journey(JourneyDescription) -> int    sim.add_route([...]) -> Route
# add_agent(journey_id, stage_id, ...)      sim.add_agent(location, route=,
#                                               start=)
# switch_agent_journey(agent, journey,      agent.follow(route, start=)
#     stage)
# get_location(x, y, region_id=)            region.at(x, y)
# add_region(...) -> int                    surface.add_region(...) -> Region
# connect_regions(...) -> int               surface.connect_regions(...) -> Region
# mark_agent_for_removal(agent_id)          agent.remove(), see "Lookup and removal"
# removed_agents() -> list[int]             sim.removed_agents() -> records
#                                           with .id, .exit, .time
# stage_ids() -> list[int]                  sim.stages -> tuple of handles
# get_stage(stage_id)                       sim.stage("exit1") or
#                                           sim.stage(id=5)
# agent(agent_id)                           sim.agent(17), lookup by ID kept for
#                                           matching trajectory files
# agents() -> iterator                      sim.agents() -> tuple of handles
# travel_times(stage_id) -> {int: grid}     sim.travel_times(exit1)
#                                           -> {Region: grid}
# set_default_speed_field(region_id, ...)   sim.set_default_speed_field(floor0, ...)
# set_custom_speed_field(stage_id,          sim.set_custom_speed_field(exit1,
#     region_id, ...)                           floor0, ...)
# custom_speed_fields(stage_id)             sim.custom_speed_fields(exit1)
# clear_custom_speed_field(stage_id,        sim.clear_custom_speed_field(exit1,
#     region_id)                                floor0)

# --- Open questions ---------------------------------------------------
# Not part of the specification yet.
#
# 1. Starting with a rule. Rules fire only on completing a stage, so a
#    split at the spawn needs a waypoint there. Since rules carry no
#    source, start= could accept a rule, evaluated in add_agent() and
#    follow() with one counter per route:
#        sim.add_route(start=jps.RoundRobin({exit1: 1, exit2: 1}))
#        sim.add_route(start=jps.LeastTravelTime([exit1, exit2]))
#    Open: how update_weights() addresses the start rule, and whether a
#    target that cannot be reached from the agent raises or is skipped.
# 2. Stage completions per iteration. removed_agents() reports exits only.
#    A record per completed stage (agent id, stage, time) would support
#    counts and controllers; removed_agents() is then its exit subset. A
#    stored history per agent is a separate commitment and is deferred.
# 3. Waiting. Three behaviours are possible: stop where the area is
#    entered, remain within the area, or walk to assigned waiting
#    positions. Proposal: hold() suspends route progression and the
#    waypoint's arrival behaviour decides where the agent waits, so hold()
#    (temporary, lifted by release()) and STAY (terminal, left through
#    follow() or steer_to()) behave alike. Neither makes agents walk
#    farther inside or spread out; that needs a waiting-area policy that
#    says how positions are chosen.
# 4. Areas of a stage. A stage with several areas is one destination: a
#    removal record cannot tell which door was used. Doors that need their
#    own closure and counts are separate exits. Area identity is added only
#    if grouping and reporting per door are needed together.
# 5. Who owns the geometry. #1724 replaces the native WalkableSurface by a
#    GeometryBuilder whose build() returns an immutable Geometry that
#    simulations share, with all validation in C++. The Python layer of
#    #1722 keeps its own copy of every region polygon and checks points and
#    stage areas a second time; the two validators can disagree. Proposal:
#    C++ owns the geometry and its validation, Python keeps identity only
#    (owner, id, name) and asks C++: contains(region, x, y), the regions at
#    a point, the height, whether a region is a connector. This needs
#    region ids that stay the same across build(), and names passed to
#    C++, so that an error is complete where it is raised.
#    Open: whether the class is called WalkableSurface or GeometryBuilder;
#    whether region.at() is checked on the builder or only on the built
#    Geometry (handles are then usable for locations after build());
#    whether errors raised in C++ arrive as ValueError and TypeError, as
#    the Errors section promises, or as SimulationError; and how the
#    statements on copying the surface and on regions added after the
#    simulation was built change once the Geometry is immutable.
# 6. Zero speeds as closures. Proposal: a speed field may be exactly 0 on
#    a cell, which makes the cell unwalkable for routing, and a helper
#    returns the cells of a region's grid covered by a polygon or by a
#    point with a radius, e.g. as a boolean mask, so that
#    values[mask] = 0 closes a door. This covers what close() cannot: a
#    blocked corridor or a smoke barrier makes an exit unreachable for
#    some agents only. With one exit of two doors, zeroing one door sends
#    everybody to the other without rules or a second route.
#    It does not cover informed=False (a floor field reroutes all agents
#    at once), rules that ignore travel time (RoundRobin and fixed
#    transitions would have to skip targets that cannot be reached), which
#    door was used (question 4) and weighted splits.
#    Open: what zero means for an agent standing on such a cell, for an
#    agent already assigned to the exit and for removal in a zeroed exit
#    area; that later targets are then no longer always reachable, so
#    agents can be stranded by a speed field; the cost of solving floor
#    fields again on every change; and whether exit.close() stays as the
#    call that states the intent and is built on this mechanism.
