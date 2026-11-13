# Routing API: my proposal compared with PR 1717

Sources:

- **PR 1717:** `examples/route_api_draft.py` and its five examples, commit `eb26d5170`
  (2026-11-11, 18:57), and the discussion on the PR up to that time.
- **Proposal:** described in full in this document. `ralf_train_platform_boarding_example.py` shows
  it on a complete scenario (a train platform with a waiting area, two doors and a door that jams);
  "the train example" below refers to it.
- **Geometry:** the proposal builds on PR 1724 (`GeometryBuilder`, an immutable, shared `Geometry`),
  with one change: a builder can be built only once, see 2.1.

## 0. Scope: 2.0 and 2.1

What goes into 2.0, what comes later, and what is still open. Rows in the sections below refer to it.

**In 2.0**

- Stages and routes, with lookup; `RoundRobin` and `LeastTargeted`, which 1.x already has.
- Handles for regions, targets and routes.
- `sim.add_target()`, not `add_routing_target()`.
- `sim.add_route(list_of_transitions, name=...)`: each transition is one pair of a source and a
  target (or a rule) and states its kind. Verbose, but easy to read later and easy to generate.
  There is no shorthand: a `FixedTransition` always takes exactly two targets.
- The entry of this proposal, named `jps.START_TARGET`.
- No `jps.STAY` and no `endless` flag, without a warning.
- `add_route` checks that the route graph is connected.
- `agent.reroute(...)` for switching routes and for direct steering.
- `sim.travel_time(stage, location)`, so that a least-travel-time choice can be written by hand.
- Speed fields, including custom fields that block a corridor.
- No sub-targets for closing or opening parts of a target.

**In 2.1 or later**

- `route.update_weights()` and `LeastTravelTime`.
- `exit.close()` / `open()`. To be specified: whether this is based on routes, or on something like
  `agent.stop()`.
- `hold()` / `release()`: not in 2.0; whether and in which form is still under discussion.

**Still open**

- Reporting: which information is exposed all the time, and what users compute in their scripts.
  Reporting costs performance, so the suggestion is to start small and extend along common
  patterns. This concerns `agents_completed_target()` and the records of `removed_agents()`.

## 1. Summary

- **Less control machinery.** No `hold`/`release`, no `close`/`open`, no `STAY`, no `endless`
  (loops need no flag). Instead, the script reacts to events (`agents_completed_target()`) and redirects single
  agents with **one** verb (`agent.reroute(...)`).
- **One format for routes.** Transitions are objects that carry their source. The entry is a
  transition from `jps.START_TARGET`. This makes the linear list, the pairs, `start=` and `STAY`
  unnecessary as special forms.
- **C++ owns the geometry.** `builder.add_region()` returns a `Region` handle, like every other
  `add_*`. After `builder.build()`, `region.at()` returns a real `Location`, resolved on the mesh.
  Python keeps no copy of the polygons. This is what 1717 itself proposes in its open question 5,
  "Who owns the geometry".
- **Direct steering is not part of the graph,** but a variant of `reroute`.

## 2. Differences and reasons

### 2.1 Geometry and regions

```python
builder = jps.GeometryBuilder()
ground = builder.add_region(
    polygon=ground_polygon, height=0.0, name="ground floor"
)
upper = builder.add_region(
    polygon=upper_polygon, height=3.0, name="first floor"
)
stairs = builder.connect_regions(
    from_region=ground,
    from_edge=edge0,
    to_region=upper,
    to_edge=edge1,
    name="stairs",
)
geometry = (
    builder.build()
)  # once per builder; binds ground, upper and stairs to `geometry`
sim = jps.Simulation(model=model, geometry=geometry)
sim.add_agent(
    ground.at(2, 2), ...
)  # a real Location, checked against the mesh right here
```

