# Migrating to JuPedSim 2.0

JuPedSim 2.0 is a clean break: the model builder classes and the
`XModelAgentParameters` dataclasses have been **removed without deprecation
shims**. This guide maps every 1.x construct to its 2.0 replacement.

The four changes you will encounter in every script:

1. **Simulation construction** — every model is passed as a *configured
   instance* carrying its model-level parameters (e.g.
   `jps.SocialForceModel(body_force=..., friction=...)`). The
   `ModelType` enum has been removed.
2. **Adding agents** — `add_agent` now takes a `Location`, a `route` (or a
   `target` location) and `state`, where `state` is a directly
   instantiated per-model state object such as
   {class}`~jupedsim.CollisionFreeSpeedModelState`.
3. **Agent access** — {class}`~jupedsim.Agent` objects are lightweight
   *handles by id* that resolve the agent freshly on every attribute access.
4. **Regions** — everything placed in the walkable area (agents and stages)
   takes a required region handle. The walkable area is a
   {class}`~jupedsim.WalkableSurface` made of regions.

## Constructing a Simulation

Every built-in model is now constructed as an instance that carries its
model-level parameters. The `ModelType` enum no longer exists.

Simulation construction is **source-compatible** with 1.x for the built-in
models: `jps.Simulation(model=jps.CollisionFreeSpeedModel(...), geometry=area)`
is written the same way. What changed underneath: the model argument is no
longer a parameter-holding dataclass but the operational model itself, it is
consumed by the constructor (see the warning below), and its keyword arguments
are now the single place where model-level parameters are set.

The model instance holds the parameters that govern the model globally.
Constructed with their defaults they reproduce exactly the configuration that
the 1.x defaults produced, so existing simulations remain byte-identical:

```python
# Model-level parameters are keyword-only; defaults shown reproduce 1.x.
sim = jps.Simulation(
    model=jps.SocialForceModel(body_force=120000, friction=240000),
    geometry=area,
)

# CollisionFreeSpeedModel: the repulsion parameters are model-level.
sim = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(
        strength_neighbor_repulsion=8.0,
        range_neighbor_repulsion=0.1,
        strength_geometry_repulsion=5.0,
        range_geometry_repulsion=0.02,
    ),
    geometry=area,
)

# AnticipationVelocityModel: pushout strength and the RNG seed are model-level.
sim = jps.Simulation(
    model=jps.AnticipationVelocityModel(pushout_strength=0.3, rng_seed=1234),
    geometry=area,
)

# WarpDriverModel: the intrinsic field (sigma), the collision-prediction
# settings and the RNG are all model-level. All arguments are keyword-only.
sim = jps.Simulation(
    model=jps.WarpDriverModel(sigma=0.3, rng_seed=42), geometry=area
)
```

`CollisionFreeSpeedModelV2` and `CollisionFreeSpeedModelV3` keep all of their
repulsion parameters per-agent by design, so their instances take no
model-level arguments:

```python
sim = jps.Simulation(model=jps.CollisionFreeSpeedModelV2(), geometry=area)
sim = jps.Simulation(model=jps.CollisionFreeSpeedModelV3(), geometry=area)
```

```{warning}
Model instances are consumed by the `Simulation` constructor and must not
be reused for a second simulation.
```

Custom Python models are passed as an instance of a
{class}`~jupedsim.CustomOperationalModel` subclass:

```python
sim = jps.Simulation(model=MyCustomModel(), geometry=area)
```

Passing a wrong `model` argument raises `TypeError`.

## Adding agents

`add_agent` takes a location (`region.at(x, y)`), a `route` or a `target`
location, and a per-model state object. The agent spawns at the location.
All state constructors are
keyword-only and expose the per-agent fields of the model, with defaults
taken from the C++ implementation. Model-level parameters (see above) live on
the model instance and are *not* part of the state.

```python
# 1.x
agent_id = sim.add_agent(
    jps.CollisionFreeSpeedModelAgentParameters(
        journey_id=journey_id,
        stage_id=stage_id,
        position=(1.0, 1.0),
        desired_speed=1.4,
    )
)

# 2.0
agent = sim.add_agent(
    region.at(1.0, 1.0),
    route=route,
    state=jps.CollisionFreeSpeedModelState(desired_speed=1.4),
)
```

Passing a state class that does not match the simulation's model raises
{class}`~jupedsim.SimulationError`.

For custom models the state is your own object, of whatever type your
{class}`~jupedsim.CustomOperationalModel` expects (a frozen dataclass is
recommended):

```python
sim.add_agent(region.at(1.0, 1.0), route=route, state=MyState())
```

## Regions

The walkable area consists of regions, and regions may lie on top of each
other, e.g. the floors of a building. An `(x, y)` alone therefore does not
say where a place is: `add_agent`, `add_waypoint` and `add_exit` take a
region handle, and locations are built with `region.at(x, y)`.

