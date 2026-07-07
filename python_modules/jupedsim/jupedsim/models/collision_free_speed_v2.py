# SPDX-License-Identifier: LGPL-3.0-or-later
"""Collision Free Speed Model V2.

Variant of the Collision Free Speed Model (https://arxiv.org/abs/1512.05597)
in which all repulsion parameters are per-agent instead of global.

The model instance is passed to the simulation:

.. code:: python

    sim = jupedsim.Simulation(
        model=jupedsim.CollisionFreeSpeedModelV2(),
        geometry=...,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        state=jupedsim.CollisionFreeSpeedModelV2State(
            position=(1.0, 1.0), strength_neighbor_repulsion=9.0
        ),
    )

.. warning::

    The model instance is consumed by the ``Simulation`` constructor and must
    not be reused afterwards.

:class:`CollisionFreeSpeedModelV2State` exposes the complete per-agent state
of the model as keyword-only constructor arguments with sensible defaults:
``position``, ``orientation``, ``strength_neighbor_repulsion``,
``range_neighbor_repulsion``, ``strength_geometry_repulsion``,
``range_geometry_repulsion``, ``time_gap``, ``desired_speed`` and ``radius``.
"""

import jupedsim.native as py_jps

CollisionFreeSpeedModelV2 = py_jps.CollisionFreeSpeedModelV2
CollisionFreeSpeedModelV2State = py_jps.CollisionFreeSpeedModelV2State

__all__ = ["CollisionFreeSpeedModelV2", "CollisionFreeSpeedModelV2State"]