| PR 1717 | Proposal | Reason |
|---|---|---|
| `surface = jps.WalkableSurface()`, `add_region(...)` returns a `Region` handle | `builder = jps.GeometryBuilder()`, `add_region(...)` returns a `Region` handle as well | This is a consequence of PR 1724 using `GeometryBuilder`. |
| `region.at(x, y)` checks against a Python copy of the polygons (shapely). The simulation checks again against the mesh when the location is used | `region.at(x, y)` goes straight to `Geometry::get_location`: one check in C++, and z is known at once. Before `build()`, `at()` raises immediately ("call build() first") | No second copy of the geometry, no two checks that can disagree at the boundary, and no deferred errors. In a script, locations are needed only after the simulation exists, i.e. after `build()`. |
| The simulation copies the surface. Regions added later are rejected | `build()` can be called **once** per builder. Afterwards the builder rejects further calls ("create a new one"). The `Geometry` is immutable and shared between simulations (`shared_ptr`) | If one builder could produce several geometries, a handle would not know which geometry `at()` should resolve on. A `Location` depends only on its geometry, not on a simulation, so it can be used in every simulation that shares that geometry. |
| `connect_regions(from_region=Region, ...)` | Same | — |
| Names live in Python | Names are stored in C++ | Errors can then be complete where they are raised. 1717 proposes this in its open question 5 as well. |

**How `build()` reaches all handles.** The builder and every handle it returns share one small state
object. `build()` stores the geometry there once, and all handles see it at once; no handle has to be
tracked or updated:

```cpp
/// Shared by a builder and all region handles it returned.
struct BuildState {
    std::shared_ptr<const Geometry> geometry;  // empty until build()
};

class Region {
    std::shared_ptr<const BuildState> _state;
    std::size_t _id;
    std::optional<std::string> _name;
public:
    Location at(double x, double y) const
    {
        if(!_state->geometry) {
            throw SimulationError("Region {} is not built yet; call build() first.", _id);
        }
        return _state->geometry->get_location(x, y, _id);
    }
};
```

- Lifetime: a handle keeps the state alive, the state keeps the geometry alive; the builder itself may
  be gone. A `Location` returned by `at()` is tied to its handle with `keep_alive<0, 1>`.
- Equality of handles: by `(state, id)`.
- A simulation checks that a handle belongs to it by comparing `region._state->geometry` with its own
  geometry.
- The class is bound in C++, so the errors of `at()` come from C++ and the Python layer stays a thin
  pass-through.

**Changes to PR 1724.** `build()` is no longer `const`, and a second call raises. The two tests on
reusing a builder (`GeometryBuilder.BuildLeavesTheBuilderReusable`,
`test_build_leaves_the_builder_reusable`) turn around. No example or notebook builds twice; for
variants (e.g. a building with and without an emergency staircase), a function fills a fresh builder
for each variant.

### 2.2 Targets (stages)

```python
waiting_area = sim.add_target(
    jps.Waypoint([(waiting_area_polygon, platform)]), name="waiting area"
)
door1 = sim.add_target(jps.Exit([(door1_polygon, platform)]), name="door 1")
type(door1)  # jps.RouteTarget, one handle class for every kind
```

| PR 1717 | Proposal | Reason |
|---|---|---|
| `sim.add_waypoint(area, region=, radius=, name=)`, `sim.add_exit(...)` | `sim.add_target(jps.Waypoint([...]), name=)`, `jps.Exit([...])` | One method; the kind is given by the object passed in, as with `Simulation(model=jps.CollisionFreeSpeedModel(...))` and `add_agent(state=jps.CollisionFreeSpeedModelState(...))`. This maps 1:1 to the C++ variant `StageDescription`. The name is `add_target`, not `add_routing_target`: there are no targets other than those for routing. |
| `Waypoint` and `Exit` are the handle classes | `jps.Waypoint` and `jps.Exit` are immutable values describing the target. The handle returned is a `jps.RouteTarget`, the same class for every kind | A description and a live handle are different things, so they get different names. Agent states work the same way: `jps.CollisionFreeSpeedModelState(...)` goes in, `agent.state` returns a handle of another class. |
| Shorthand: a point with `radius`, e.g. `sim.add_waypoint((x, y), radius=0.5, region=floor0)` | Not adopted: `jps.Waypoint([(jps.circle_to_polygon(center=(x, y), radius=0.5), floor0)])` | `jps.circle_to_polygon` exists already; a parameter that applies to one kind of area only is not needed. |
| Shorthand: a single polygon with `region=`, e.g. `sim.add_waypoint(door_polygon, region=floor0)` | Rather not adopted: only the list of `(area, region)` pairs, `jps.Waypoint([(door_polygon, floor0)])` | Saves two brackets in what is probably the most common case, but gives `jps.Waypoint` two signatures to tell apart. With keyword-only parameters, as for `add_region` (`polygon=` or `boundary=`), the two forms would be unambiguous: `jps.Waypoint(areas=[...])` or `jps.Waypoint(area=..., region=...)`. Not convinced yet. |

### 2.3 Routes

