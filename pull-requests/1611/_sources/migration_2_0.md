# Migrating to JuPedSim 2.0

JuPedSim 2.0 is a clean break: the model builder classes and the
`XModelAgentParameters` dataclasses have been **removed without deprecation
shims**. This guide maps every 1.x construct to its 2.0 replacement.

The three changes you will encounter in every script:

1. **Simulation construction** — models are selected with a
   {class}`~jupedsim.ModelType` enum member (stateless models) or a model
   instance (models with simulation-global state, custom Python models).
2. **Adding agents** — `add_agent` now takes
   `(journey_id, stage_id, state)`, where `state` is a directly
   instantiated per-model state object such as
   {class}`~jupedsim.CollisionFreeSpeedModelState`.
3. **Agent access** — {class}`~jupedsim.Agent` objects are lightweight
   *handles by id* that resolve the agent freshly on every attribute access.

## Constructing a Simulation

Stateless built-in models are selected with an enum member instead of a
parameter-holder instance:

```python
import jupedsim as jps

# 1.x
sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=area)

# 2.0
sim = jps.Simulation(model=jps.ModelType.COLLISION_FREE_SPEED, geometry=area)
```

Available enum members: `COLLISION_FREE_SPEED`, `COLLISION_FREE_SPEED_V2`,
`COLLISION_FREE_SPEED_V3`, `GENERALIZED_CENTRIFUGAL_FORCE`, `SOCIAL_FORCE`.
Enum-constructed models reproduce exactly the global configuration that the
1.x defaults produced, so existing simulations remain byte-identical.

Models that carry simulation-global state are passed as an instance:

```python
# AnticipationVelocityModel: the random number generator is global state.
# rng_seed is keyword-only and required.
sim = jps.Simulation(
    model=jps.AnticipationVelocityModel(rng_seed=1234), geometry=area
)

# WarpDriverModel: the precomputed intrinsic field (sigma) and the RNG are
# global state. All arguments are keyword-only; defaults shown.
sim = jps.Simulation(
    model=jps.WarpDriverModel(sigma=0.3, rng_seed=42), geometry=area
)
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

`add_agent` takes the journey id, the stage id and a per-model state object.
The agent spawns at `state.position`. All state constructors are
keyword-only and expose *every* per-agent field — including the parameters
that were global in 1.x — with defaults taken from the C++ implementation.

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
    journey_id,
    stage_id,
    jps.CollisionFreeSpeedModelState(position=(1.0, 1.0), desired_speed=1.4),
)
```

Passing a state class that does not match the simulation's model raises
{class}`~jupedsim.SimulationError`; passing a non-state object raises
`TypeError`.

For custom models the state is your own object satisfying the
{class}`~jupedsim.CustomModelAgentState` protocol (an object with a
`position` attribute; a frozen dataclass is recommended):

```python
sim.add_agent(journey_id, stage_id, MyState(position=(1.0, 1.0)))
```

## Old → new mapping per model

| 1.x (deleted) | 2.0 `Simulation(model=...)` | 2.0 `add_agent` state |
|---|---|---|
| `CollisionFreeSpeedModel` + `CollisionFreeSpeedModelAgentParameters` | `ModelType.COLLISION_FREE_SPEED` | {class}`~jupedsim.CollisionFreeSpeedModelState` |
| `CollisionFreeSpeedModelV2` + `CollisionFreeSpeedModelV2AgentParameters` | `ModelType.COLLISION_FREE_SPEED_V2` | {class}`~jupedsim.CollisionFreeSpeedModelV2State` |
| `CollisionFreeSpeedModelV3` + `CollisionFreeSpeedModelV3AgentParameters` | `ModelType.COLLISION_FREE_SPEED_V3` | {class}`~jupedsim.CollisionFreeSpeedModelV3State` |
| `GeneralizedCentrifugalForceModel` + `GeneralizedCentrifugalForceModelAgentParameters` | `ModelType.GENERALIZED_CENTRIFUGAL_FORCE` | {class}`~jupedsim.GeneralizedCentrifugalForceModelState` |
| `SocialForceModel` + `SocialForceModelAgentParameters` | `ModelType.SOCIAL_FORCE` | {class}`~jupedsim.SocialForceModelState` |
| `AnticipationVelocityModel` (dataclass) + `AnticipationVelocityModelAgentParameters` | `jps.AnticipationVelocityModel(rng_seed=...)` instance | {class}`~jupedsim.AnticipationVelocityModelState` |
| `WarpDriverModel` (parameter holder) + `WarpDriverModelAgentParameters` | `jps.WarpDriverModel(sigma=..., rng_seed=...)` instance | {class}`~jupedsim.WarpDriverModelState` |

Parameters that were *global* in 1.x (e.g. the repulsion strengths of the
Collision Free Speed Model, `body_force`/`friction` of the Social Force
Model, or `time_horizon`/`step_size` of the WarpDriver model) are now
*per-agent* fields of the corresponding state class and can differ between
agents and change over time.

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
- The stateless model parameter-holder dataclasses
  (`CollisionFreeSpeedModel`, `SocialForceModel`, ... as constructor
  arguments); use {class}`~jupedsim.ModelType` instead.
