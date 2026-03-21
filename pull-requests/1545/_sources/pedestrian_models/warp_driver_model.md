# WarpDriver Model

Based on: Wolinski, D. (2016). *Crowd Simulation Focusing on Pedestrians' Social Relations*. PhD thesis, Université de Rennes 1, Chapter 4 and Appendix B.

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

$$W = W_{th} \circ W_{tu} \circ W_r \circ W_v \circ W_\text{ref}$$

Forward composition order (B.1–B.15): $W_\text{ref} \to W_v \to W_r \to W_{tu} \to W_{vu} \to W_{th}$.
Gradient inverse is applied in reverse order via Jacobian transpose chain.

| Operator | Forward transform | Inverse gradient | Purpose |
|---|---|---|---|
| $W_\text{ref}$ (B.1) | Rotate + translate from $a$'s frame to $b$'s frame | Rotate gradient back | Reference frame change |
| $W_v$ (B.10) | $x' = x - v_b \cdot t$ | $g_t \mathrel{-}= v_b \cdot g_x$ (B.12) | Account for $b$'s forward motion |
| $W_r$ (B.7) | $(x', y') = (x, y) / \alpha$ where $\alpha = r_a + r_b$ | Identity — no scaling (B.9) | Normalize by Minkowski sum |
| $W_{tu}$ (B.4) | $(x', y') = \beta \cdot (x, y)$ where $\beta = 1/(1 + \lambda \cdot \max(t, 0))$ | $g_{x,y} \mathrel{*}= \beta$; $g_t \mathrel{+}= \gamma_1 g_x + \gamma_2 g_y$ (B.6) | Time uncertainty spreading |
| $W_{vu}$ (B.13) | $(x', y') = (\beta_1 x,\; \beta_2 y)$ where $\beta_1 = 1/(1 + \mu_x \cdot v_b/v_a)$, $\beta_2 = 1 + \mu_y$ | $g_x \mathrel{*}= \beta_1$; $g_y \mathrel{*}= \beta_2$ (B.15) | Anisotropic velocity uncertainty |
| $W_{th}$ (B.1) | $t' = t / T$ | $g_t \mathrel{/}= T$ (B.3) | Time horizon normalization |

#### Probability scaling (B.5, B.14)

The warp operators $W_{tu}$ and $W_{vu}$ change the area of the collision field. To preserve probability mass, the intrinsic field value is scaled by the Jacobian determinants:

$$p_b = I(x', y') \cdot \beta_{tu}^2 \cdot \beta_{1,vu} \cdot \beta_{2,vu}$$

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

Four integrals are computed over the sampled trajectory (Eq. 4.4–4.7 from the thesis):

$$N = \int_0^T p(t) \, dt$$

$$P = \frac{1}{N} \int_0^T p(t)^2 \, dt$$

$$\mathbf{G} = \frac{1}{N} \int_0^T p(t) \, \nabla p(t) \, dt$$

$$\mathbf{S} = \frac{1}{N} \int_0^T p(t) \, \mathbf{r}(t) \, dt$$

The corrected trajectory point is (Eq. 4.8):

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

### Step 5 — Velocity smoothing (implementation addition)

The computed velocity is blended with the agent's previous orientation to damp frame-to-frame oscillations in dense clusters:

$$\mathbf{v}_\text{smooth} = 0.5 \cdot \mathbf{v}_\text{new} + 0.5 \cdot \hat{o} \cdot \|\mathbf{v}_\text{new}\|$$

where $\hat{o}$ is the agent's current orientation. This prevents agents from flipping direction every timestep while still allowing gradual course changes.

### Step 6 — Stuck detection and detour (implementation addition)

Agents can become deadlocked — oscillating in place with high instantaneous speed but zero net progress. To detect this, each agent tracks its **net displacement** from an anchor position:

1. When the agent moves more than 0.3 m from its anchor, the anchor resets to the current position and the stuck timer resets.
2. If the agent stays within 0.3 m of its anchor for 5 seconds, it enters **detour mode**.
3. In detour mode (1 second duration), the agent moves laterally (perpendicular to its desired direction) at $0.5 \times v_0$, with a random choice of left or right side. The detour direction is 80% lateral and 20% forward.
4. If the detour would place the agent outside the walkable area, it tries the opposite side. If both sides fail, the agent creeps slowly toward the goal.
5. After the detour ends, all stuck tracking resets and normal avoidance resumes.

This mechanism reliably breaks deadlocks that velocity smoothing alone cannot resolve, such as two agents blocking each other in a narrow passage near a wall.

## Parameters

### Model-level parameters

Set once when creating the simulation. Shared by all agents.

| Parameter | Symbol | Default | Unit | Description |
|---|---|---|---|---|
| `time_horizon` | $T$ | 2.0 | s | Look-ahead time for collision prediction. Larger values detect collisions earlier but increase computation. |
| `step_size` | $\alpha$ | 0.5 | — | Gradient descent step size. Controls how aggressively agents deviate from their projected trajectory. Larger = stronger avoidance. |
| `sigma` | $\sigma$ | 0.3 | — | Gaussian spread of the intrinsic field. Larger values create smoother, wider collision zones. |
| `time_uncertainty` | $\lambda$ | 0.5 | — | Time uncertainty parameter. Spreads the collision field along the time axis — collisions further in the future are treated as less certain. |
| `velocity_uncertainty_x` | $\mu_x$ | 0.2 | — | Longitudinal velocity uncertainty. Compresses the collision field along the direction of motion based on the speed ratio $v_b / v_a$ (B.13). |
| `velocity_uncertainty_y` | $\mu_y$ | 0.2 | — | Lateral velocity uncertainty. Expands the collision field perpendicular to the direction of motion (B.13). |
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
    velocity_uncertainty_x=0.2,
    velocity_uncertainty_y=0.2,
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

## Known limitations

- **$W_\text{ref}$ uses straight-line frame change only.** The thesis defines graph-based variants (Algorithm 3) that warp space along navigable paths, enabling anticipatory avoidance around corners and bends. The routing infrastructure exists (`RoutingEngine::ComputeAllWaypoints`); integrating it with the warp chain is a future extension.

## References

Wolinski, D. (2016). *Crowd Simulation Focusing on Pedestrians' Social Relations*. PhD thesis, Université de Rennes 1. Chapter 4 (model), Appendix B (warp operator derivations).

Wolinski, D., Lin, M. C., and Pettré, J. (2016). *WarpDriver: Context-Aware Probabilistic Motion Prediction for Crowd Simulation*. IEEE Transactions on Visualization and Computer Graphics, 22(12), 2466–2480.
