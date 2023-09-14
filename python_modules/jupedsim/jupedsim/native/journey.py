# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Optional

try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps


class Transition:
    def __init__(self, backing) -> None:
        self._obj = backing

    @staticmethod
    def create_fixed_transition(stage_id):
        return Transition(py_jps.Transition.create_fixed_transition(stage_id))

    @staticmethod
    def create_round_robin_transition(stage_weights: list[tuple[int, int]]):
        return Transition(
            py_jps.Transition.create_round_robin_transition(stage_weights)
        )


class JourneyDescription:
    def __init__(self, stage_ids: Optional[list[int]] = None):
        if stage_ids is None:
            self._obj = py_jps.JourneyDescription()
        else:
            self._obj = py_jps.JourneyDescription(stage_ids)

    def add(self, stages: int | list[int]) -> None:
        self._obj.add(stages)

    def set_transition_for_stage(self, stage_id: int, transition: Transition):
        self._obj.set_transition_for_stage(stage_id, transition._obj)
