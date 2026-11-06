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
# Polygon: a sequence of >= 3 (x, y) vertices or a shapely Polygon.
# Coordinates are in metres, in the same frame as the geometry.
# Agents steer to the centroid of a polygon waypoint.
wp1 = sim.add_waypoint((x1, y1), radius=0.5)
wp2 = sim.add_waypoint((x2, y2), radius=0.5)
wp3 = sim.add_waypoint((x3, y3), radius=0.5)
door_polygon = [(), (), (), ()]
wp4 = sim.add_waypoint(door_polygon)
exit1 = sim.add_exit(exit_polygon1)
exit2 = sim.add_exit(exit_polygon2)

# --- Routes -----------------------------------------------------------
# A route is a directed graph of stages, bound to one simulation.
# All its stages must belong to that simulation; it is bound on first use
# and cannot be used in another simulation.

# Linear (shorthand): a DIRECTED list of stages, walked in order.
# The last stage must be an exit or jps.STAY, unless endless=True.
route_a = jps.Route([wp1, wp2, exit1])
route_b = jps.Route([wp2, wp3, exit2])

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
# STAY is a terminal action, not a stage: on reaching wp3 the agent is
# arrived, agent.stage stays wp3, and it waits inside wp3's arrival region.
route_e = jps.Route([wp1, wp2, wp3, jps.STAY])

# Loops are allowed if an exit or STAY stays reachable (via weights > 0).
# This is a structural check only: it does not guarantee that every agent
# eventually leaves the loop.
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
agent1 = sim.add_agent(pos1, route=route_a)  # starts at wp1
agent2 = sim.add_agent(pos2, route=route_b)  # starts at wp2
agent3 = sim.add_agent(
    pos3, route=route_c
)  # starts at the route's default (wp1)
agent4 = sim.add_agent(pos4, route=route_c, start=wp2)  # overrides the default
agent5 = sim.add_agent(pos5, target=(x, y))  # direct steering, no route

# --- Runtime changes --------------------------------------------------
while sim.agent_count(moving=True) > 0:
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
    # Resets the counter of (route_c, wp2). Takes effect at the next decision;
    # agents already assigned keep their target.
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
    # Agents whose next target is the closed exit via a fixed transition are
    # stranded: they stop where they are (informed) or at the exit (not
    # informed). Reroute them with follow().
    stranded = sim.close(exit2, informed=True)  # returns the stranded agents
    sim.open(exit2)  # valid target again; rerouted agents are not reassigned

# --- State (read-only) ------------------------------------------------
print(agent2.route, agent2.stage)  # Route(...), Waypoint(id=2, pos=(x2, y2))
print(agent5.route, agent5.stage)  # None, None in direct steering
print(agent2.target)  # point the agent is heading for, in both modes
print(agent3.held)  # True while held at a stage
print(agent3.arrived)  # True once at a STAY stage
print(agent3.stranded)  # True if its next target is a closed exit
print(agent3.alive)  # False once the agent has left the simulation
print(route_c.held_at(wp2))  # agents held at wp2
sim.agent_count(moving=True)  # excludes held, arrived and stranded agents
sim.run_until(t=600)  # needed for endless routes

# Inspecting a route
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
# ValueError: wp4 is not a target of wp2; targets are wp3, exit1, exit2 (see route_c.weights(wp2))
route_c.weights(wp1)
# ValueError: wp1 has a fixed transition; use route_c.next(wp1)
route_c.next(wp4)
# ValueError: wp4 is not part of this route; stages are wp1, wp2, wp3, exit1, exit2
route_c.update_weights(wp2, {wp3: 0, exit1: 0, exit2: 0})
# ValueError: at least one target at wp2 needs weight > 0; to stop agents, use route_c.hold(wp2)
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
sim.add_agent(pos, route=route_c, start=exit1)
# ValueError: start is an exit; the agent would be removed immediately
other_sim.add_agent(pos, route=route_c)
# ValueError: route_c is bound to another simulation
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
#      stranded == True; stops (informed) or waits at exit2 (not informed);
#      returned by sim.close; user calls agent.follow(...).
# 5. Switching routes:
#    agent.follow(route_b) -> stage == wp2 (first stage of route_b);
#    held/stranded are cleared; counters of route_c are not changed.
