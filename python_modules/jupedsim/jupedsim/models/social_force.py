# SPDX-License-Identifier: LGPL-3.0-or-later
"""Social Force Model.

See the scientific publication for more details about this model:
https://doi.org/10.1038/35035023

The model-level parameters (``body_force`` and ``friction``) are carried by
the model instance, which is passed to the simulation:

.. code:: python

    sim = jupedsim.Simulation(
        model=jupedsim.SocialForceModel(
            body_force=120000, friction=240000
        ),
        geometry=...,
    )
    sim.add_agent(
        journey_id=journey_id,
        stage_id=stage_id,
        state=jupedsim.SocialForceModelState(position=(1.0, 1.0), mass=75.0),
    )

.. warning::

    The model instance is consumed by the ``Simulation`` constructor and must
    not be reused afterwards.

:class:`SocialForceModel` exposes the model-level parameters as keyword-only
constructor arguments with sensible defaults: ``body_force`` (k) and
``friction`` (kappa).

:class:`SocialForceModelState` exposes the complete per-agent state of the
model as keyword-only constructor arguments with sensible defaults:
``position``, ``velocity``, ``mass`` (m), ``desired_speed`` (v0),
``reaction_time`` (tau), ``agent_scale`` (A), ``obstacle_scale`` (A),
``force_distance`` (B) and ``radius`` (r).
"""

import jupedsim.native as py_jps

SocialForceModel = py_jps.SocialForceModel
SocialForceModelState = py_jps.SocialForceModelState

__all__ = ["SocialForceModel", "SocialForceModelState"]
