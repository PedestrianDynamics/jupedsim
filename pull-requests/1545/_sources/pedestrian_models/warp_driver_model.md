# WarpDriver Model

Based on: Wolinski, Lin, and Pettré, *"WarpDriver: Context-Aware Probabilistic Motion Prediction for Crowd Simulation"*, IEEE TVCG, 2016.

## Overview

The WarpDriver model computes collision-free velocities using **probabilistic collision fields**.
Each agent projects its trajectory forward in time, evaluates the collision probability with every neighbor via a **warped intrinsic field**, and adjusts its velocity through gradient descent on the resulting probability landscape.

The pipeline per agent per timestep:

```
Setup → Perceive → Solve → Update
```

## Algorithm

### Step 1 — Setup: projected trajectory

Each agent projects a straight-line trajectory in its own reference frame:

$$\mathbf{r}(t) = (v_0 \cdot t,\; 0,\; t), \quad t \in [0,\; T]$$

where $v_0$ is the agent's desired speed and $T$ is the time horizon.
This trajectory is sampled at `num_samples` evenly spaced points.

### Step 2 — Perceive: collision probability via warped intrinsic fields

#### Intrinsic Field $I(x, y)$

A 2D scalar field precomputed once at model construction:

$$I(x, y) = (f \ast g)(x, y)$$

where $g$ is the indicator function of the unit disk and $f = \mathcal{N}(0, \sigma^2)$ is a Gaussian kernel.
The result is a smooth bump centered at the origin, normalized so $I(0,0) = 1$.

The field is stored on a 61×61 grid over $[-3, 3]^2$ with spacing $\Delta = 0.1$.
Both $I$ and its gradient $\nabla I = (\partial I/\partial x,\; \partial I/\partial y)$ are precomputed.
Lookups use bilinear interpolation; out-of-bounds queries return zero.

#### Warp operators

To evaluate the collision probability between agent $a$ and neighbor $b$, each sample point on $a$'s trajectory is transformed into $b$'s intrinsic field space through a chain of coordinate warps:

$$W = W_{vu} \circ W_{ts} \circ W_r \circ W_v \circ W_\text{local}$$

| Operator | Transform | Purpose |
|---|---|---|
| $W_\text{local}$ | Rotate + translate from $a$'s frame to $b$'s frame | Reference frame change |
| $W_v$ | $x' = x - v_b \cdot t$ | Account for $b$'s forward motion |
| $W_r$ | $(x', y') = (x, y) / r_b$ | Normalize by combined radii ($r_a + r_b$) |
| $W_{ts}$ | $(x', y') = (x, y) / (1 + \lambda t)$ | Spread field with time uncertainty |
| $W_{vu}$ | $(x', y') = (x, y) / (1 + \mu \|v_b\|)$ | Spread field with velocity uncertainty |

After warping, the normalized time $t' = t / T$ must lie in $[0, 1]$; otherwise the sample is skipped.

#### Collision probability combination

For each sample point, the collision probability from neighbor $b$ is looked up as $p_b = I(x', y')$.
Multiple neighbors are combined with the **probabilistic union** (independence assumption):

$$p_\text{total} = p_\text{total} + p_b - p_\text{total} \cdot p_b$$

The gradient is combined analogously:

$$\nabla p_\text{total} = \nabla p_\text{total} + \nabla p_b - p_\text{total} \cdot \nabla p_b - p_b \cdot \nabla p_\text{total}$$

The gradient $\nabla p_b$ is obtained by transforming $\nabla I$ back to $a$'s frame via the **inverse Jacobian** of the warp chain applied in reverse order.
This expands the 2-component field gradient into a 3-component space-time gradient $(g_x, g_y, g_t)$, because the velocity warp $W_v$ couples spatial and temporal dimensions.

### Step 3 — Solve: gradient descent

Four integrals are computed over the sampled trajectory (Eq. 4–7 from the paper):

$$N = \int_0^T p(t) \, dt$$

$$P = \frac{1}{N} \int_0^T p(t)^2 \, dt$$

$$\mathbf{G} = \frac{1}{N} \int_0^T p(t) \, \nabla p(t) \, dt$$

$$\mathbf{S} = \frac{1}{N} \int_0^T p(t) \, \mathbf{r}(t) \, dt$$

The corrected trajectory point is (Eq. 8):

$$\mathbf{q} = \mathbf{S} - \alpha \cdot P \cdot \mathbf{G}$$

where $\alpha$ is the `step_size` parameter. The new velocity in the agent's local frame is:

$$\mathbf{v}_\text{local} = \left(\frac{q_x}{q_t},\; \frac{q_y}{q_t}\right)$$

This is rotated back to world coordinates and clamped to $[0, v_0]$.

If $N \approx 0$ (no collision risk), the agent follows its desired direction at speed $v_0$.

### Step 4 — Random perturbation (implementation addition)

In perfectly symmetric head-on encounters (agents at the same $y$-coordinate moving towards each other), the intrinsic field gradient $\partial I / \partial y = 0$ by symmetry, producing no lateral avoidance.

To break this symmetry, each trajectory sample receives a small random lateral perturbation:

$$\mathbf{r}_i(t) = (v_0 \cdot t,\; \epsilon_i,\; t)$$

