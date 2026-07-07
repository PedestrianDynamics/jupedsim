# SPDX-License-Identifier: LGPL-3.0-or-later
"""Collision Free Speed Model V3.

Variant of the Collision Free Speed Model with anisotropic neighbor
interaction and relaxed heading dynamics.

The model instance is passed to the simulation:

.. code:: python

    sim = jupedsim.Simulation(
        model=jupedsim.CollisionFreeSpeedModelV3(),
        geometry=...,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        state=jupedsim.CollisionFreeSpeedModelV3State(
            position=(1.0, 1.0), desired_speed=1.4
        ),
    )

.. warning::

    The model instance is consumed by the ``Simulation`` constructor and must
    not be reused afterwards.

:class:`CollisionFreeSpeedModelV3State` exposes the complete per-agent state
of the model as keyword-only constructor arguments with sensible defaults:
``position``, ``orientation``, ``strength_neighbor_repulsion``,
``range_neighbor_repulsion``, ``strength_geometry_repulsion``,
``range_geometry_repulsion``, ``range_x_scale``, ``range_y_scale``,
``theta_max_upper_bound``, ``agent_buffer``, ``time_gap``, ``desired_speed``,
``radius`` and ``heading_angle``.
"""

import jupedsim.native as py_jps

CollisionFreeSpeedModelV3 = py_jps.CollisionFreeSpeedModelV3
CollisionFreeSpeedModelV3State = py_jps.CollisionFreeSpeedModelV3State

__all__ = ["CollisionFreeSpeedModelV3", "CollisionFreeSpeedModelV3State"]
