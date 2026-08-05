# SPDX-License-Identifier: LGPL-3.0-or-later
"""WarpDriver Model.

Based on Wolinski, Lin, and Pettré (2016) -- probabilistic collision
avoidance using warped intrinsic fields.

The model-level parameters (the precomputed intrinsic collision-probability
field controlled by ``sigma``, the sampling and look-ahead parameters and a
random number generator seeded by ``rng_seed``) are carried by the model
instance, which is passed to the simulation:

.. code:: python

    sim = jupedsim.Simulation(
        model=jupedsim.WarpDriverModel(sigma=0.3, rng_seed=42),
        geometry=...,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        position=(1.0, 1.0),
        state=jupedsim.WarpDriverModelState(),
    )

.. warning::

    The model instance is consumed by the ``Simulation`` constructor and must
    not be reused afterwards.

:class:`WarpDriverModel` exposes the model-level parameters as keyword-only
constructor arguments with sensible defaults: ``sigma``, ``time_horizon``,
``step_size``, ``time_uncertainty``, ``velocity_uncertainty_x``,
``velocity_uncertainty_y``, ``num_samples`` and ``rng_seed``.

:class:`WarpDriverModelState` exposes the complete per-agent state of the
model as keyword-only constructor arguments with sensible defaults:
``orientation``, ``radius``, ``desired_speed``, ``stuck_time``,
``displacement_x``, ``displacement_y``, ``detour_time`` and ``detour_side``.
"""

import jupedsim.native as py_jps

WarpDriverModel = py_jps.WarpDriverModel
WarpDriverModelState = py_jps.WarpDriverModelState

__all__ = [
    "WarpDriverModel",
    "WarpDriverModelState",
]
