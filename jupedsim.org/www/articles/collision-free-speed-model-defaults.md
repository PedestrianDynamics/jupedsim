---
date: "2026-11-18"
author: "Mohcine Chraibi"
description: "Where the default parameters of the CollisionFreeSpeedModel come from, what they reproduce and where they do not, and what that means for your simulations."
---

<!-- SPDX-License-Identifier: LGPL-3.0-or-later -->

# How the CollisionFreeSpeedModel defaults were calibrated

A default parameter set is what you get when you set nothing. In this article, we
explain where the defaults of the `CollisionFreeSpeedModel` in JuPedSim 2.0 come
from, what they reproduce, and where they do not. The full calibration, with all
intermediate results, is described in a series of three notes on
pedestriandynamics.org, listed at the end of this article.

## The defaults

Until JuPedSim 1.4, the parameters of the collision-free speed model (there
`CollisionFreeSpeedModelV2`) were chosen by hand. In 2.0, they come from a calibration against two open experiments, and
the model has one more parameter, the vision factor described below.

| parameter | until 1.4 | 2.0 |
|---|---|---|
| `radius` [m] | 0.2 | 0.155 |
| `time_gap` [s] | 1.0 | 0.956 |
| `strength_neighbor_repulsion` | 8.0 | 2.83 |
| `range_neighbor_repulsion` [m] | 0.1 | 0.15 |
| `strength_geometry_repulsion` | 5.0 | 5.95 |
| `range_geometry_repulsion` [m] | 0.02 | 0.021 |
| `neighbor_repulsion_anisotropy` | (not available) | 0.5 |
| `desired_speed` [m/s] | 1.2 | 1.2 |

All of them are per-agent parameters of `CollisionFreeSpeedModelState`. Any value
you pass replaces the default for that agent.

## The model

In each time step, agent $i$ first chooses a direction $\mathbf{e}_i$ and then a
speed along it. The direction is the normalised sum of the desired direction
$\mathbf{e}_0$, given by the route, and the repulsion from neighbors and walls:

```{math}
:label: cfsm-direction

\mathbf{e}_i = \frac{\mathbf{u}_i}{\lVert \mathbf{u}_i \rVert}, \qquad
\mathbf{u}_i = \mathbf{e}_0 + \sum_j w_{ij}\, \mathbf{R}_{ij} + \sum_w \mathbf{R}_{iw}.
```

The repulsion of neighbor $j$ and of wall $w$ decays exponentially with the
distance:

```{math}
:label: cfsm-repulsion

\mathbf{R}_{ij} = -k \exp\left(\frac{\ell_{ij} - d_{ij}}{D}\right) \mathbf{e}_{ij}, \qquad
\mathbf{R}_{iw} = k_w \exp\left(\frac{r_i - d_{iw}}{D_w}\right) \mathbf{n}_{iw},
```

where $d_{ij}$ is the distance between the centres of $i$ and $j$,
$\mathbf{e}_{ij}$ the unit vector from $i$ to $j$, $\ell_{ij} = r_i + r_j$ the
sum of the radii, $d_{iw}$ the distance from $i$ to wall $w$, and
$\mathbf{n}_{iw}$ the unit vector from the wall to the agent. Neighbors count
within 3 m if no wall lies between them, walls within 4 m.

The speed is the optimal velocity function of the spacing $s_i$ to the closest
neighbor in front:

```{math}
:label: cfsm-speed

\mathbf{v}_i = V(s_i)\, \mathbf{e}_i, \qquad
V(s) = \min\left(\max\left(\frac{s}{T},\, 0\right),\, v_0\right), \qquad
s_i = \min_{j \in J_i} \left(d_{ij} - \ell_{ij}\right).
```

$J_i$ contains the neighbors in front of $i$ (with
$\mathbf{e}_i \cdot \mathbf{e}_{ij} \geq 0$) whose centre lies at most
$\ell_{ij}$ to the side of the line through $i$ in direction $\mathbf{e}_i$.
Without such a neighbor, the agent walks at $v_0$. Since the speed is never
larger than $s_i / T$, an agent never steps into the neighbor in front.

The symbols map to the per-agent parameters of `CollisionFreeSpeedModelState`
as follows:

| symbol | parameter |
|---|---|
| $r_i$ | `radius` |
| $T$ | `time_gap` |
| $v_0$ | `desired_speed` |
| $k$, $D$ | `strength_neighbor_repulsion`, `range_neighbor_repulsion` |
| $k_w$, $D_w$ | `strength_geometry_repulsion`, `range_geometry_repulsion` |
| $\lambda$ | `neighbor_repulsion_anisotropy` |

