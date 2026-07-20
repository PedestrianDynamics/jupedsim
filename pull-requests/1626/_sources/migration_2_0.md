# Migrating to JuPedSim 2.0

JuPedSim 2.0 is a clean break: the model builder classes and the
`XModelAgentParameters` dataclasses have been **removed without deprecation
shims**. This guide maps every 1.x construct to its 2.0 replacement.

The three changes you will encounter in every script:

1. **Simulation construction** — every model is passed as a *configured
   instance* carrying its model-level parameters (e.g.
   `jps.SocialForceModel(body_force=..., friction=...)`). The
   `ModelType` enum has been removed.
2. **Adding agents** — `add_agent` now takes the keyword-only arguments
   `journey_id`, `stage_id` and `state`, where `state` is a directly
   instantiated per-model state object such as
   {class}`~jupedsim.CollisionFreeSpeedModelState`.
3. **Agent access** — {class}`~jupedsim.Agent` objects are lightweight
   *handles by id* that resolve the agent freshly on every attribute access.

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

`add_agent` takes the journey id, the stage id and a per-model state object —
**all three as keyword-only arguments**; positional calls raise `TypeError`.
The agent spawns at `state.position`. All state constructors are
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
agent_id = sim.add_agent(
    journey_id=journey_id,
    stage_id=stage_id,
    state=jps.CollisionFreeSpeedModelState(position=(1.0, 1.0), desired_speed=1.4),
)
```

Passing a state class that does not match the simulation's model raises
{class}`~jupedsim.SimulationError`; passing a non-state object raises
`TypeError`.

For custom models the state is your own object satisfying the
{class}`~jupedsim.CustomModelAgentState` protocol (an object with a
`position` attribute; a frozen dataclass is recommended):

```python
sim.add_agent(journey_id=journey_id, stage_id=stage_id, state=MyState(position=(1.0, 1.0)))
```

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
  has left the simulation raises {class}`~jupedsim.SimulationError`.
- **Property syntax keeps working, including mutation:**

```python
agent = sim.agent(agent_id)
agent.target = (10.0, 5.0)
agent.model.desired_speed = 0.8   # takes effect in the next iterate()

for _ in range(1000):
    sim.iterate()

print(agent.position)             # still valid, freshly resolved
```

`agent.model` returns a per-model state handle with the same semantics: it
resolves per access and raises once the agent is gone.

## Mutation during iterate() is now an error

Calling mutating simulation methods — `add_agent`,
`mark_agent_for_removal`, journey or stage mutation — while `iterate()` is
running (e.g. from a custom-model callback or a trajectory writer invoked
during iteration) now raises {class}`~jupedsim.SimulationError` instead of
causing undefined behavior. Perform such mutations between `iterate()`
calls.

## Removed APIs at a glance

- All model builder classes (C++ and Python).
- All `XModelAgentParameters` dataclasses.
- The `ModelType` enum; every model is now constructed directly as a
  configured instance (e.g.
  `jps.SocialForceModel(body_force=..., friction=...)`).
