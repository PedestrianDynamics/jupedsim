# Draft (sketch, not runnable: names like model and x1 are placeholders)
# ruff: noqa: F821
# - Stages are owned by the simulation and returned as handles.
# - Routes are built and validated in one call: either a valid route or an error.
# - One way to do each thing; explicit keywords; errors are informative say how to fix the call.


import jupedsim as jps

sim = jps.Simulation(model, geometry)

# --- Stages -----------------------------------------------------------
# A waypoint is a point with an arrival radius or a polygon (area).
# It counts as reached once the agent is within `radius` or inside the polygon.
# Point: a pair of numbers or a shapely Point.
# Polygon: a simple polygon without holes, as a sequence of >= 3 (x, y)
# vertices or a shapely Polygon.
# Coordinates are in metres, in the same frame as the geometry.
# The agent navigates to the waypoint's arrival region. For a polygon it
# steers to a point guaranteed to lie inside it (not the centroid, which can
# lie outside a concave polygon).
# `name` is optional and used in reprs and error messages; unnamed stages
# appear as Waypoint(id=3). The messages below assume named stages.
wp1 = sim.add_waypoint((x1, y1), radius=0.5, name="wp1")
wp2 = sim.add_waypoint((x2, y2), radius=0.5, name="wp2")
wp3 = sim.add_waypoint((x3, y3), radius=0.5, name="wp3")
door_polygon = [(), (), (), ()]
wp4 = sim.add_waypoint(door_polygon, name="wp4")
exit1 = sim.add_exit(exit_polygon1, name="exit1")
exit2 = sim.add_exit(exit_polygon2, name="exit2")

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

    # Close an exit for all routes. Rules skip closed targets.
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
    stranded = sim.close(exit2, informed=True)  # agents stranded now
    # Valid target again. Agents stranded by this exit resume: they re-run
    # the decision at their last decision stage. Agents already rerouted
    # are not reassigned.
    resumed = sim.open(exit2)  # agents that resumed

# --- State (read-only) ------------------------------------------------
# After the runtime block: agent2 is in direct steering, agent5 on route_a.
print(agent2.route, agent2.stage)  # None, None in direct steering
# Route(start=wp1, ...), Waypoint(id=1, name="wp1")
print(agent5.route, agent5.stage)
print(agent2.target)  # point the agent is heading for, in both modes
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

# --- Errors -----------------------------------------------------------
sim.add_waypoint(door_polygon, radius=0.5)
# ValueError: radius applies to point waypoints only
sim.add_waypoint((x1, y1), radius=0)
# ValueError: radius must be > 0
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
# ValueError: exits cannot be held; use sim.close(exit1)
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
# RuntimeError: agent 1 has left the simulation (agent.alive is False)

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
#      sim.close(exit2, informed=True)  -> reassigned now to exit1.
#      sim.close(exit2, informed=False) -> walks to exit2, reassigned there.
#    stage == exit2 via the fixed transition wp3 -> exit2:
#      informed=True  -> stranded now, stops where it is.
#      informed=False -> walks to exit2, stranded on arrival.
#      Returned by sim.close (informed=True); resumes on sim.open(exit2),
#      or the user calls agent.follow(...).
# 5. No open target left:
#    rule at wp2 is RoundRobin({exit1: 1, exit2: 1}); both exits closed ->
#    agents deciding at wp2 are stranded inside wp2; agents walking to
#    either exit are stranded where they are (informed) or at the exit.
#    sim.open(exit1) -> they re-run the decision at wp2 and go to exit1.
# 6. Switching routes:
#    agent.follow(route_b) -> stage == wp2 (first stage of route_b);
#    held/stranded are cleared; counters of route_c are not changed.
