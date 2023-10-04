:py:mod:`journey`
=================

.. py:module:: journey


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   journey.Transition
   journey.JourneyDescription




.. py:class:: Transition(backing)


   .. py:method:: create_fixed_transition(stage_id)
      :staticmethod:


   .. py:method:: create_round_robin_transition(stage_weights: list[tuple[int, int]])
      :staticmethod:


   .. py:method:: create_least_targeted_transition(stage_ids: list[int])
      :staticmethod:



.. py:class:: JourneyDescription(stage_ids: Optional[list[int]] = None)


   .. py:method:: add(stages: int | list[int]) -> None


   .. py:method:: set_transition_for_stage(stage_id: int, transition: Transition)