A walkable area is a {class}`~jupedsim.WalkableSurface`; `add_region` and
`connect_regions` return region handles:

```python
# 1.x
exit_id = sim.add_exit_stage(exit_polygon)

# 2.0
surface = jps.WalkableSurface()
ground = surface.add_region(polygon=ground_outline, height=0.0)
upper = surface.add_region(polygon=upper_outline, height=3.0)
surface.connect_regions(
    from_region=ground, from_edge=foot_of_stairs,
    to_region=upper, to_edge=head_of_stairs,
)
sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)
exit_stage = sim.add_exit(exit_polygon, region=upper)
```

A location outside its region raises `ValueError`.

## Old → new mapping per model

| 1.x (deleted) | 2.0 `Simulation(model=...)` | 2.0 `add_agent` state |
|---|---|---|
| `CollisionFreeSpeedModel` + `CollisionFreeSpeedModelAgentParameters` | `jps.CollisionFreeSpeedModel(...)` instance | {class}`~jupedsim.CollisionFreeSpeedModelState` |
| `CollisionFreeSpeedModelV2` + `CollisionFreeSpeedModelV2AgentParameters` | `jps.CollisionFreeSpeedModelV2()` instance | {class}`~jupedsim.CollisionFreeSpeedModelV2State` |
| `CollisionFreeSpeedModelV3` + `CollisionFreeSpeedModelV3AgentParameters` | `jps.CollisionFreeSpeedModelV3()` instance | {class}`~jupedsim.CollisionFreeSpeedModelV3State` |
| `GeneralizedCentrifugalForceModel` + `GeneralizedCentrifugalForceModelAgentParameters` | `jps.GeneralizedCentrifugalForceModel(...)` instance | {class}`~jupedsim.GeneralizedCentrifugalForceModelState` |
| `SocialForceModel` + `SocialForceModelAgentParameters` | `jps.SocialForceModel(...)` instance | {class}`~jupedsim.SocialForceModelState` |
| `AnticipationVelocityModel` (dataclass) + `AnticipationVelocityModelAgentParameters` | `jps.AnticipationVelocityModel(rng_seed=...)` instance | {class}`~jupedsim.AnticipationVelocityModelState` |
| `WarpDriverModel` (parameter holder) + `WarpDriverModelAgentParameters` | `jps.WarpDriverModel(sigma=..., rng_seed=...)` instance | {class}`~jupedsim.WarpDriverModelState` |

Parameters that were *global* in 1.x (e.g. the repulsion strengths of the
Collision Free Speed Model, `body_force`/`friction` of the Social Force
Model, or `time_horizon`/`step_size` of the WarpDriver model) remain
*model-level*: they are keyword-only constructor arguments of the
corresponding model instance and are fixed for the lifetime of the
simulation. `CollisionFreeSpeedModelV2` and `CollisionFreeSpeedModelV3` are
the exception — by design they keep their repulsion parameters as *per-agent*
fields of the state class, where they can differ between agents and change
over time.

## Agent handles

{class}`~jupedsim.Agent` objects returned by `sim.agent(id)`,
`sim.agents()`, `sim.agents_in_range(...)` and `sim.agents_in_polygon(...)`
are now lightweight handles storing only the agent id. Every attribute read
and write resolves the agent freshly through the simulation:

- **No stale handles.** A handle stays valid across `iterate()` calls; it
  never points at reallocated memory. You can keep a handle for the whole
  simulation.
- **Removed agents raise.** Accessing any attribute of a handle whose agent
  has left the simulation raises `RuntimeError`; check `agent.alive`.
- **Property syntax keeps working, including mutation:**

```python
agent = sim.agent(agent_id)
agent.steer_to(region.at(10.0, 5.0))
agent.state.desired_speed = 0.8   # takes effect in the next iterate()

for _ in range(1000):
    sim.iterate()

print(agent.position)             # still valid, freshly resolved
```

`agent.state` returns a per-model state handle with the same semantics: it
resolves per access and raises once the agent is gone.

`agent.target` returns a {class}`~jupedsim.Location` instead of an `(x, y)`
tuple, and `None` while the agent follows a route; use `agent.stage` then.
It is read-only: set it with `agent.steer_to(location)`. A location outside
its region raises immediately, not at the next `iterate()`.

## Mutation during iterate() is now an error

Calling mutating simulation methods — `add_agent`,
`agent.remove()`, route or stage mutation — while `iterate()` is
running (e.g. from a custom-model callback or a trajectory writer invoked
during iteration) now raises {class}`~jupedsim.SimulationError` instead of
causing undefined behavior. Perform such mutations between `iterate()`
calls.

## Round-robin order

