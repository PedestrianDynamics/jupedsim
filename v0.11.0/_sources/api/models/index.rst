:py:mod:`models`
================

.. py:module:: models


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   models.VelocityModelBuilder
   models.GCFMModelBuilder
   models.GCFMModelAgentParameters
   models.VelocityModelAgentParameters
   models.GeneralizedCentrifugalForceModelState
   models.VelocityModelState




.. py:class:: VelocityModelBuilder(a_ped: float, d_ped: float, a_wall: float, d_wall: float)


   .. py:method:: add_parameter_profile(id: int, time_gap: float, tau: float, v0: float, radius: float) -> None


   .. py:method:: build()



.. py:class:: GCFMModelBuilder(nu_ped: float, nu_wall: float, dist_eff_ped: float, dist_eff_wall: float, intp_width_ped: float, intp_width_wall: float, maxf_ped: float, maxf_wall: float)


   .. py:method:: add_parameter_profile(profile_id: int, mass: float, tau: float, v0: float, a_v: float, a_min: float, b_min: float, b_max: float)


   .. py:method:: build()



.. py:class:: GCFMModelAgentParameters


   Agent parameters for Generalized Centrifugal Model.

   See the scientifc publication for more details about this model
   https://arxiv.org/abs/1008.4297

   Objects of this type can be used to add new agents to the simulation and are
   returned by the simulation when inspecting agent state. Setting properties on
   objects returned by the simulation has no effect on the agents as this object
   is a copy of internal state.

   Setting properties on this object is only useful when adding multiple agents
   and they share many properties without reprating them on each 'add_agent'
   call

   .. py:property:: speed
      :type: float

      Current speed

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: e0
      :type: tuple[float, float]

      e0 (Currently desired orientation)

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: position
      :type: tuple[float, float]

      Current position

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: orientation
      :type: tuple[float, float]

      Current orientation

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: journey_id
      :type: int

      Id of curently followed journey

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: profile_id
      :type: int

      Id of curently used profile

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: id
      :type: int

      Id of this Agent

      NOTE: Setting this property has no effect on agents that are already part of the simulation



.. py:class:: VelocityModelAgentParameters


   Agent parameters for Velocity Model.

   See the scientifc publication for more details about this model
   https://arxiv.org/abs/1512.05597

   Objects of this type can be used to add new agents to the simulation and are
   returned by the simulation when inspecting agent state. Setting properties on
   objects returned by the simulation has no effect on the agents as this object
   is a copy of internal state.

   Setting properties on this object is only useful when adding multiple agents
   and they share many properties without reprating them on each 'add_agent'
   call

   .. py:property:: e0
      :type: tuple[float, float]

      e0 (Currently desired direction)

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: position
      :type: tuple[float, float]

      Current position

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: orientation
      :type: tuple[float, float]

      Current orientation

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: journey_id
      :type: int

      Id of curently followed journey

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: profile_id
      :type: int

      Id of curently used profile

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: id
      :type: int

      Id of this Agent

      NOTE: Setting this property has no effect on agents that are already part of the simulation



.. py:class:: GeneralizedCentrifugalForceModelState(backing)


   .. py:property:: speed
      :type: float


   .. py:property:: e0
      :type: tuple[float, float]



.. py:class:: VelocityModelState(backing)


   .. py:property:: e0
      :type: tuple[float, float]



