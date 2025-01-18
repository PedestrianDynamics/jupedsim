=================
Pedestrian Models
=================

JuPedSim allows creating pedestrian simulations with different microscopic
models. Below is a list of all the models that are currently available. Please
refer to the links in the respective section for a detailed discussion of the
respective model.


**************************
Collision Free Speed Model
**************************

The collision-free speed model is a mathematical approach designed for
pedestrian dynamics, emphasizing the prevention of collisions among agents. The
direction in which an agent moves is determined through an isotropic
combination of exponential repulsion from nearby agents. The strength of this
repulsion is influenced by the proximity to others within their surroundings,
treating all directions equally in terms of influence. Agents adjust their
speed according to the nearest neighbor in their headway, allowing them to
navigate through congested areas without overlapping or obstructing each other.
The collision-free speed model takes into account the length of the agent,
which determines the required space for movement, and the maximum achievable
speed of the agent. This simplified and computationally efficient model aims to
mirror real-world pedestrian behaviors while maintaining smooth movement
dynamics.

The collision-free speed model is available in two variants in JuPedSim. Both
variants implement the same algorithm but differ when it comes to defining model
parameters globally vs. per-agent.

In :class:`~jupedsim.models.CollisionFreeSpeedModel` neighbor and geometry
repulsion parameters are global parameters, i.e. all agents use the same values
and the values are constant over the simulation.

In :class:`~jupedsim.models.CollisionFreeSpeedModelV2` neighbor and geometry
repulsion parameters are per-agent parameters that can be set individually via
:class:`~jupedsim.models.CollisionFreeSpeedModelV2AgentParameters` and can be
changed at any time.

A `detailed description
<https://pedestriandynamics.org/models/collision_free_speed_model/>`_ is
available on `PedestrianDynamics`_.

The original publication can be found at https://arxiv.org/abs/1512.05597

**************************
Anticipation Velocity Model
**************************

The anticipation velocity model (AVM) is a mathematical approach for pedestrian
dynamics that prevents collisions through anticipatory behavior. The model divides
anticipation into three components: situation perception, future prediction, and
strategy selection.
Building upon the collision-free speed model (CSM), AVM determines agent movement
through exponential repulsion from nearby agents. Unlike CSM, the influence
direction is orthogonal to the agent's desired direction.
Repulsion strength is affected by agents within the perception field -
specifically, those in the union of two half-planes where the agent moves or
intends to move. Agents adjust their speed based on anticipated distance to the
nearest neighbor in their headway, enabling navigation through congested areas
without overlap.
The model incorporates a reaction time factor to adjust the turning process rate
from the current to the new direction.

**Wall Influence**

Walls are treated as gliding surfaces, meaning that agents adjust their movement
to avoid collisions while maintaining smooth trajectories along wall boundaries.
The influence of walls on an agent's movement is determined based on their distance
to the wall and the direction of their movement.
The following rules govern the behavior:

1. **Critical Wall Distance**:
   - A critical wall distance is defined as the sum of the agent's radius and
     a configurable buffer distance (`wallBufferDistance`).
   - If an agent comes within this critical distance to a wall,
     their direction is adjusted to ensure a minimum distance is maintained.

2. **Influence Start Distance**:
   - An influence start distance is set at twice the critical wall distance to
     allow for smoother adjustments as the agent approaches the wall.

3. **Direction Adjustment**:
   - When an agent is within the critical wall distance and moving toward the wall:
     - The direction is projected parallel to the wall to ensure no penetration occurs.
     - A small outward component is added to maintain the minimum distance from the wall.
   - When an agent is between the critical wall distance and the influence start distance:
     - If moving toward the wall, their direction is adjusted with a smooth transition
       factor to gradually reduce components moving perpendicular to the wall.

4. **Wall Direction Handling**:
   - The agent's movement direction is decomposed into parallel and perpendicular
     components relative to the wall surface.
   - Adjustments are applied only to the perpendicular component, ensuring the parallel
     component remains intact for smooth movement along the wall.

In summary walls do not affect the speed of agents, only their direction.
Agents glide along walls by adjusting their direction while maintaining the
desired movement as much as possible. When necessary, the influence transitions smoothly
as the agent moves closer to or farther from the wall.

The anticipation velocity  model takes into account the length of the agent,
which determines the required space for movement, and the maximum achievable
speed of the agent. This simplified and computationally efficient model aims to
mirror real-world pedestrian behaviors while maintaining smooth movement
dynamics.

The parameters of the anticipation velocity model can be defined per-agent.

In :class:`~jupedsim.models.AnticipationVelocityModel` neighbor and wall
parameters are per-agent parameters that can be set individually via
:class:`~jupedsim.models.AnticipationVelocityModelAgentParameters` and can be
changed at any time.


The original publication can be found at https://doi.org/10.1016/j.trc.2021.103464


***********************************
Generalized Centrifugal Force Model
***********************************

The Generalized Centrifugal Force Model is a force-based model that defines
the movement of pedestrians through the combination of small-range forces. This
model represents the spatial requirement of pedestrians, including their body
asymmetry, in an elliptical shape with two axes dependent on speed. The
semi-axis representing the dynamic space requirement in the direction of motion
increases proportionally as speed increases. Conversely, the semi-axis along
the shoulder direction decreases with higher velocities.

A `detailed description
<https://pedestriandynamics.org/models/generalized_centrifugal_force_model/>`_
is available on `PedestrianDynamics`_.

.. note::
   The implementation does not allow to modify all parameters described.
   Espcially the following parameters are defined constant as:

   * :math:`r'_c = r_c - r_{eps}`
   * :math:`s_0 = \tilde{l} - r_{eps}`
   * :math:`\tilde{l} = 0.5`

The original publication can be found at https://arxiv.org/abs/1008.4297

******************
Social Force Model
******************

The Social Force Model [1] is a force-based model that defines the movement of
pedestrians by the combination of different social forces affecting an
individual. The model defines forces that affect an individual:

* A driving force
* A repulsive force
* An obstacle force

The driving force represents a person’s desire to move in a certain direction,
independent of other people and obstacles. The repulsive force is caused by the
interaction between the individuals and causes them to avoid each other in
order to avoid collisions. The obstacle force acts in a similar way to the
person force to avoid collisions with obstacles in the environment.

A `detailed description
<https://pedestriandynamics.org/models/social_force_model/>`_ is available on
`PedestrianDynamics`_.

.. _PedestrianDynamics: https://PedestrianDynamics.org/
