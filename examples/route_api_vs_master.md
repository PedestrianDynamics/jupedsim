# Route API: branch `route-api` compared with master

Sources:

- **Branch `route-api`**, on master `75ff51724` (with PR 1724, `GeometryBuilder`).
- **Draft:** `examples/route_api_draft.py` in PR 1717 (state of 2026-11-13). The branch
  implements it; section 4 lists where it deliberately differs.
- **Runnable tour:** `examples/route_api_overview.py` walks through the whole API in the order of
  the draft and prints the real error messages.
- **Train example:** `examples/route_api_train_platform_boarding.py`, a complete scenario (waiting
  area, two doors, a door that jams), with an optional viewer (`--viewer`).

These three files and this one are temporary and not meant to be merged.

## 1. Summary

- **Handles instead of ids.** Regions, targets, routes and agents are handles with `.id` and
  `.name`. Ids remain for lookup and serialization.
- **Routes instead of journeys.** A route is a list of transitions, each with its source. The
  entry is the transition from `jps.AGENT_LOCATION`. `add_route` checks the graph and either
  returns a valid route or raises.
- **No direct-steering stage.** An agent without a route is steered to a location:
  `steer_to=` in `add_agent` and `agent.reroute`.
- **One verb for changes at runtime:** `agent.reroute(route=, first_target=, steer_to=)`.
- **Events as records.** `sim.reached_targets()` and `sim.removed_agents()` return immutable
  records for the most recent `iterate()` call; current state is read from handles.
- **Regions everywhere.** Locations come from `region.at(x, y)`; target areas, polygon queries
  and speed fields take regions; travel times and speed fields are returned by region.

## 2. Changes by area

### 2.1 Geometry and regions

```python
# master
builder = jps.GeometryBuilder()
room = builder.add_region(polygon=p, height=0.0)  # -> int
sim = jps.Simulation(model=model, geometry=builder.build())
location = sim.get_location(x, y, region_id=room)
sim.get_geometry()

# route-api
builder = jps.GeometryBuilder()
room = builder.add_region(polygon=p, height=0.0, name="room")  # -> Region
stairs = builder.connect_regions(from_region=room, ..., name="stairs")  # -> Region
geometry = builder.build()
sim = jps.Simulation(model=model, geometry=geometry)
location = room.at(x, y)
location.region  # == room
geometry.region("room"), geometry.region(0), sim.geometry
```

- `region.at()` before `build()` raises `SimulationError: Region 0 (room) is not built yet;
  call build() first.` Handles of one region are equal, whether they come from the builder, from
  `geometry.region()` or from `location.region`.
- Region names are optional and unique in the geometry; they live in C++ (`GeometryBuilder`,
  `Geometry::region_name`).
- A geometry can be shared by several simulations. A region of another geometry is rejected
  with `ValueError: ... belongs to another geometry`.
- `sim.get_location` and `geometry.get_location` are gone; the trame viewer uses
  `geometry.region(id).at(x, y)` as well.

### 2.2 Targets (stages)

```python
# master
exit_id = sim.add_exit_stage([(polygon, room)])  # -> int
wp_id = sim.add_waypoint_stage([(polygon, room)])
steering_id = sim.add_direct_steering_stage()
sim.get_stage(exit_id), sim.stage_ids()

# route-api
exit1 = sim.add_target(
    jps.Exit([(polygon, room)]), name="exit1"
)  # -> RouteTarget
wp1 = sim.add_target(jps.Waypoint([(polygon, room)]), name="wp1")
exit1.description  # the jps.Exit it was created from, areas as given
sim.target("exit1"), sim.target(exit1.id), sim.targets()
```

- `jps.Waypoint` and `jps.Exit` are immutable descriptions. Areas are clipped to their region as
  on master.

### 2.3 Routes (journeys)

```python
# master
journey = jps.JourneyDescription([wp_id, exit_id])
journey.set_transition_for_stage(
    wp_id, jps.Transition.create_fixed_transition(exit_id)
)
journey_id = sim.add_journey(journey)

# route-api
route = sim.add_route(
    [
        jps.FixedTransition(jps.AGENT_LOCATION, wp1),
        jps.RoundRobinTransition(wp1, {exit1: 2, exit2: 1}),
    ],
    name="route",
)  # -> Route
sim.route("route"), sim.routes()
```

