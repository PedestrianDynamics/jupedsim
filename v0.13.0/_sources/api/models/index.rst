:py:mod:`models`
================

.. py:module:: models


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   models.VelocityModelParameters
   models.GeneralizedCentrifugalForceModelParameters
   models.GeneralizedCentrifugalForceModelAgentParameters
   models.VelocityModelAgentParameters
   models.GeneralizedCentrifugalForceModelState
   models.VelocityModelState




.. py:class:: VelocityModelParameters


   Parameters for Velocity Model

   All attributes are initialized with reasonably good defaults.

   Attributes:
       a_ped (float): TODO
       d_ped (float): TODO
       a_wall (float):
       d_wall (float):

   .. py:attribute:: a_ped
      :type: float
      :value: 8.0

      

   .. py:attribute:: d_ped
      :type: float
      :value: 0.1

      

   .. py:attribute:: a_wall
      :type: float
      :value: 5.0

      

   .. py:attribute:: d_wall
      :type: float
      :value: 0.02

      


.. py:class:: GeneralizedCentrifugalForceModelParameters


   Parameters for Generalized Centrifugal Force Model

   All attributes are initialized with reasonably good defaults.

   Attributes:
       nu_ped (float):
       nu_wall (float):
       dist_eff_ped (float)
       dist_eff_wall (float)
       intp_width_ped (float)
       intp_width_wall (float)
       maxf_ped (float)
       maxf_wall (float)

   .. py:attribute:: nu_ped
      :type: float
      :value: 0.3

      

   .. py:attribute:: nu_wall
      :type: float
      :value: 0.2

      

   .. py:attribute:: dist_eff_ped
      :type: float
      :value: 2

      

   .. py:attribute:: dist_eff_wall
      :type: float
      :value: 2

      

   .. py:attribute:: intp_width_ped
      :type: float
      :value: 0.1

      

   .. py:attribute:: intp_width_wall
      :type: float
      :value: 0.1

      

   .. py:attribute:: maxf_ped
      :type: float
      :value: 3

      

   .. py:attribute:: maxf_wall
      :type: float
      :value: 3

      


.. py:class:: GeneralizedCentrifugalForceModelAgentParameters


   Agent parameters for Generalized Centrifugal Force Model.

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


   .. py:property:: stage_id
      :type: int

      Id of curently followed stage

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: mass
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: tau
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: v0
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: a_v
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: a_min
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: b_min
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: b_max
      :type: float

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


   .. py:property:: stage_id
      :type: int

      Id of curently followed stage

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: time_gap
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: v0
      :type: float

      NOTE: Setting this property has no effect on agents that are already part of the simulation


   .. py:property:: radius
      :type: float

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


   .. py:property:: tau
      :type: float


   .. py:property:: v0
      :type: float


   .. py:property:: a_v
      :type: float


   .. py:property:: a_min
      :type: float


   .. py:property:: b_min
      :type: float


   .. py:property:: b_max
      :type: float



.. py:class:: VelocityModelState(backing)


   .. py:property:: e0
      :type: tuple[float, float]


   .. py:property:: time_gap
      :type: float


   .. py:property:: tau
      :type: float


   .. py:property:: v0
      :type: float


   .. py:property:: radius
      :type: float



