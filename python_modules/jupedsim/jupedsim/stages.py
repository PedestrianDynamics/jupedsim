# SPDX-License-Identifier: LGPL-3.0-or-later
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
