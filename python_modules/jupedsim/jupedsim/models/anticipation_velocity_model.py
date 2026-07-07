# SPDX-License-Identifier: LGPL-3.0-or-later
"""Anticipation Velocity Model (AVM).

The AVM incorporates pedestrian anticipation, divided into three phases:
perception of the current situation, prediction of future situations and
strategy selection leading to action. A general description of the AVM can be
found in the originating publication
https://doi.org/10.1016/j.trc.2021.103464

The model-level parameters (``pushout_strength`` and ``rng_seed``) are carried
by the model instance, which is passed to the simulation:

.. code:: python

    sim = jupedsim.Simulation(
        model=jupedsim.AnticipationVelocityModel(rng_seed=1234),
        geometry=...,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        state=jupedsim.AnticipationVelocityModelState(position=(1.0, 1.0)),
    )

.. warning::

    The model instance is consumed by the ``Simulation`` constructor and must
    not be reused afterwards.

:class:`AnticipationVelocityModel` exposes the model-level parameters as
keyword-only constructor arguments with sensible defaults:
``pushout_strength`` and ``rng_seed``.

:class:`AnticipationVelocityModelState` exposes the complete per-agent state
of the model as keyword-only constructor arguments with sensible defaults:
``position``, ``orientation``, ``strength_neighbor_repulsion``,
``range_neighbor_repulsion``, ``wall_buffer_distance``,
``anticipation_time``, ``reaction_time``, ``velocity``, ``time_gap``,
``desired_speed`` and ``radius``.
"""

import jupedsim.native as py_jps

AnticipationVelocityModel = py_jps.AnticipationVelocityModel
AnticipationVelocityModelState = py_jps.AnticipationVelocityModelState

__all__ = [
    "AnticipationVelocityModel",
    "AnticipationVelocityModelState",
]