Equations {eq}`cfsm-direction` to {eq}`cfsm-speed` are the model of Tordeux et
al. (2016) with per-agent parameters, except for the weight $w_{ij}$, which is
new in JuPedSim 2.0.

## The vision factor

In the published model, every neighbor within range repels the agent, i.e.
$w_{ij} = 1$, whether the neighbor is in front or behind. In front of a narrow
bottleneck, this can block the flow: the crowd behind pushes the agents of the
front row sideways against the wall next to the opening. They turn away from
the exit, find a neighbor or a wall in their path, and stop. Since the speed
depends only on the spacing in the walking direction, nothing restarts them.

The Generalized Centrifugal Force Model (Chraibi et al. 2010) weights the force
of a neighbor by the cosine of its angle to the desired direction and ignores
neighbors behind. The `CollisionFreeSpeedModel` adopts this vision factor with
a weight $\lambda$:

```{math}
:label: cfsm-vision

w_{ij} = 1 - \lambda + \lambda \max\left(0,\, \mathbf{e}_0 \cdot \mathbf{e}_{ij}\right).
```

$\lambda = 0$ is the isotropic model of the paper, and $\lambda = 1$ is the full
vision factor. With the default $\lambda = 0.5$, a neighbor in front counts
fully, and a neighbor beside or behind counts half, see Fig. 1.

```{figure} cfsm_vision_weight.png
:alt: Polar plot of the weight of a neighbor's repulsion against its angle to the desired direction, for lambda 0, 0.5 and 1
:width: 420px

Weight $w_{ij}$ of a neighbor's repulsion, Eq. {eq}`cfsm-vision`, against the
angle $\theta$ between the desired direction and the direction to the neighbor.
```

## How the values were found