| master | route-api |
|---|---|
| `Transition.create_fixed_transition(t)` | `jps.FixedTransition(source, t)` |
| `Transition.create_round_robin_transition([(t, w), ...])` | `jps.RoundRobinTransition(source, {t: w, ...})` |
| `Transition.create_least_targeted_transition([t, ...])` | `jps.LeastTargetedTransition(source, [t, ...])` |
| first stage passed to `add_agent` | transition from `jps.AGENT_LOCATION` |

`add_route` rejects: an empty route; no or several transitions from `AGENT_LOCATION`; several
transitions from one target; parts not connected to the rest (directions ignored, so side entries
reached with `first_target=` are allowed); transitions leaving an exit; transitions back to
their own source (for all three kinds). Loops are allowed; a target without an outgoing
transition ends the route. Round robin keeps master's block order (2:1 → e1 e1 e2).

### 2.4 Agents

```python
# master
agent_id = sim.add_agent(
    journey_id=journey_id,
    stage_id=wp_id,
    position=(x, y),
    state=state,
    region_id=room,
)
sim.switch_agent_journey(agent_id, other_journey, stage_id)
sim.agent(agent_id).final_target = (x, y)  # with a direct steering journey
agent.journey_id, agent.stage_id, agent.position

# route-api
agent = sim.add_agent(
    room.at(x, y), route=route
)  # -> Agent; state defaults per model
agent = sim.add_agent(
    room.at(x, y), route=route, first_target=exit1, state=s, name="anna"
)
agent = sim.add_agent(room.at(x, y), steer_to=room.at(u, v))
agent.reroute(route=other)  # join, AGENT_LOCATION decides
agent.reroute(route=other, first_target=t)  # join at t
agent.reroute(first_target=t)  # jump within the current route
agent.reroute(steer_to=room.at(u, v))  # direct steering
agent.route, agent.current_target, agent.location, agent.name
```

- `first_target=` replaces the transition from `AGENT_LOCATION` and may be any target of the
  route. An agent added inside its first target has reached it; it moves on in `iterate()`.
- `state=` is optional for built-in models and required for custom models.
- Direct steering: arrived = same region and at most 0.05 m away; reported once.
- `agent.position` and `agent.final_target` are gone (`agent.location.xy`, `steer_to=`).
- Agent names are unique among living agents and free again once the agent has left.

### 2.5 Reporting

```python
# master
sim.removed_agents()  # -> [agent_id, ...]

# route-api
for record in sim.reached_targets():  # TargetReachedRecord
    record.agent_id, record.target, record.time, record.location
for removal in sim.removed_agents():  # RemovalRecord
    (
        removal.agent_id,
        removal.exit,
        removal.time,
    )  # exit is None after mark_agent_for_removal
```

Both cover the most recent `iterate(count=n)`; `reached_targets()` also the agents added since.
A removal record comes in the `iterate()` that removes the agent, at its beginning; until then
the agent is `agent.leaving`.

### 2.6 Lookup and queries

| master | route-api |
|---|---|
| `sim.agent(agent_id)` | `sim.agent(17)` or `sim.agent("anna")`; unknown → `KeyError` |
| `sim.agents()` | `sim.agents()`, `sim.agents(filter=lambda a: a.current_target == exit1)` |
| `sim.get_stage(id)`, `sim.stage_ids()` | `sim.target(...)`, `sim.targets()` |
| (none) | `sim.route(...)`, `sim.routes()` |
| `sim.agents_in_range((x, y), d)` | `sim.agents_in_range(room.at(x, y), d)`, only agents in plain sight, as the models see them |
| `sim.agents_in_polygon(polygon)` | `sim.agents_in_polygon(polygon, region=room)`, clipped to the region like a target area |
| `sim.mark_agent_for_removal(agent_id)` | `sim.mark_agent_for_removal(agent)` |
| `sim.get_geometry()` | `sim.geometry` |

Lookup errors carry no list of names (`no target named 'exit9'`); in a large building the list
would be unreadable.

### 2.7 Floor fields and speed fields

