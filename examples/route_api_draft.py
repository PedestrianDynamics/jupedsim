# Draft (sketch, not runnable: names like model and x1 are placeholders)
# ruff: noqa: F821
# - Stages are owned by the simulation and returned as handles.
# - Routes are built and validated in one call: either a valid route or an error.
# - One way to do each thing; explicit keywords; errors are informative say how to fix the call.


import jupedsim as jps

# --- Regions ----------------------------------------------------------
# Regions are returned as handles, like stages, not as bare ints.
# `name` is optional and used in reprs and error messages.
surface = jps.WalkableSurface()
floor0 = surface.add_region(polygon=floor0_polygon, height=0.0, name="floor0")
floor1 = surface.add_region(polygon=floor1_polygon, height=3.0, name="floor1")
# A connector (staircase, ramp) is a region of its own: connect_regions returns
# its handle. edge0 lies on floor0's boundary and edge1 on floor1's; each is
# a line segment lifted to its region's height. The quad they span must be
# planar: the edges may be skewed but not twisted.
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
surface.regions_at((x, y))  # every region covering a point

# Locations: region.at(x, y) returns a Location (x, y, z, region). It is
# checked against the surface right away and works in any simulation built
# from this surface. z is the region's height, interpolated on a staircase.
# regions_at is a lookup (which regions cover this point?); at() builds the
# location in a region you name:
surface.regions_at((5, 2))  # (floor0, floor1): stacked floors
floor1.at(5, 2)  # Location(5.0, 2.0, 3.0) in floor1

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
# the stage; an area whose clipped part is empty is an error.
# Shorthands: a single polygon, or a point with `radius` (a disc-shaped
# area). The region is inferred from the regions the area intersects
# (non-empty intersection): exactly one gives that region; several (stacked
# floors, or an area straddling adjacent regions) require region=.
# Agent positions and target points are locations: region.at(x, y), or a
# plain (x, y) whose region is inferred by the same rule.
# Coordinates are in metres, in the same frame as the geometry.
#
# Stage grouping sets what close() and hold() act on together: one exit
# with two doors closes both doors. Create separate exits for doors that
# must close individually.
#
# Routing to areas uses floor fields (see "Routing backend" below): agents
# move toward the area along paths of least travel time, which need not be
# the geometrically nearest part. The stage is reached once the agent is
# inside an area. Direct steering targets a point and uses no floor field.
#
# `name` is optional and used in reprs and error messages; unnamed stages
# appear as Waypoint(id=3). The messages below assume named stages.
wp1 = sim.add_waypoint((x1, y1), radius=0.5, name="wp1")
wp2 = sim.add_waypoint((x2, y2), radius=0.5, name="wp2")
wp3 = sim.add_waypoint((x3, y3), radius=0.5, name="wp3")
door_polygon = [(), (), (), ()]
wp4 = sim.add_waypoint(door_polygon, name="wp4")
doors = sim.add_waypoint(
    [(door_a, floor0), (door_b, floor0)], name="doors"
)  # either door completes it
exit1 = sim.add_exit(exit_polygon1, name="exit1")
exit2 = sim.add_exit(exit_polygon2, name="exit2")
main_exits = sim.add_exit(
    [(exit_polygon3, floor0), (exit_polygon4, floor0)], name="main_exits"
)  # main_exits.close() closes both doors; exit1, exit2 close one at a time

# --- Routes -----------------------------------------------------------
# A route is a directed graph of stages. It is bound at construction to the
# simulation that owns its stages and cannot be used in another simulation.
#
# Validation: from every stage of the route, an exit or STAY must be
# reachable via weights > 0 (agents may enter at any stage via start=).
# This is checked at construction and on every update_weights(); exit
# closures are exempt because they are temporary by intent.
# The check is structural: it does not guarantee that every agent
# eventually leaves a loop.

# Linear (shorthand): a DIRECTED list of stages, walked in order.
# The last stage must be an exit or jps.STAY, unless endless=True.
route_a = jps.Route([wp1, wp2, exit1])
route_b = jps.Route([wp2, wp3, exit2])
route_x = jps.Route([exit1])  # minimal evacuation route: walk to exit1