We calibrated the model with [Dakota](https://dakota.sandia.gov) against two
experiments from the Pedestrian Dynamics Data Archive (see the references):

- the Hermes bottleneck experiment (Düsseldorf 2009), widths 2.4, 3.6 and 5.0 m,
  about 350 participants per run;
- the CrowdQueue experiment (Wuppertal 2018), a 0.5 m gate in corridors of
  1.2 to 5.6 m, seven runs with normal motivation.

Experiment and simulation are analysed with the same code
([PedPy](https://pedpy.readthedocs.io)). The objective is the sum of the squared
differences of flow, density and speed, each divided by a tolerance of 6 % (flow)
or 10 % (density, speed) of the measured value. A run in which nobody passes for
20 s counts as stalled and adds a penalty. Without this penalty, the optimizer
selected sets that fit well on average but clogged in some runs.

The desired speed was fixed at 1.55 m/s, the free speed measured on the Hermes
participants, so that the other parameters could not absorb a wrong walking
speed. The default stays at 1.2 m/s, the usual value for normal walking.

We compared three values of the vision factor. With $\lambda = 0$, the best set
stalled in 13 of 50 runs. With $\lambda = 1$, the fit at the gate was about
twice as bad. With $\lambda = 0.5$, the selected set stalled in none of 50 runs
and fitted the data slightly better than the best set found earlier for
JuPedSim 1.4, see Fig. 2. Its parameters are the defaults in the table above.

```{figure} cfsm_calibration_results.png
:alt: Four panels with the flow in a narrow bottleneck, the calibration objective of all candidate sets, the flow in the Hermes bottleneck and the relative errors at the CrowdQueue gate

Calibration results. (a) Flow in the bottleneck of Xu et al. (2019), not used in
the calibration, desired speeds as in that paper. (b) Calibration objective of
the candidate sets, each re-evaluated with 5 seeds (50 runs); red is the stall
penalty, the default set is "C, anisotropy 0.5". (c) Hermes flow; open markers
are widths not used in the calibration. (d) Relative error of flow, density and
speed at the seven CrowdQueue runs.
```

## What the defaults reproduce

The following results come from data that were not used in the calibration.

**Narrow bottleneck.** Fig. 3 shows the bottleneck of Xu et al. (2019), 1.0 to
2.5 m wide, with all agents at the default desired speed of 1.2 m/s. With the
former defaults, the flow is 39 to 56 % below the experiment, and in 2 of 10
runs at 1.0 m nobody passes for 60 s. With the 2.0 defaults, no run stalls, and
the flow is 4 to 26 % below the experiment.

```{figure} cfsm_defaults_comparison.png
:alt: Flow through a bottleneck of 1.0 to 2.5 m and snapshots of one run at 1.0 m, for the former and the 2.0 defaults

Former and 2.0 defaults in the bottleneck of Xu et al. (2019), all agents at
1.2 m/s. (a) Flow of the runs that emptied the room, 10 seeds per width. (b)
Agents in the room at w = 1.0 m for one seed. Right: snapshots of the same two
runs.
```

**Wide bottleneck.** At the Hermes widths 3.0 and 4.4 m, the flow is about 20 %
too low.

**Both changes are needed.** Fig. 4 separates the two changes at w = 1.0 m. The
vision factor alone does not change the former defaults. The calibrated
parameters alone, with $\lambda = 0$, clog after about 34 s: a dense arch closes
over the entrance. Only the combination empties the room. The calibrated
parameters make the crowd denser and faster, and the vision factor keeps such a
crowd from blocking the opening. Fig. 4 shows one seed. However, it agrees with
the stalls of the five-seed tests above.

```{figure} cfsm_defaults_variants.png
:alt: Agents in the room over time for four parameter variants, and snapshots of the gap region at 40 s

The two changes separately, w = 1.0 m, one seed, all agents at 1.2 m/s. Top:
agents in the room over time. Bottom: the region around the gap at 40 s,
coloured by speed, with velocity arrows.
```

## What this means for your simulations

- **The defaults are a compromise.** No parameter set we found matches both
  experiments within the tolerances. The defaults do not clog in any of our
  tests, but they underestimate the flow through bottlenecks by about 5 to
  30 %, and in front of the 0.5 m gate the speed is 27 to 53 % too high in six
  of seven runs.
- **Calibrate for your question.** If your results depend on the flow through
  a door or on the density in front of it, compare with data from a similar
  geometry and calibrate the parameters for it. A calibration holds for the
  regime it was fitted to; part 2 of the series shows how far that regime
  reaches.
- **Keep the vision factor when you change the repulsion.** In our tests, the
  calibrated parameters with $\lambda = 0$ clogged in narrow bottlenecks. If
  you need the isotropic model of the paper, use
  `CollisionFreeSpeedModelTordeux2016`, which is kept unchanged.
- **Use a small time step.** The calibration used $\Delta t = 0.01$ s. With
  0.05 s, a single agent in a 0.6 m wide curved corridor walked at two thirds of
  its desired speed, since the walls turned its direction back and forth.
- **The desired speed is yours to set.** The default of 1.2 m/s is a common
  value for normal walking. Set it per agent if you know the population.

## Further reading

The calibration is described step by step in three notes on
pedestriandynamics.org:

1. [Part 1](https://pedestriandynamics.org/notes/dakota-calibration/): calibrating
   the model with Dakota on the Hermes bottleneck.
2. [Part 2](https://pedestriandynamics.org/notes/dakota-validation/): where the
   calibrated parameters stop working.
3. [Part 3](https://pedestriandynamics.org/notes/dakota-defaults/): the
   recalibration for JuPedSim 2.0 that produced these defaults.

The scripts, Dakota input files and results are in the
[jupedsim-dakota-calibration](https://github.com/PedestrianDynamics/jupedsim-dakota-calibration/tree/1d7fc4e/joint2)
repository.

## References

- Chraibi, M., Seyfried, A., Schadschneider, A. (2010). Generalized
  centrifugal-force model for pedestrian dynamics. Physical Review E 82, 046111.
  [doi:10.1103/PhysRevE.82.046111](https://doi.org/10.1103/PhysRevE.82.046111).
- Tordeux, A., Chraibi, M., Seyfried, A. (2016). Collision-free speed model for
  pedestrian dynamics. In: Traffic and Granular Flow '15, Springer, 225–232.
  [doi:10.1007/978-3-319-33482-0_29](https://doi.org/10.1007/978-3-319-33482-0_29).
- Xu, Q., Chraibi, M., Tordeux, A., Zhang, J. (2019). Generalized collision-free
  velocity model for pedestrian dynamics. Physica A 535, 122521.
  [doi:10.1016/j.physa.2019.122521](https://doi.org/10.1016/j.physa.2019.122521).
- Experiment data: Hermes bottleneck experiment, Düsseldorf 2009,
  [doi:10.34735/ped.2009.6](https://doi.org/10.34735/ped.2009.6); CrowdQueue
  experiment, Wuppertal 2018,
  [doi:10.34735/ped.2018.1](https://doi.org/10.34735/ped.2018.1).