A route is defined by its transitions only; the nodes are the targets that appear in them. The entry
is a virtual node, `jps.START_TARGET`, meaning "from where the agent currently is". The transition
leaving it decides an agent's first target, once, when the agent joins the route (`add_agent`,
`reroute(route=)`). `add_route` therefore needs no `first_target`. `add_agent` and `reroute` can still
override it with `first_target=`; the entry transition is then skipped.

```python
# Linear: wp1, wp2, then exit1
route_a = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.FixedTransition(wp2, exit1),
    ],
)

# Split right at the entry, 2:1, no helper waypoint needed
route_b = sim.add_route(
    [
        jps.RoundRobinTransition(jps.START_TARGET, {wp1: 2, wp2: 1}),
        jps.FixedTransition(wp1, exit1),
        jps.FixedTransition(wp2, exit1),
    ],
    name="split at the entry",
)

# Ends at an assembly point: a node without a successor is a valid end, no exit needed
route_c = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp1),
        jps.LeastTargetedTransition(wp1, [assembly1, assembly2]),
    ],
)

# A circle, e.g. for single-file
ring = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, wp0),
        jps.FixedTransition(wp0, wp1),
        jps.FixedTransition(wp1, wp2),
        jps.FixedTransition(wp2, wp0),
    ],
)

sim.add_agent(
    platform.at(2, 2), route=route_b
)  # the entry decides via RoundRobin: wp1 or wp2
sim.add_agent(
    platform.at(3, 2), route=route_b, first_target=wp2
)  # skips the entry transition
agent.reroute(first_target=exit1)  # jump within the current route
```

The list of transitions is the only positional parameter of `add_route`; `name=` is keyword-only.
A possible later extension, not needed for 2.0: an optional `name=` on each transition, so that code
can refer to a transition, e.g. `route["distribution"].update_weights(...)`.

`add_route` checks:

1. Exactly one transition leaves `START_TARGET`.
2. At most one transition leaves each node.
3. `START_TARGET` is never the target of a transition.
4. The graph, `START_TARGET` included, is connected when edge directions are ignored. Separate parts
   share neither edges nor round-robin state, so they belong in a route of their own. A node that cannot be
   reached from `START_TARGET` but is connected to the rest is allowed: example is a side entry, used with
   `first_target=`.