# Branching (general form): a list of (from, to) pairs; order does not matter.
# Each stage has exactly one outgoing transition: a stage or a rule.
# `start` is the default start stage for agents on this route.
route_c = jps.Route(
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
route_e = jps.Route([wp1, wp2, wp3, jps.STAY])

# Loops are allowed if an exit or STAY stays reachable from every stage.
route_f = jps.Route(
    [
        (wp1, wp2),
        (wp2, jps.RoundRobin({exit1: 2, exit2: 1, wp3: 1, wp4: 4})),
        (wp3, jps.STAY),
        (wp4, wp1),
    ],
    start=wp1,
)

# Endless loops (tours, single-file) must be explicit.
# endless=True means the route never ends: a reachable exit or STAY is an error.
# In a linear list, repeating the first stage at the end closes the loop.
patrol = jps.Route([wp1, wp2, wp3, wp4, wp1], endless=True)
tour = jps.Route(
    [
        (wp1, wp2),
        (wp2, jps.RoundRobin({wp3: 1, wp4: 1})),
        (wp3, wp1),
        (wp4, wp1),
    ],
    start=wp1,
    endless=True,
)

# Weights: non-negative integers, at least one > 0.
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
route_n = jps.Route(
    [
        (wp1, jps.LeastTravelTime([exit1, exit2])),
    ],
    start=wp1,
)

# --- Agents -----------------------------------------------------------
# `start` is the agent's first target; agent.stage is always the stage the
# agent is heading for. An agent spawned inside its start stage's arrival
# region reaches it in the first iteration and goes on to the next stage.
# `start` may be an exit: the agent walks to it and is removed on reaching it.
agent1 = sim.add_agent(pos1, route=route_a)  # starts at wp1
agent2 = sim.add_agent(pos2, route=route_b)  # starts at wp2
agent3 = sim.add_agent(
    pos3, route=route_c
)  # starts at the route's default (wp1)
agent4 = sim.add_agent(pos4, route=route_c, start=wp2)  # overrides the default
agent5 = sim.add_agent(pos5, target=(x, y))  # direct steering, no route
# Steering to a location on another floor walks there over the connecting
# staircase.
agent7 = sim.add_agent(floor0.at(x0, y0), target=floor1.at(x, y))
agent6 = sim.add_agent(pos6, route=route_x)  # walks to exit1

# --- Runtime changes --------------------------------------------------
# Bound the loop by time: with holds or closures, agents may wait for a
# later release or reopening, so "no agent is moving" is not a stop
# condition.
while sim.agent_count() > 0 and sim.elapsed_time() < 600:
    sim.iterate()

    # Direct steering: change the target with steer_to(); also switches a
    # route agent to direct steering.
    agent5.steer_to((x_new, y_new))
    agent7.steer_to(floor1.at(x_new, y_new))  # location on floor1
    # route -> direct steering; route and stage become None
    agent2.steer_to((x_new, y_new))

    # Routes are changed with follow() only (route and start in one call).
    agent1.follow(route_b)  # starts at the first stage, wp2
    agent1.follow(route_b, start=wp3)  # or an explicit start
    agent3.follow(route_c, start=wp2)
    # direct steering -> route; route == route_a, stage == wp1
    agent5.follow(route_a)

    # Weights: a patch, like dict.update; targets not named keep their weight.
    # Validated against the whole route and applied atomically: on error the
    # old weights and the old counter are kept. On success the counter of
    # (route_c, wp2) resets. Takes effect at the next decision; agents
    # already assigned keep their target.
    route_c.update_weights(wp2, {wp3: 0})  # wp3: 0, exit1: 2, exit2: 1
    print(route_c.weights(wp2))  # {wp3: 0, exit1: 2, exit2: 1}

    # Hold agents at a stage (e.g. phased evacuation), weights are kept.
    # Applies to all agents whose current target is wp2, including those
    # already inside it; agents that completed wp2 are not affected.
    # A held agent that follow()s another route is no longer held.
    route_c.hold(wp2)
    # Held agents go on in arrival order (ties by agent id), current rule.
    route_c.release(wp2)

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
    stranded = exit2.close(informed=True)  # agents stranded now
    # Valid target again. Agents stranded by this exit resume: they re-run
    # the decision at their last decision stage. Agents already rerouted
    # are not reassigned.
    resumed = exit2.open()  # agents that resumed

# --- State (read-only) ------------------------------------------------
# After the runtime block: agent2 is in direct steering, agent5 on route_a.
print(agent2.route, agent2.stage)  # None, None in direct steering
# Route(start=wp1, ...), Waypoint(id=1, name="wp1")
print(agent5.route, agent5.stage)
print(agent2.target)  # (x_new, y_new): the steering point in direct steering
print(agent5.target)  # None on a route: use agent5.stage
print(agent5.direction)  # unit vector the agent is currently steered along
print(agent3.held)  # True while held at a stage
# True after completing a waypoint whose transition is STAY
print(agent3.arrived)
print(agent3.stranded)  # True once the agent cannot continue (closed exit)
print(agent3.alive)  # False once the agent has left the simulation
print(route_c.held_at(wp2))  # agents held at wp2
# Excludes held, arrived and stranded agents. Not a stop condition when
# holds or closures are used (see the loop above).
sim.agent_count(moving=True)
sim.run_until(t=600)  # needed for endless routes

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
#   wp2 -> RoundRobin(wp3: 0, exit1: 2, exit2: 1)
#   wp3 -> exit2

# --- Two floors -------------------------------------------------------
# An agent upstairs leaves through an exit on the ground floor. The staircase
# is a region (from connect_regions), not a stage: the exit's floor field
# already routes over it.
exit_ground = sim.add_exit(ground_exit_polygon, name="exit_ground")  # floor0
evacuate = jps.Route([exit_ground])
# floor0 and floor1 overlap in plan, so the position names its region
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
via_landing = jps.Route([landing0, exit_ground])

# --- Routing backend: floor fields ------------------------------------
# Everything floor-field-specific is in this section; the route API above
# only needs "is the agent inside the stage's area" and "which
# direction leads there", plus travel times for LeastTravelTime.
# Configuration: jps.Simulation(model, surface,
#     floorfield_options=jps.FloorfieldOptions(cell_size=0.2,
#                                              wall_influence_radius=0.5))
sim.travel_times(exit1)  # {floor0: TravelTimeGrid, staircase: ..., floor1: ...}
sim.default_speed_fields()  # {floor0: SpeedGrid, staircase: ..., floor1: ...}
sim.set_default_speed_field(floor0, values)  # all stages without their own
sim.set_custom_speed_field(exit1, floor0, values)  # one stage only
sim.custom_speed_fields(exit1)  # {floor0: SpeedGrid, staircase: None, ...}
sim.clear_custom_speed_field(exit1, floor0)
# Speed fields change routes, not walking speed. Floor fields are solved
# when first needed and cost memory per stage and region, so stages are
# created once and shared by routes.

# --- Errors -----------------------------------------------------------
sim.add_waypoint(door_polygon, radius=0.5)
# ValueError: radius applies to point waypoints only
sim.add_waypoint((x1, y1), radius=0)
# ValueError: radius must be > 0
sim.add_waypoint(landing_polygon)  # lies where floor0 and floor1 overlap
# ValueError: area intersects floor0 and floor1; pass region=
sim.add_waypoint(door_polygon, region=floor1)  # door_polygon is not in floor1
# ValueError: area does not intersect floor1
sim.add_waypoint(door_polygon, region=1)
# TypeError: region must be a Region handle, e.g. surface.region("floor1")
surface.region("floor2")
# KeyError: no region named floor2; regions are floor0, floor1, staircase
surface.connect_regions(
    from_region=floor0,
    from_edge=edge0,
    to_region=floor1,
    to_edge=edge1_reversed,
)
# ValueError: edges span a twisted quad; reverse to_edge
sim.add_agent(pos5, target=(x, y))  # (x, y) on floor0 and floor1
# ValueError: (x, y) lies in floor0 and floor1; use floor0.at(x, y) or floor1.at(x, y)
floor1.at(100, 100)
# ValueError: (100, 100) is not in floor1
sim.add_waypoint(polygon_with_hole)
# ValueError: waypoint polygons must be simple and without holes
jps.Route([])
# ValueError: route is empty
jps.Route([(wp1, wp2), (wp2, wp3)])
# ValueError: wp3 has no next stage; end with an exit or add (wp3, jps.STAY)
jps.Route([(wp1, wp2), (wp2, wp3), (wp2, jps.RoundRobin({exit1: 2, exit2: 1}))])
# ValueError: wp2 has 2 outgoing transitions; combine them into one rule
jps.Route([(wp1, wp2), (wp2, wp1)])
# ValueError: no exit or STAY reachable; to loop on purpose, use endless=True
jps.Route([wp1, wp2, exit1], endless=True)
# ValueError: route can end at exit1; drop endless=True
jps.Route([wp1, wp2, wp1, exit1])
# ValueError: wp1 appears twice; only repeating the first stage at the end closes a loop
jps.Route([exit1, wp1])
# ValueError: exits are terminal; remove (exit1, wp1)
jps.Route([wp1, other_sim_wp, exit1])
# ValueError: all stages must belong to the same simulation
jps.RoundRobin({exit1: 0.6, exit2: 0.4})
# ValueError: weights must be integers; for probabilities use jps.Random({...}, seed=...)
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
route_g = jps.Route(
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
route_c.release(wp1)
# ValueError: wp1 is not held
sim.add_agent(pos, route=route_a, target=(x, y))
# ValueError: use route= or target=, not both
sim.add_agent(pos, route=jps.Route([(wp1, wp2), (wp2, exit1)]))
# ValueError: route given as pairs has no default start; pass start= here or to Route
sim.add_agent(pos, route=route_c, start=wp4)
# ValueError: wp4 is not part of this route
other_sim.add_agent(pos, route=route_c)
# ValueError: route belongs to another simulation
agent1.route = route_c
# AttributeError: use agent.follow(route, start=...)
agent5.target = (x_new, y_new)
# AttributeError: use agent.steer_to(point)
agent1.stage  # after agent1 left through an exit
# RuntimeError: Agent(id=1) has left the simulation (agent.alive is False)

# --- Lifecycle of one agent -------------------------------------------
# 1. Spawned inside its first waypoint:
#    sim.add_agent(pos_in_wp1, route=route_c)
#    stage == wp1 -> first iteration: wp1 reached -> stage == wp2.
# 2. Weighted branch:
#    reaches wp2 -> counter of (route_c, wp2) picks the next slot, e.g.
#    exit1 -> stage == exit1; the assignment is kept until exit1 or a
#    change (follow, close with informed=True).
# 3. Held:
#    route_c.hold(wp2) while stage == wp2 -> agent stops inside wp2,
#    held == True, not counted as moving. route_c.release(wp2) -> served
#    in arrival order by the current rule -> held == False.
# 4. Exit closed on the way:
#    stage == exit2, assigned by the rule at wp2:
#      exit2.close(informed=True)  -> reassigned now to exit1.
#      exit2.close(informed=False) -> walks to exit2, reassigned there.
#    stage == exit2 via the fixed transition wp3 -> exit2:
#      informed=True  -> stranded now, stops where it is.
#      informed=False -> walks to exit2, stranded on arrival.
#      Returned by exit2.close (informed=True); resumes on exit2.open(),
#      or the user calls agent.follow(...).
# 5. No open target left:
#    rule at wp2 is RoundRobin({exit1: 1, exit2: 1}); both exits closed ->
#    agents deciding at wp2 are stranded inside wp2; agents walking to
#    either exit are stranded where they are (informed) or at the exit.
#    exit1.open() -> they re-run the decision at wp2 and go to exit1.
# 6. Switching routes:
#    agent.follow(route_b) -> stage == wp2 (first stage of route_b);
#    held/stranded are cleared; counters of route_c are not changed.

# --- From 1.0 to 2.0: no int IDs in calls -----------------------------
# IDs remain as attributes (agent.id, stage.id, region.id) to match
# trajectory files, but no call takes or returns a bare int.
#
# 1.0                                       2.0
# add_waypoint_stage(...) -> int            sim.add_waypoint(...) -> Waypoint
# add_exit_stage(...) -> int                sim.add_exit(...) -> Exit
# add_direct_steering_stage() -> int        removed: target= / agent.steer_to()
# add_journey(JourneyDescription) -> int    jps.Route([...])
# add_agent(journey_id, stage_id, ...)      sim.add_agent(pos, route=, start=)
# switch_agent_journey(agent, journey,      agent.follow(route, start=)
#     stage)
# get_location(x, y, region_id=)            region.at(x, y)
# add_region(...) -> int                    surface.add_region(...) -> Region
# connect_regions(...) -> int               surface.connect_regions(...) -> Region
# mark_agent_for_removal(agent_id)          agent.remove()
# removed_agents() -> list[int]             sim.removed_agents() -> agents with
#                                           alive == False
# stage_ids() -> list[int]                  sim.stages -> tuple of handles
# get_stage(stage_id)                       sim.stage("exit1"), by name
# agent(agent_id)                           sim.agent(17), lookup by ID kept for
#                                           matching trajectory files
# travel_times(stage_id) -> {int: grid}     sim.travel_times(exit1)
#                                           -> {Region: grid}
# set_default_speed_field(region_id, ...)   sim.set_default_speed_field(floor0, ...)
# set_custom_speed_field(stage_id,          sim.set_custom_speed_field(exit1,
#     region_id, ...)                           floor0, ...)
# custom_speed_fields(stage_id)             sim.custom_speed_fields(exit1)
# clear_custom_speed_field(stage_id,        sim.clear_custom_speed_field(exit1,
#     region_id)                                floor0)
agent1.remove()
sim.stages  # (Waypoint(id=0, name="wp1"), ..., Exit(id=8, name="exit_ground"))
sim.stage("exit1")  # Exit(id=5, name="exit1")
sim.agent(17)  # Agent(id=17)
