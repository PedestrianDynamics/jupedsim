# SPDX-License-Identifier: LGPL-3.0-or-later
"""Collision Free Speed Model.

A general description of the Collision Free Speed Model can be found in the
originating publication https://arxiv.org/abs/1512.05597. A more detailed
description can be found at
https://pedestriandynamics.org/models/collision_free_speed_model/

The model-level repulsion parameters are carried by the model instance, which
is passed to the simulation:

.. code:: python

    sim = jupedsim.Simulation(
        model=jupedsim.CollisionFreeSpeedModel(),
        geometry=...,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        state=jupedsim.CollisionFreeSpeedModelState(
            position=(1.0, 1.0), desired_speed=1.4
        ),
    )

.. warning::

    The model instance is consumed by the ``Simulation`` constructor and must
    not be reused afterwards.

:class:`CollisionFreeSpeedModel` exposes the model-level parameters as
keyword-only constructor arguments with sensible defaults:
``strength_neighbor_repulsion``, ``range_neighbor_repulsion``,
``strength_geometry_repulsion`` and ``range_geometry_repulsion``.

:class:`CollisionFreeSpeedModelState` exposes the complete per-agent state of
the model as keyword-only constructor arguments with sensible defaults:
``position``, ``orientation``, ``time_gap``, ``desired_speed`` and
``radius``.
"""

import jupedsim.native as py_jps

CollisionFreeSpeedModel = py_jps.CollisionFreeSpeedModel
CollisionFreeSpeedModelState = py_jps.CollisionFreeSpeedModelState

__all__ = ["CollisionFreeSpeedModel", "CollisionFreeSpeedModelState"]