| PR 1717 | Proposal | Reason |
|---|---|---|
| Two forms: a linear list `[wp1, wp2, exit1]` or pairs `(from, stage_or_rule)` | One form: a list of transitions, `sim.add_route([...], name=...)`; each transition carries its source and its kind | No switching between two forms. In the linear form of 1717, `STAY` sits among the nodes as if it were one; in the pair form it is a rule. |
| `start=` is always a stage. Rules fire only on completing a stage (1717's open question 1, "Starting with a rule"; see section 3) | Exactly one transition from `jps.START_TARGET` | A split at the spawn needs no helper waypoint. A route without an entry is an error when it is added, not later in `add_agent`. |
| Linear list as a shorthand | None: every transition is one pair, so a linear route over three targets takes three `FixedTransition`s | One form only. Verbose, but easy to read later and easy to generate. |
| Loops are allowed if an exit or `STAY` stays reachable. `endless=True` for tours | Loops are allowed, without a reachability check and without a flag, as on master | No exit is required at the end, so there is nothing for a reachability check or `endless` to protect. Circles were allowed on master already, e.g. by `notebooks/single-file.ipynb`. |
| `STAY` marks an explicit end. A missing end is an error | A node without an outgoing transition is the end; the agent stays inside its area. With the collision-free speed models it keeps walking inside the area, as on master | `STAY` does not change behaviour compared with a transition to the stage itself. In 1717 it serves the `arrived` flag and the distinction from `hold`; the proposal has neither. Arrival is reported by `agents_completed_target()`. |
| One round-robin counter per (route, stage). Rules are independent values | Same | Unchanged. To document: `LeastTargeted` counts globally, over all routes. |

### 2.4 Direct steering

| PR 1717 | Proposal | Reason |
|---|---|---|
| `add_agent(..., steer_to=loc)`, `agent.steer_to(loc)` | `add_agent(..., steer_to=loc)`, `agent.reroute(steer_to=loc)` | Same parameter. 1717 adopted `steer_to=` from this proposal in `eb26d5170` (it was `target=`). |
| Arrival within 0.05 m of the last step, then `arrived = True` | Never completed (as on master) | Direct steering is deliberately not part of the graph. A target per agent cannot be a node, because no transition could set the point. Whether arrival should be reported is open, see section 5. |
| `agent.target` is the steering point, `None` on a route. The stage is in `agent.stage` | `agent.target` returns a `RouteTarget` or a `Location`, read-only | One getter for "where is the agent heading now". It reads the stage, not `final_target`, so it is correct right after `reroute`. |

### 2.5 Agents

| PR 1717 | Proposal | Reason |
|---|---|---|
| `agent.follow(route, start=)` and `agent.steer_to(loc)` | **One** verb: `agent.reroute(route=, first_target=)`, `agent.reroute(first_target=)` (within the current route) and `agent.reroute(steer_to=)` | Same parameters as `add_agent`, same rules, one shared validation. |
| `follow()` always restarts, also on the agent's own route | `reroute(first_target=)` without `route=` jumps within the current route | Overriding within the route is the common case, e.g. when a door jams. It is not a setter, because it can raise, changes how many agents count as targeting each stage, and takes effect in the next step. |
| `start=` overrides at the start | `first_target=` can be any node of the route except `START_TARGET`. The entry transition is then skipped | Agents can enter a route at any node, e.g. at a side entry (see 2.3), and the same parameter lets `reroute` jump within the current route. Note: these agents do not take part in a round robin at the entry, so its ratio applies only to the agents that enter without `first_target=`; this belongs in the docs. |
| — | `agent.stop()` / `agent.resume()`, and the read-only `agent.stopped` | Holding per agent, at any time, on a route or in direct steering, instead of `hold` per (route, stage). See below. |
| `agent.route`: the route, `None` in direct steering | Same, read-only | — |
| — | `name=` for agents, `sim.get_agent(name)` | Consistent with all other `add_*` methods. |

**`agent.stop()` / `agent.resume()`**

Not in 2.0. Whether `exit.close()` / `open()` in 2.1 build on routes or on something like
`agent.stop()` is to be specified.

- `stop()` sets the agent's desired speed to zero, `resume()` restores it; this is already used in
  the web app. The agent slows down to a standstill as its model allows, and others can still push
  it. Every built-in model has a desired speed (`desired_speed` in Python, mostly `v0` in C++); a
  model implemented in Python (`CustomOperationalModel`) has none the simulation knows about, so it
  needs its own way to stop.
- Setting only the desired direction to zero is not enough: the collision-free speed models (all
  variants) and the anticipation velocity model then keep walking in their last direction at full
  speed.
- Only the movement is suspended, not the route. A stopped agent standing inside a target completes
  it: it is reported by `agents_completed_target()`, and an exit removes it.
- `stop()` and `reroute()` are independent. A stopped agent stays stopped until `resume()`, also after
  `reroute()`, so a waiting agent can be redirected without it starting to walk.
- Calling `stop()` on a stopped agent, or `resume()` on a moving one, does nothing.
- Caveat compared with `hold()`: when a script stops an agent because it completed a target, the
  rule at that target has already chosen the next target. With `hold()`, the choice is made later, at
  `release()`. This matters for rules that depend on the state at the moment of the decision, such as
  `LeastTargeted`. If needed, the script can choose again with `reroute(first_target=…)` when it calls
  `resume()`.

### 2.6 Events and queries

| PR 1717 | Proposal | Reason |
|---|---|---|
| 1717's open question 2, "Stage completions per iteration": one record per completed stage | `sim.agents_completed_target()` returns `{agent: target}` for the most recent step. An agent is reported in every step in which it is inside the target, e.g. while it stays at a target without a successor | This is what scripts react to (waiting area, jammed door). Which reporting to expose is still open, see section 0. |
| `removed_agents()` returns records with `.id`, `.exit`, `.time` | Open: as on master, or adopt the records of 1717 | For discussion. The same can be derived from `agents_completed_target()`, since an agent at an exit stays alive until the start of the next `iterate()`. The records are cheap to implement, though, and give the exit and the time without bookkeeping in the script. |
| — | Events cover the most recent step. Calling `iterate(n > 1)` misses events | A user error. The docstrings should say "the most recent iteration"; on master they say "since the last call of `iterate`". |

### 2.7 Control at runtime

| PR 1717 | Proposal | Reason |
|---|---|---|
| `route.hold(wp)` / `release(wp)` | Not in 2.0; whether and in which form is still under discussion. This proposal: dropped. Waiting can be implemented e.g. with `stop()` (keeps the route), with `reroute(steer_to=agent.location)` (keeps the spot and returns to it when pushed away, but leaves the route), or with spots inside a waiting area via `reroute(steer_to=…)` | Held agents stop where they enter the area and block the entrance. PR 1722 documents this itself ("Held agents stop where they first enter the waypoint area … They can block the entrance"). For a door that is acceptable, for a waiting area it is not (agreed in the PR discussion). Holding at a door works with `stop()`/`resume()` as well (see 2.5 and the example below), with one difference: the next target is chosen on arrival, not on release. |
| `exit.close(informed=)` / `open()`, agents become "stranded" | 2.1. Replacement in the train example: `reroute(first_target=door2)` | `informed`, `stranded` is not covered. Do we really need this? E.g. the train example works without it. |
| `route.update_weights(stage, {...})`, `route.weights(stage)` | 2.1. This proposal for then: with `jps.START_TARGET` as a valid stage, `route.update_weights(jps.START_TARGET, {wp1: 1, wp2: 0})` | Steering a split at runtime is useful, e.g. for a controller. Same semantics as 1717: a patch like `dict.update`; weight 0 allowed, at least one > 0; the state resets only if a weight changed; effective from the next decision. The re-check "an exit is still reachable" is not needed, since the proposal requires no exit. |
| `LeastTravelTime([...])` as a rule | 2.1. In 2.0: `sim.travel_time(target, location)` as a building block | Counting per door works with one exit with several areas plus `agent.location` from `agents_completed_target()`. The rule pays off only when the alternatives continue differently in the graph. |

The phased release from the PR discussion (rooms A and B share one staircase; room B waits at its
door for 60 s), written with `stop()`/`resume()`:

```python
route_a = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, door_a),
        jps.FixedTransition(door_a, exit_),
    ]
)
route_b = sim.add_route(
    [
        jps.FixedTransition(jps.START_TARGET, door_b),
        jps.FixedTransition(door_b, exit_),
    ]
)
# ... add the agents of room A to route_a and those of room B to route_b ...

released = False
while sim.agent_count() > 0 and sim.elapsed_time() < 600:
    sim.iterate()
    if sim.elapsed_time() < 60:
        for agent, target in sim.agents_completed_target().items():
            if target == door_b:
                agent.stop()  # its next target, exit_, is already chosen
    elif not released:
        released = True
        for agent in sim.agents():
            if agent.stopped:
                agent.resume()
```

Agents of room A are not affected. Since the next target is fixed here, it does not matter that the
agents resume all at once rather than in arrival order, as with `release()`.

### 2.8 Naming still under discussion

`steer_to=` in `add_agent`, `sim.add_target()` and `jps.START_TARGET` are settled; the rest of these
points is not:

| Question | 1717 | Proposal |
|---|---|---|
| The things a route consists of | **Stage** is the thing; "target" is a role ("what an agent is heading for right now"). A route also lists stages nobody is walking to yet | **Target** for the things (`add_target`, `RouteTarget`); a direct-steering `Location` is a target as well, just not one a route can contain |
| What an agent is heading for | `agent.stage` (on a route) and `agent.target` (the direct-steering location); the other one is `None` | One getter, `agent.target`: `RouteTarget` or `Location` |
| Where an agent enters a route | `start=` on `add_route`, `add_agent` and `follow`. Reads well with a rule (`start=jps.RoundRobin(...)`); can be misread as the spawn location, so the docs must say it is the first stage the agent walks to | `first_target=` on `add_agent` and `reroute` only. It is always a node, never a rule: a rule at the entry is a transition from `jps.START_TARGET`. `add_route` has no such parameter |

## 3. Answers to the open questions of 1717

| Question in 1717 | Answer of the proposal |
|---|---|
| 1. Starting with a rule | `jps.START_TARGET` as the source of a transition. Evaluated once per agent on joining (`add_agent`, `reroute(route=)`). The round-robin state belongs to the route. `update_weights` (2.1) would address the entry as `route.update_weights(jps.START_TARGET, {...})`. |
| 2. Completions per step | `agents_completed_target()`, see 2.6. To adopt from 1717: completions happen only in `iterate()`, not in `add_agent()`. |
| 3. Waiting | Spots via `reroute(steer_to=…)`, holding via `stop()`. A dedicated waiting-area target is deferred. |
| 4. Which area of a stage was used | Check `agent.location` from `agents_completed_target()` against the areas. No area identity is needed for this. |
| 5. Who owns the geometry | C++, see 2.1. The same as the proposal in 1717. Its open sub-questions: the class is `GeometryBuilder`; `region.at()` works only on the built `Geometry`; regions cannot be added after `build()`, because the builder is built once and the `Geometry` is immutable. Still open: whether errors from C++ arrive as `ValueError`/`TypeError` or as `SimulationError`. |
| 6. Zero speeds as closures | Not part of the proposal. From the PR discussion: a speed of 0 does not close an exit, because the exit area is the source of the floor field. Suggested instead: an open flag per area of an exit or waypoint, so that a closed area is no source of the floor field; custom speed fields for blocked corridors. No sub-targets for closing or opening parts of a target (section 0). |

## 4. What the proposal lacks: overview for an assessment

Effort is a rough estimate: **S** = Python or one place in C++, **M** = several places in C++ plus
semantics, **L** = a lot of semantics and edge cases. Where PR 1722 already has code, the effort is
lower, because it can be taken over.

| Feature in 1717 | Used for | Replacement in the proposal | Effort | Initial assessment |
|---|---|---|---|---|
| `sim.travel_time(stage, location)` | Building `LeastTravelTime` by hand, reachability checks | None, only the grids from `travel_times()` | S (the value is in the floor field already) | **Adopt** (2.0). |
| `LeastTravelTime` as a rule | Nearest target with different continuations | `travel_time` + `reroute(first_target=)`. The graph then needs a workaround (one route per alternative) | S (exists in 1722) | 2.1. |
| `route.update_weights(...)`, `weights()` | Steering the split at runtime | — | S–M (`set_round_robin_weights` and the smooth weighted round robin, which allows weight 0, exist in 1722; validation is the effort) | 2.1, see 2.7. |
| `route.next()`, `route.stages`, `repr` | Inspecting a route | None | S (Python only) | Cheap and useful. |
| `sim.agents_targeting(stage, route=)` | Who is heading where | `[a for a in sim.agents() if a.target == wp]`, filtered by route with `and a.route == route_c` | S | Not needed as long as Python is fast enough. |
| `removed_agents()` with `.exit` and `.time` | Counts and flow over time | `agents_completed_target()` in the matching step | S (all records of a step share the same time, so the simulation can add it; 1722 hands the clock to the exit instead, which is not needed) | Small, and useful for analysis after the run. |
| Arrival in direct steering (`arrived`) | Point reached | Check the distance by hand | S (`has_reached` in 1722) | Depends on section 5, item 1. |
| `STAY` + `arrived` | Explicit end, flag on the agent | Node without successor + `agents_completed_target()` | S | Do not adopt. |
| `hold` / `release` / `is_held` / `held_at` | Phased release | `stop()` / `resume()` per agent | M | Not in 2.0; this proposal: do not adopt (blocks the entrance). |
| `exit.close(informed=)` / `open()` / `closed`, "stranded" | Blocking exits | `reroute(first_target=…)` | L | 2.1. |
| Stage shorthand: a single polygon with `region=` | Shorter code | List with one pair | S | Rather not adopt, see 2.2. (The point with `radius` is not adopted.) |
| `surface.regions_at((x, y))`, `surface.regions`, `surface.region(name)`, `Region.height` | Exploring regions, e.g. of a geometry loaded from a file; interactive tools | None | S (`faces_at` exists in `Geometry`; names and heights are known to the builder and need to be passed on) | Cheap. A lookup by name on the `Geometry` is worth having. |
| `agent.alive`, `agent.remove()` | Handle after removal | `sim.mark_agent_for_removal(id)` as on master | S | Fits the handle model; rather adopt. |
| Catalogue of error messages | Guiding users | — | — | Write along while implementing. |

## 5. Open points in the proposal

1. **Arrival in direct steering.** `stop()` at a waiting spot needs to know that the spot was
   reached. Should `agents_completed_target()` report steered agents (with a `Location` as value), or
   should there be a flag on the agent, or does the user check the distance?
2. **Keys of `agents_completed_target()`:** `Agent` handle or id.
3. **Kind of a `RouteTarget`:** how does a script tell a waypoint from an exit, e.g. a `kind`
   property? `isinstance(handle, jps.Exit)` does not work, because `jps.Exit` is the description.
4. **Names of removed agents:** what does `sim.get_agent("agent1")` return afterwards? Suggestion:
   free the name and raise.
