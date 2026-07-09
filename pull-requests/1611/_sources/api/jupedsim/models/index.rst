:py:mod:`jupedsim.models`
=========================

.. py:module:: jupedsim.models

.. autoapi-nested-parse::

   Operational models of JuPedSim.

   Stateless built-in models are selected with :class:`ModelType` when creating
   a :class:`~jupedsim.simulation.Simulation`. Models with simulation-global
   state (:class:`~jupedsim.models.anticipation_velocity_model.AnticipationVelocityModel`,
   :class:`~jupedsim.models.warp_driver.WarpDriverModel`) and custom Python
   models (:class:`~jupedsim.models.custom_model.CustomOperationalModel`
   subclasses) are passed as instances instead.



Package Contents
----------------

.. py:data:: ModelType

   Selects one of the stateless built-in operational models.

   Members: ``COLLISION_FREE_SPEED``, ``COLLISION_FREE_SPEED_V2``,
   ``COLLISION_FREE_SPEED_V3``, ``GENERALIZED_CENTRIFUGAL_FORCE``,
   ``SOCIAL_FORCE``.

