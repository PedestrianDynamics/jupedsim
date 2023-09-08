# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
from enum import Enum

try:
    from .. import py_jupedsim as py_jps
except ImportError:
    import py_jupedsim as py_jps


class NotifiableQueueProxy:
    def __init__(self, backing):
        self._obj = backing

    def count_targeting(self) -> int:
        return self._obj.count_targeting()

    def count_enqueued(self) -> int:
        return self._obj.count_enqueued()

    def pop(self, count) -> None:
        return self._obj.pop(count)

    def enqueued(self) -> list[int]:
        return self._obj.enqueued()


class WaitingSetState(Enum):
    ACTIVE = py_jps.WaitingSetState.Active
    INACTIVE = py_jps.WaitingSetState.Inactive


class WaitingSetProxy:
    def __init__(self, backing):
        self._obj = backing

    def count_targeting(self):
        return self._obj.count_targeting()

    def count_waiting(self):
        return self._obj.count_waiting()

    def waiting(self):
        return self._obj.waiting()

    @property
    def state(self):
        return WaitingSetState(self._obj.state)

    @state.setter
    def state(self, new_state: WaitingSetState):
        self._obj.state = new_state.value


class WaypointProxy:
    def __init__(self, backing):
        self._obj = backing

    def count_targeting(self):
        return self._obj.count_targeting()


class ExitProxy:
    def __init__(self, backing):
        self._obj = backing

    def count_targeting(self):
        return self._obj.count_targeting()
