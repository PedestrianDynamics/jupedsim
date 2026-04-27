# CollisionFreeSpeedModelV3 (Rotational Steering)

## Overview

This document describes `CollisionFreeSpeedModelV3`, a minimal rotational
steering model derived from `CollisionFreeSpeedModelV2`.

The model separates:

- **Direction update** (rotation of a reference desired direction).
- **Speed update** (spacing-based, with explicit buffer semantics).

The design goal is to answer two calibration questions with very few controls:

1. How many neighbors affect me?
2. How strongly do I overtake vs. stick to the goal?

## Direction Update

Let:

- `p` be the current agent position.
- `p_j` be neighbor position.
- `r_j = p_j - p`.
- `e_des` be normalized desired direction.
- `r_wall` be the boundary influence vector (same exponential form as V2).
- `e_ref = normalize(e_des + r_wall)` (fallback to current orientation if zero).

Unlike V2, where boundary and neighbor forces are summed into the final velocity,
V3 uses `r_wall` only to bend the reference direction. Neighbor avoidance then
operates via rotational steering on top of this adjusted reference.

Only forward neighbors contribute:

```math
x_j = e_{\mathrm{ref}} \cdot r_j,\quad x_j > 0
```

For each forward neighbor:

```math
s_j = \mathrm{cross}(e_{\mathrm{ref}}, r_j),\quad y_j = |s_j|
```

```math
w_j = \exp(-x_j / r_x)\,\exp(-y_j / r_y)
```

```math
a_j = -\,w_j\,\frac{s_j}{|s_j| + \varepsilon_s}
```

The model uses only the **most relevant** forward neighbor:

```math
j^* = \arg\max_{x_j>0} w_j
```

```math
\theta = \theta_{\max}\,\tanh(a_{j^*})
```

### Temporal Smoothness (Heading Relaxation)

The target turn angle is not applied instantly. Instead, each agent stores a
heading angle state and relaxes toward the target:

```math
\alpha = \mathrm{clamp}(\Delta t/\tau_\theta,\,0,\,1)
```

```math
\theta_{t+\Delta t} = \theta_t + \alpha(\theta_{\mathrm{target}}-\theta_t)
```

```math
\theta_{\mathrm{target}}=\theta_{\max}\tanh(a_{j^*})
```

```math
e_{\mathrm{move}} = R(\theta_{t+\Delta t})\,e_{\mathrm{ref}}
```

The negative sign in `a_j` means the agent turns **away from occupied side**.
Relaxation damps fast left-right switching, reduces snake-like oscillations,
and keeps direction changes smooth under noisy local neighbor constellations.

## Speed Update

Spacing is computed as a blend of move-direction and goal-direction spacing:

```math
s = (1 - w_b)\,s_{\mathrm{move}} + w_b\,s_{\mathrm{goal}}
```

where `w_b = 0.15` (`SpacingBlendWeight`). Each component is the minimum
spacing over all neighbors along the respective direction.

Neighbor is considered in corridor when:

```math
|\mathrm{left}(e_{\mathrm{move}})\cdot r_j| \le l,\quad l=r_i+r_j
```

Geometric spacing:

```math
s = \|r_j\| - l
```

Speed:

```math
v = \min\left(\max\left(\frac{s - b_f}{T}, v_{\min}\right), v_0\right)
```

So speed is zero at:

```math
s = b_f
```

In the current implementation, `v_min = -0.01 m/s` (hard-coded, deterministic)
to allow tiny backward release in blocked contact situations.

## Parameter Mapping (Old vs Current)

| Tuning question | Parameter(s) | Intuition |
|---|---|---|
| How many neighbors influence me? | `rangeNeighborRepulsion` mapped to anisotropic ranges `r_x`, `r_y` | Forward and lateral reach are different (`r_x > r_y`) |
| How strongly do I deviate/overtake? | `strengthNeighborRepulsion` mapped to `theta_max` | Directly sets max turning angle (with clamp) |
| How strongly walls bend direction? | `strengthGeometryRepulsion`, `rangeGeometryRepulsion` | Affects `e_ref` via wall influence |
| How conservative is following speed? | `timeGap`, `v0`, `agentBuffer` | `agentBuffer` shifts speed-zero point to positive spacing |

## Implemented Internal Mapping

Current constants in code:

| Symbol | Code constant | Value | Unit | Role |
|---|---|---:|---|---|
| `r_x` scaling | `RangeXScale` | `20.0` | `-` | `r_x = 20 * rangeNeighborRepulsion` |
| `r_y` scaling | `RangeYScale` | `8.0` | `-` | `r_y = 8 * rangeNeighborRepulsion` |
| max turn cap | `ThetaMaxUpperBound` | `1.0` | `rad` | hard upper bound on `theta_max` |
| spacing blend | `SpacingBlendWeight` | `0.15` | `-` | weight of goal-direction spacing in blend |
| heading relaxation | `TauTheta` | `0.3` | `s` | relaxation timescale for heading state |
| sign smoothing | `SideEps` | `0.05` | `m` | avoids hard sign flips near centerline |
| min reverse speed | `MinReverseSpeed` | `-0.01` | `m/s` | tiny deterministic backward release |

Derived:

- `theta_max = clamp(strengthNeighborRepulsion, 0, ThetaMaxUpperBound)`.
- `r_x = max(eps, rangeNeighborRepulsion * RangeXScale)`.
- `r_y = max(eps, rangeNeighborRepulsion * RangeYScale)`.

## Practical Interpretation

| If you observe... | Primary knob | Typical direction |
|---|---|---|
| Too much queueing / too little overtaking | `strengthNeighborRepulsion` | increase (small steps) |
| Too much zig-zag / snake behavior | `strengthNeighborRepulsion` | decrease |
| Agents react too late to leaders | `rangeNeighborRepulsion` | increase |
| Agents react to too many lateral neighbors | `rangeNeighborRepulsion` | decrease, or reduce `RangeYScale` in code |
| Early lane-like splitting | `rangeNeighborRepulsion` | often increase with moderate strength |

## Notes

- This variant intentionally avoids blending and case-based deadlock logic.
- Steering uses one dominant forward neighbor, which reduces chain-following artifacts from summing many neighbors.
- The anisotropy comes solely from the parameter ratio `r_x / r_y`, both axes use the same linear-exponential decay.