where $\epsilon_i \sim \mathcal{U}(-0.05, 0.05)$ is drawn per sample per timestep. The perturbation is small enough not to affect normal avoidance but sufficient to break exact symmetry.

**Short-range repulsion** (implementation addition, not in the original paper): when two agents are closer than $3 \times (r_a + r_b)$, a repulsive velocity term pushes them apart proportional to the overlap depth. This is necessary because the collision probability field is anticipatory — it steers agents *before* they collide, but cannot guarantee separation when agents are already close (e.g. dense crowds, simultaneous arrivals). Similar pushout mechanisms exist in the CFS and AVM models.

**Boundary steering**: when an agent is closer than $3 \times r$ to a wall segment, a steering term adjusts its velocity away from the wall proportional to the proximity.

### Step 5 — Jam detection (chill mode)

After computing the new speed:

1. If speed $< $ `jam_speed_threshold`: increment `jam_counter`
2. If `jam_counter` $\geq$ `jam_step_count`: enter **chill mode** — skip collision avoidance, creep toward the goal at $0.3 \times v_0$, and decay the counter by half so the agent can eventually resume normal avoidance
3. If speed recovers above the threshold: reset `jam_counter` to 0

This prevents oscillation in dense crowds where agents repeatedly try and fail to avoid each other.

## Parameters

### Model-level parameters

Set once when creating the simulation. Shared by all agents.

| Parameter | Symbol | Default | Unit | Description |
|---|---|---|---|---|
| `time_horizon` | $T$ | 2.0 | s | Look-ahead time for collision prediction. Larger values detect collisions earlier but increase computation. |
| `step_size` | $\alpha$ | 0.5 | — | Gradient descent step size. Controls how aggressively agents deviate from their projected trajectory. Larger = stronger avoidance. |
| `sigma` | $\sigma$ | 0.3 | — | Gaussian spread of the intrinsic field. Larger values create smoother, wider collision zones. |
| `time_uncertainty` | $\lambda$ | 0.5 | — | Time uncertainty parameter. Spreads the collision field along the time axis — collisions further in the future are treated as less certain. |
| `velocity_uncertainty` | $\mu$ | 0.2 | — | Velocity uncertainty parameter. Spreads the collision field based on neighbor speed — faster neighbors have less certain positions. |
| `num_samples` | — | 20 | — | Number of points sampled along the projected trajectory. More samples = better accuracy but higher cost. Cost scales as $O(\text{num\_samples} \times \text{neighbors})$. |
| `jam_speed_threshold` | — | 0.1 | m/s | Speed below which an agent is considered jammed. |
| `jam_step_count` | — | 10 | steps | Consecutive jammed steps before entering chill mode. |
| `rng_seed` | — | 42 | — | Seed for the internal random number generator used for symmetry-breaking perturbations. Fixed for reproducibility. |

### Agent-level parameters

Set per agent. Can be modified at runtime.

| Parameter | Default | Unit | Description |
|---|---|---|---|
| `desired_speed` ($v_0$) | 1.2 | m/s | Free-flow speed the agent tries to maintain. |
| `radius` ($r$) | 0.15 | m | Physical radius of the agent. Used in the Minkowski sum for collision detection ($r_a + r_b$) and in the short-range repulsion. |

### Read-only agent state

| Property | Description |
|---|---|
| `jam_counter` | Current count of consecutive steps the agent has been below `jam_speed_threshold`. Resets to 0 when speed recovers. |

## Python API

```python
import jupedsim as jps

# Model-level parameters (shared by all agents)
model = jps.WarpDriverModel(
    time_horizon=2.0,
    step_size=0.5,
    sigma=0.3,
    time_uncertainty=0.5,
    velocity_uncertainty=0.2,
    num_samples=20,
    jam_speed_threshold=0.1,
    jam_step_count=10,
)

sim = jps.Simulation(model=model, geometry=area, dt=0.01)

# Agent-level parameters
agent_id = sim.add_agent(jps.WarpDriverModelAgentParameters(
    position=(2.0, 2.0),
    orientation=(1.0, 0.0),
    journey_id=journey_id,
    stage_id=stage_id,
    desired_speed=1.2,
    radius=0.15,
))

# Runtime state access
state = sim.agent(agent_id).model
print(state.desired_speed)   # 1.2
print(state.radius)          # 0.15
print(state.jam_counter)     # 0

# Mutable at runtime
state.desired_speed = 0.8
state.radius = 0.2
```

## Computational cost

Per agent per timestep: $O(K \times N)$ where $K$ = `num_samples` and $N$ = number of neighbors within `cut_off_radius` (= $3T$).

Each iteration involves:
- $K \times N$ forward warp compositions (6 operators each)
- $K \times N$ intrinsic field lookups (bilinear interpolation)
- $K \times N$ inverse Jacobian gradient transforms
- 1 integration pass over $K$ samples

Typical bottleneck: the warp composition in the inner loop.

## Reference

Wolinski, D., Lin, M. C., and Pettré, J. (2016). *WarpDriver: Context-Aware Probabilistic Motion Prediction for Crowd Simulation*. IEEE Transactions on Visualization and Computer Graphics, 22(12), 2466–2480.
