# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Optional

import jupedsim.native as py_jps


class Transition:
    """Describes the Transition at a stage.

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
    """

    def __init__(self, backing) -> None:
        self._obj = backing

    @staticmethod
    def create_fixed_transition(stage_id: int) -> "Transition":
        """Create a fixed transition.

        On completion of this transitions stage all agents will proceed to the
        specified next stage.

        Arguments:
            stage_id: id of the stage to move to next.

        """
        return Transition(py_jps.Transition.create_fixed_transition(stage_id))

    @staticmethod
    def create_round_robin_transition(
        stage_weights: list[tuple[int, int]]
    ) -> "Transition":
        """Create a round-robin transition.

        Round-robin transitions: On completion of this transitions stage agents
        will proceed in a weighted round-robin manner. A round-robin
        transitions with 3 outgoing stages and the weights 5, 7, 11 the first 5
        agents to make a choice will take the first stage, the next 7 the
        second stage and the next 11 the third stage. Next 5 will take the
        first stage, and so on...

        Arguments:
            stage_weights: list of id/weight tuples.

        """
        return Transition(
            py_jps.Transition.create_round_robin_transition(stage_weights)
        )

    @staticmethod
    def create_least_targeted_transition(stage_ids: list[int]) -> "Transition":
        """Create a least targeted transition.

        On completion of this stage agents will proceed towards the currently
        least targeted amongst the specified choices. The number of "targeting"
        agents is the amount of agents currently moving towards this stage.
        This includes agents from different journeys.

        Arguments:
            stage_ids: list of stage ids to choose the next target from.

        """
        return Transition(
            py_jps.Transition.create_least_targeted_transition(stage_ids)
        )


class JourneyDescription:
    """Used to describe a journey for construction by the :class:`~jupedsim.simulation.Simulation`.

    A Journey describes the desired stations an agent should take when moving through
    the simulation space. A journey is described by a graph of stages (nodes) and
    transitions (edges). See :class:`~jupedsim.journey.Transition` for an overview of the possible
    transitions.
    """

    def __init__(self, stage_ids: Optional[list[int]] = None) -> None:
        """Create a Journey Description.

        Arguments:
            stage_ids: list of stages this journey should contain.

        """
        if stage_ids is None:
            self._obj = py_jps.JourneyDescription()
        else:
            self._obj = py_jps.JourneyDescription(stage_ids)

    def add(self, stages: int | list[int]) -> None:
        """Add additional stage or stages.

        Arguments:
            stages: A single stage id or a list of stage ids.

        """
        self._obj.add(stages)

    def set_transition_for_stage(
        self, stage_id: int, transition: Transition
    ) -> None:
        """Set a new transition for the specified stage.

        Any prior set transition for this stage will be removed.

        Arguments:
            stage_id: id of the stage to set the transition for.
            transition: transition to set

        """
        self._obj.set_transition_for_stage(stage_id, transition._obj)
