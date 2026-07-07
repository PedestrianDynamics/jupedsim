# SPDX-License-Identifier: LGPL-3.0-or-later
"""Generalized Centrifugal Force Model.

See the scientific publication for more details about this model:
https://arxiv.org/abs/1008.4297

The model-level repulsion parameters are carried by the model instance, which
is passed to the simulation:

.. code:: python

    sim = jupedsim.Simulation(
        model=jupedsim.GeneralizedCentrifugalForceModel(),
        geometry=...,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        state=jupedsim.GeneralizedCentrifugalForceModelState(
            position=(1.0, 1.0), desired_speed=1.0
        ),
    )

.. warning::

    The model instance is consumed by the ``Simulation`` constructor and must
    not be reused afterwards.

:class:`GeneralizedCentrifugalForceModel` exposes the model-level parameters
as keyword-only constructor arguments with sensible defaults:
``strength_neighbor_repulsion``, ``strength_wall_repulsion``,
``max_neighbor_interaction_distance``, ``max_geometry_interaction_distance``,
``max_neighbor_interpolation_distance``,
``max_geometry_interpolation_distance``, ``max_neighbor_repulsion_force`` and
``max_geometry_repulsion_force``.

:class:`GeneralizedCentrifugalForceModelState` exposes the complete per-agent
state of the model as keyword-only constructor arguments with sensible
defaults: ``position``, ``orientation``, ``speed``, ``desired_direction``
(e0), ``mass``, ``tau``, ``desired_speed`` (v0), ``a_v``, ``a_min``,
``b_min`` and ``b_max``.
"""

import jupedsim.native as py_jps

GeneralizedCentrifugalForceModel = py_jps.GeneralizedCentrifugalForceModel
GeneralizedCentrifugalForceModelState = (
    py_jps.GeneralizedCentrifugalForceModelState
)

__all__ = [
    "GeneralizedCentrifugalForceModel",
    "GeneralizedCentrifugalForceModelState",
]