`jps.RoundRobin` splits agents by smooth weighted round-robin: with the
weights 3 and 2 the agents take e1 e2 e1 e2 e1, where 1.x sent them in
blocks (e1 e1 e1 e2 e2). Each cycle has the same shares, but the order
differs, so 1.x scenarios with round-robin transitions give different
results.

## Stage completion at insertion

In 1.x, `add_agent` already completed a stage the agent was placed
inside. In 2.0 that happens in the first `iterate()`:

- Right after `add_agent`, `agent.stage` is the stage the agent starts
  inside, not the next one.
- A `hold()` or `close()` set up between adding agents and the first
  `iterate()` now applies to agents placed inside that stage.
- An agent placed inside an exit takes one step and is then removed,
  with a removal record. 1.x removed it without a record, so exit
  counts can be higher than in 1.x (for example 500 instead of 497
  exits in `avm-sharp-corners.py`), and the motion of the other agents
  can differ from that step on.

## Pitfalls

Most of these do not raise an error: the simulation runs and gives
numbers that look plausible.

**Routes and rules**

- Route validation is structural. It checks that an exit or `STAY` can
  be reached along the transitions, not that agents get there.
  `LeastTravelTime` and `LeastTargeted` are deterministic, so in a
  cycle they can send agents round forever. A rule that lists its own
  stage as a candidate warns. An agent stuck in such a loop is neither
  `stranded` nor `arrived`, so check that `agent_count()` reaches zero.
- A weight of 0 is allowed. A route whose only exit has weight 0 is
  valid, and nobody leaves through it.
- `update_weights()` with changed weights starts the round-robin
  sequence afresh. Changing them at every step favours the target with
  the highest weight. Applying unchanged weights keeps the sequence.
- `update_weights()` does not resume stranded agents; they decide again
  when an exit of their rule opens.
- One exit stage made of several areas is one exit: the areas close
  together and share one count in the removal records. Use one exit
  per door when doors must be closed or counted separately.

**Timing**

- `removed_agents()` holds the records of the most recent `iterate()`
  only. Read it after every `iterate()`, or removals are lost.
- An agent's removal record appears one `iterate()` before its `alive`
  turns False. Records of `agent.remove()` appear after the next
  `iterate()`.
- A stage an agent starts inside completes in the first `iterate()`
  (see above), so holds and closures set up before it apply.

**Holds and closures**

- Held agents stop where they first enter the waypoint area; they are
  not spread over it. They can block the entrance, so `held_at()` can
  count fewer agents than are queueing.
- With `close(informed=False)`, agents keep walking to the closed exit
  and are stranded there. `close(informed=True)` reassigns agents at
  once but leaves those already leaving through the exit alone.

**Steering to a point**

- `route_orientation` is (0, 0) at a `steer_to()` point only when the
  agent stands exactly on it; agents oscillate around the point. Use
  `agent.arrived`: the last step passed within 0.05 m of the point.
- When no candidate of `LeastTravelTime` can be reached, it picks the
  first open candidate.

**Geometry and handles**

- The simulation keeps the surface as it was at `Simulation()`.
  Regions added later are rejected by that simulation.
- A connector's edges snap to the corners of the regions it joins;
  its area is the snapped one, not the coordinates passed in.
- Stage names and region names must be unique; a repeated name raises
  `ValueError`.
- A handle to a removed agent raises `RuntimeError` for everything but
  `id` and `alive`. Handles from another simulation are rejected.
  `sim.agents()` is a snapshot.
- `Simulation.close()` closes the trajectory writer and frees the
  simulation; it cannot be used afterwards. In notebooks, call it in
  the last cell that uses the simulation.

## Removed APIs at a glance

- All model builder classes (C++ and Python).
- All `XModelAgentParameters` dataclasses.
- The `ModelType` enum; every model is now constructed directly as a
  configured instance (e.g.
  `jps.SocialForceModel(body_force=..., friction=...)`).
- The queue and waiting set stages: `add_queue_stage`,
  `add_waiting_set_stage`, `NotifiableQueueStage`, `WaitingSetStage` and
  `WaitingSetState`.
- The wrapper modules `jupedsim.location`, `jupedsim.stages`,
  `jupedsim.library` and `jupedsim.agent_view`, and
  `jupedsim.journey.Transition`. Their types and functions are the native
  ones now; import them as `jupedsim.<Name>` (e.g. `jps.get_build_info`,
  `jps.AgentView`).
- `CustomOperationalModel._compute_next_state` and
  `CustomOperationalModel._check_model_constraint`.
- Keeping the {class}`~jupedsim.AgentView` or
  {class}`~jupedsim.AgentStep` passed to a custom-model callback beyond the
  callback: such views expire when the callback returns, and any later
  access raises {class}`~jupedsim.SimulationError`.