| master | route-api |
|---|---|
| `sim.travel_times(stage_id)` → `{int: grid}` | `sim.travel_times(exit1)` → `{Region: grid}` |
| (none) | `sim.travel_time(exit1, location)`: seconds from a location, `inf` if unreachable |
| `sim.default_speed_fields()` → `{int: grid}` | `sim.default_speed_fields()` → `{Region: grid}` |
| `sim.set_default_speed_field(region_id, v)` | `sim.set_default_speed_field(room, v)` |
| `sim.set_custom_speed_field(stage_id, region_id, v)` | `sim.set_custom_speed_field(exit1, room, v)` |
| `sim.custom_speed_fields(stage_id)` | `sim.custom_speed_fields(exit1)` → `{Region: grid or None}` |
| `sim.clear_custom_speed_field(stage_id, region_id)` | `sim.clear_custom_speed_field(exit1, room)` |
| HDF5: column `stage_id`, `loader.load_travel_time_stage_ids()` | column `target_id`, `loader.load_travel_time_target_ids()` |

### 2.8 Errors

Checks in Python raise `TypeError` (wrong type), `ValueError` (invalid value or combination) and
`KeyError` (unknown name or id). Errors found in C++ are still `SimulationError`; mapping them to
Python error types is planned as a separate PR. The messages say how to fix the call, e.g.
`No transition leaves AGENT_LOCATION; add one, e.g. jps.FixedTransition(jps.AGENT_LOCATION,
...).` All messages: end of `route_api_overview.py`.

Messages name targets, routes and regions by their kind, id and, if there is one, name, so they
read the same with and without names: `Target 6 (main_exits) is not part of route 1 (route_a).`,
`Point (5, 5) is not in region 1`.

### 2.9 Unchanged from master

`sim.iterate(count)`, `agent_count()`, `elapsed_time()`, `delta_time()`, `iteration_count()`,
`close()`, `set_tracing()`, `timer`; `agent.id`, `agent.route_orientation`, `agent.state`; the
operational models, their states and the trajectory writers.

## 3. Inside

- C++: `Route` replaces `Journey`; `BaseStage` holds its `Destination` and name; the direct
  steering stage is gone. The strategic system records reached targets after each step (and in
  `add_agent`); it moves agents on at the beginning of the next iteration. Agent names live in a map in `Simulation`, as agents are copied every step.
- Bindings: `jps.Region` is native; a region handle and its builder share a scope that `build()`
  fills with the geometry. `Simulation::geo()` returns the shared geometry.
- The operational step is untouched, so parallelizing it is not affected.

## 4. Deliberate differences from the 1717 draft

1. Record names `TargetReachedRecord` and `RemovalRecord`: "record" marks an immutable
   snapshot, as opposed to a handle. A target is *reached* after a step (or by `add_agent`
   inside it); the agent moves on at the beginning of the next iteration, one target per step.
   An agent reaching an exit is `agent.leaving` and removed at the beginning of the next
   iteration, like a marked agent; its removal record comes then.
2. `sim.agents(filter=...)` instead of a separate `sim.filter_agents(predicate)`.
3. No lists of names in error messages, neither in lookups nor in `Target 2 (wp2) is not part
   of route 0 (route_a).`
4. Arrival in direct steering by distance (0.05 m, same region), not by the step segment
   described in open question 1 of the draft.
5. C++ errors are `SimulationError` for now, see 2.8.
6. `add_route` also rejects transitions leaving an exit and transitions back to their source.
7. `location.region_id` stays next to `location.region` (cheap; the HDF5 writer uses it).
8. Messages name targets, routes and regions as `Target 2 (wp2)` instead of only `wp2`: names
   are optional and chosen by the user, so the kind and id keep a message clear, see 2.8.
9. `sim.targets()` and `sim.routes()` are methods, like `sim.agents()`: they build new handles
   on every call.

## 5. Open

- Not implemented: the check `wp1 cannot be reached from Location(...)` in `add_agent`, and open
  question 2 of the draft (which area of a multi-area target was reached).
- Team decision: round robin order (blocks as on master, or smooth as in PR 1722).
- Whether `location.region_id` should stay public, and whether the HDF5 writer may keep using
  internals (`simulation._obj.agents()` already on master).
- Documentation prose (e.g. the table in `custom_serialization.rst` still lists
  `agent.position`): only code blocks are ported.
