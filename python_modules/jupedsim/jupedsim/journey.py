# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Optional

from jupedsim.native import Transition


class JourneyDescription:
    """Used to describe a journey for construction by the :class:`~jupedsim.simulation.Simulation`.

    A Journey describes the desired stations an agent should take when moving through
    the simulation space. A journey is described by a graph of stages (nodes) and
    transitions (edges). See :class:`~jupedsim.Transition` for an overview of the possible
    transitions.
    """

    def __init__(self, stage_ids: Optional[list[int]] = None) -> None:
        """Create a Journey Description.

        Args:
            stage_ids: list of stages this journey should contain.

        """
        self._transitions = dict()
        if stage_ids:
            for id in stage_ids:
                self._transitions[id] = Transition.create_none_transition()

    def add(self, stages: int | list[int]) -> None:
        """Add additional stage or stages.

        Args:
            stages: A single stage id or a list of stage ids.

        """
        if isinstance(stages, int):
            self._transitions[stages] = Transition.create_none_transition()
        else:
            for id in stages:
                self._transitions[id] = Transition.create_none_transition()

    def set_transition_for_stage(
        self, stage_id: int, transition: Transition
    ) -> None:
        """Set a new transition for the specified stage.

        Any prior set transition for this stage will be removed.

        Args:
            stage_id: id of the stage to set the transition for.
            transition: transition to set

        """
        self._transitions[stage_id] = transition
