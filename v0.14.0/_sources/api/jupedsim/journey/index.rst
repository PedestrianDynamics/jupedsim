:py:mod:`jupedsim.journey`
==========================

.. py:module:: jupedsim.journey


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.journey.Transition
   jupedsim.journey.JourneyDescription




.. py:class:: Transition(backing)


   Describes the Transition at a stage.

   This type describes how a agent will proceed after completing its stage.
   This effectively describes the set of outbound edges for a stage.

   There are 3 types of transitions currently available:

   * **Fixed transitions:** On completion of this transitions stage all agents
     will proceed to the specified next stage.

   * **Round robin transitions:** On completion of this transitions stage agents
     will proceed in a weighted round-robin manner. A round-robin transitions
     with 3 outgoing stages and the weights 5, 7, 11 the first 5 agents to make
     a choice will take the first stage, the next 7 the second stage and the
     next 11 the third stage. Next 5 will take the first stage, and so on...

   * **Least targeted transition:** On completion of this stage agents will
     proceed towards the currently least targeted amongst the specified choices.
     The number of "targeting" agents is the amount of agents currently moving
     towards this stage. This includes agents from different journeys.

   .. py:method:: create_fixed_transition(stage_id)
      :staticmethod:

      Create a fixed transition.

      On completion of this transitions stage all agents will proceed to the
      specified next stage.

      :param stage_id: id of the stage to move to next.
      :type stage_id: int


   .. py:method:: create_round_robin_transition(stage_weights)
      :staticmethod:

      Create a round-robin transition.

      Round-robin transitions: On completion of this transitions stage agents
      will proceed in a weighted round-robin manner. A round-robin
      transitions with 3 outgoing stages and the weights 5, 7, 11 the first 5
      agents to make a choice will take the first stage, the next 7 the
      second stage and the next 11 the third stage. Next 5 will take the
      first stage, and so on...

      :param stage_weights: list of id/weight tuples.
      :type stage_weights: list[tuple[int, int]]


   .. py:method:: create_least_targeted_transition(stage_ids)
      :staticmethod:

      Create a least targeted transition.

      On completion of this stage agents will proceed towards the currently
      least targeted amongst the specified choices. The number of "targeting"
      agents is the amount of agents currently moving towards this stage.
      This includes agents from different journeys.

      :param stage_ids: list of stage ids to choose the next target
                        from.
      :type stage_ids: list[int]



.. py:class:: JourneyDescription(stage_ids = None)


   Used to describe a journey for construction by the :class:`~jupedsim.simulation.Simulation`.

   A Journey describes the desired stations an agent should take when moving through
   the simulation space. A journey is described by a graph of stages (nodes) and
   transitions (edges). See :class:`~jupedsim.journey.Transition` for an overview of the possible
   transitions.

   Create a Journey Description.

   :param stage_ids: list of stages this journey should contain.
   :type stage_ids: Optional[list[int]]

   .. py:method:: add(stages)

      Add additional stage or stages.

      :param stages: A single stage id or a list of stage ids.
      :type stages: int | list[int]


   .. py:method:: set_transition_for_stage(stage_id, transition)

      Set a new transition for the specified stage.

      Any prior set transition for this stage will be removed.

      :param stage_id: id of the stage to set the transition for.
      :type stage_id: int
      :param transition: transition to set
      :type transition: Transition



