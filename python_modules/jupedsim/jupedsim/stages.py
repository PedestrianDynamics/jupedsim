# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
from enum import Enum

import jupedsim.native as py_jps


class NotifiableQueueStage:
    """Models a queue where agents can wait until notified.

    The queues waiting positions are predefined and agents will wait on the
    first empty position. When agents leave the queue the remaining waiting
    agents move up. If there are more agents trying to enqueue than there are
    waiting positions defined the overflow agents will wait at the last waiting
    position in the queue.

    .. note::
        This type is used to interact with an already created stage. To create
        a stage of this type see :class:`~jupedsim.simulation.Simulation`

    """

    def __init__(self, backing):
        self._obj = backing

    def count_targeting(self) -> int:
        """
        Returns:
            Number of agents currently targeting this stage.

        """
        return self._obj.count_targeting()

    def count_enqueued(self) -> int:
        """
        Returns:
            Number of agents currently enqueued at this stage.

        """
        return self._obj.count_enqueued()

    def pop(self, count) -> None:
        """Pop `count` number of agents from the front of the queue.

        Arguments:
            count: Number of agents to be popped from the front of the
                queue
        """
        return self._obj.pop(count)

    def enqueued(self) -> list[int]:
        """Access the ids of all enqueued agents in order they are waiting at
        the queue.

        Returns:
            list of enqueued agents ordered by their position in the queue.
        """
        return self._obj.enqueued()


class WaitingSetState(Enum):
    ACTIVE = py_jps.WaitingSetState.Active
    INACTIVE = py_jps.WaitingSetState.Inactive


class WaitingSetStage:
    """Models a set of waiting positions that can be activated or deactivated.

    Similar as with a :class:`NotifiableQueueStage` there needs to be a set of
    waiting positions defined which will be filled in order of definition. The
    :class:`WaitingSetStage` now can be active or inactive. If active agents will fill
    waiting positions until all are occupied. Additional agents will all try to
    wait at the last defined waiting position. In inactive state the
    :class:`WaitingSetStage` acts as a simple waypoint at the position of the first
    defined waiting position.
    """

    def __init__(self, backing) -> None:
        self._obj = backing

    def count_targeting(self) -> int:
        """
        Returns:
            Number of agents currently targeting this stage.
        """
        return self._obj.count_targeting()

    def count_waiting(self) -> int:
        """
        Returns:
            Number of agents currently waiting at this stage.
        """
        return self._obj.count_waiting()

    def waiting(self) -> list[int]:
        """Access the ids of all waiting agents in order they are waiting.

        Returns:
            list of waiting agents ordered by their position.
        """
        return self._obj.waiting()

    @property
    def state(self) -> WaitingSetState:
        """State of the set.

        Can be active or inactive, see :class:`WaitingSetState`
        """
        return WaitingSetState(self._obj.state)

    @state.setter
    def state(self, new_state: WaitingSetState):
        self._obj.state = new_state.value


class WaypointStage:
    """Models a waypoint.

    A waypoint is considered to be reached if an agent is within the specified
    distance to the waypoint.
    """

    def __init__(self, backing) -> None:
        self._obj = backing

    def count_targeting(self) -> int:
        """Returns:
        Number of agents currently targeting this stage.
        """
        return self._obj.count_targeting()


class ExitStage:
    """Models an exit.

    Agents entering the polygon defining the exit will be removed at the
    beginning of the next iteration, i.e. agents will be inside the specified
    polygon for one frame.
    """

    def __init__(self, backing):
        self._obj = backing

    def count_targeting(self):
        """
        Returns:
            Number of agents currently targeting this stage.
        """
        return self._obj.count_targeting()
