# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps


class Location:
    """A place on the walkable surface.

    Locations are always obtained from the simulation and never built
    directly:

    .. code:: python

        sim.get_location(x, y, z_hint=3.0)
        sim.agent(id).location

    Raw coordinates become a place exactly once, in
    :meth:`~jupedsim.simulation.Simulation.get_location`: the geometry has to
    stand for a location, and on stacked floors an ``(x, y)`` on its own does
    not say which floor is meant. Afterwards the location travels -- pass it
    wherever a place is wanted instead of coordinates.

    A location is read-only and reads only what a caller can act on. It stays
    valid as long as the simulation it came from exists, and it does not
    follow an agent: reading :attr:`~jupedsim.agent.Agent.location` again
    gives where the agent stands now.
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
        """Height of the surface here, in metres.

        Zero throughout a simulation built from a polygon.
        """
        return self._obj.z

    def __repr__(self) -> str:
        return f"Location({self.x}, {self.y}, {self.z})"
