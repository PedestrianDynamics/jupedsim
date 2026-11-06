# Draft
# - Stages are owned by the simulation and returned as handles.
# - Routes are built and validated in one call: either a valid route or an error.
# - One way to do each thing; explicit keywords; errors are informative say how to fix the call.


import jupedsim as jps

sim = jps.Simulation(model, geometry)

# --- Stages -----------------------------------------------------------
# A waypoint is a point with an arrival radius or a polygon (area).
# It counts as reached once the agent is within `radius` or inside the polygon.
wp1 = sim.add_waypoint((x1, y1), radius=0.5)
wp2 = sim.add_waypoint((x2, y2), radius=0.5)
wp3 = sim.add_waypoint((x3, y3), radius=0.5)
door_polygon = [(), (), (), ()]
wp4 = sim.add_waypoint(door_polygon)
exit1 = sim.add_exit(exit_polygon1)
exit2 = sim.add_exit(exit_polygon2)

# --- Routes -----------------------------------------------------------
# Linear: a DIRECTED list of stages, walked in order.
# The last stage must be an exit or jps.STAY, unless endless=True.
route_a = jps.Route([wp1, wp2, exit1])
route_b = jps.Route([wp2, wp3, exit2])

# Branching: a list of (from, to) pairs; order does not matter.
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
route_e = jps.Route([wp1, wp2, wp3, jps.STAY])

# Loops are allowed if an exit or STAY stays reachable (via weights > 0).
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
# Slots are interleaved, so only the ratio matters:
# {exit1: 3, exit2: 2} and {exit1: 60, exit2: 40} behave the same.
# - 3:2 gives e1 e2 e1 e2 e1 / repeat
# - 60:40 gives exactly the same sequence

# --- Agents -----------------------------------------------------------
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

    # Direct steering: the target is a single value and can be assigned.
    agent5.target = (x_new, y_new)

    # Routes are changed with follow() only (route and start in one call).
    agent1.follow(route_b)  # starts at the first stage, wp2
    agent1.follow(route_b, start=wp3)  # or an explicit start
    agent3.follow(route_c, start=wp2)
    agent5.follow(route_a)  # direct steering -> route

    # Weights: only the named targets change; the counter resets.
    # Takes effect at the next decision; assigned agents keep their target.
    route_c.set_weights(wp2, {wp3: 0})  # wp3: 0, exit1: 2, exit2: 1
    print(route_c.weights(wp2))  # {wp3: 0, exit1: 2, exit2: 1}

    # Hold agents at a stage (e.g. phased evacuation), weights are kept.
    route_c.hold(wp2)  # agents arriving at wp2 stop there
    route_c.release(wp2)  # held agents go on in arrival order, current weights

    # Close an exit for all routes (treated as weight 0).
    # informed=True:  agents heading there are rerouted now.
    # informed=False: agents walk to the exit and are rerouted there.
    # Agents with no open exit left are held at their last waypoint, informed or not.
    affected = sim.close(exit2, informed=True)  # returns the affected routes

# --- State (read-only) ------------------------------------------------
print(agent2.route, agent2.stage)  # Route(...), Waypoint(id=2, pos=(x2, y2))
print(agent3.held)  # True while held at a stage
print(agent3.arrived)  # True once at a STAY stage
print(route_c.held_at(wp2))  # agents held at wp2
sim.agent_count(moving=True)  # excludes held and arrived agents
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
jps.Route([wp1, wp2, wp1, exit1])
# ValueError: wp1 appears twice; only repeating the first stage at the end closes a loop
jps.Route([exit1, wp1])
# ValueError: exits are terminal; remove (exit1, wp1)
jps.RoundRobin({exit1: 0.6, exit2: 0.4})
# ValueError: weights must be integers; for probabilities use jps.Random({...}, seed=...)
route_c.set_weights(wp1, {wp2: 3})
# ValueError: wp1 has a fixed transition, not a weighted rule
route_c.set_weights(wp2, {wp4: 1})
# ValueError: wp4 is not a target of wp2; targets are wp3, exit1, exit2 (see route_c.weights(wp2))
route_c.weights(wp1)
# ValueError: wp1 has a fixed transition; use route_c.next(wp1)
route_c.next(wp4)
# ValueError: wp4 is not part of this route; stages are wp1, wp2, wp3, exit1, exit2
route_c.set_weights(wp2, {wp3: 0, exit1: 0, exit2: 0})
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
agent1.route = route_c
# AttributeError: use agent.follow(route, start=...)
