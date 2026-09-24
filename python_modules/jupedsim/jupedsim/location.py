# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps


class Location:
    """A point on the walkable surface, together with the region it lies in.

    Locations cannot be created directly, get them from the simulation:

    .. code:: python

        sim.get_location(x, y, region_id=upper_floor)
        sim.agent(agent_id).location

    A location is read-only and stays valid as long as its simulation exists.
    It does not move with an agent: read
    :attr:`~jupedsim.agent.Agent.location` again to get the agent's current
    location.
    """

    def __init__(self, obj: py_jps.Location) -> None:
        """Do not use.

        Retrieve locations from the simulation.
        """
        self._obj = obj

    @property
    def x(self) -> float:
        """x coordinate in metres."""
        return self._obj.x

    @property
    def y(self) -> float:
        """y coordinate in metres."""
        return self._obj.y

    @property
    def z(self) -> float:
        """Height of the surface here, in metres."""
        return self._obj.z

    @property
    def region_id(self) -> int:
        """Region this location lies in."""
        return self._obj.region_id

    def __repr__(self) -> str:
        return f"Location({self.x}, {self.y}, {self.z})"
