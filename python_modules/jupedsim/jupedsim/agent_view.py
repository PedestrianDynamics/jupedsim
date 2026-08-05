# SPDX-License-Identifier: LGPL-3.0-or-later
from __future__ import annotations

from typing import TYPE_CHECKING, Any, Callable

import jupedsim.native as py_jps

if TYPE_CHECKING:
    from jupedsim.linesegment import LineSegment


class NeighborView:
    """A neighboring agent as seen from the agent that asked for it.

    Obtained from :meth:`AgentView.other_agents_in_range`. Only valid for the
    duration of the callback it was created in; never store it.
    """

    def __init__(self, obj: py_jps.NeighborView) -> None:
        """Do not use.

        Neighbor views are created by neighborhood queries.
        """
        self._obj = obj

    @property
    def relative_position(self) -> tuple[float, float]:
        """Vector from the querying agent to this neighbor."""
        return self._obj.relative_position

    @property
    def state(self) -> Any:
        """Model state of this neighbor.

        Treat it as read-only: state may only be changed by returning a new
        state object from ``compute_next_state``.
        """
        return self._obj.state

    def __repr__(self) -> str:
        return f"NeighborView(relative_position={self.relative_position})"


class WallView:
    """A wall segment as seen from the agent that asked for it.

    Obtained from :meth:`AgentView.walls_nearby` or
    :meth:`AgentView.walls_in_range`. The view is always relative to the
    agent - as if the agent sits at the origin.
    """

    def __init__(self, obj: py_jps.WallView) -> None:
        """Do not use.

        Wall views are created by wall queries.
        """
        self._obj = obj

    @property
    def segment(self) -> LineSegment:
        """The segment itself, relative to the agent."""
        from jupedsim.linesegment import LineSegment

        return LineSegment(self._obj.segment)

    @property
    def closest_point(self) -> tuple[float, float]:
        """The point on the segment closest to the agent."""
        return self._obj.closest_point

    @property
    def distance(self) -> float:
        """Distance to that point."""
        return self._obj.distance

    @property
    def normal(self) -> tuple[float, float]:
        """Unit vector pointing from the wall towards the agent.

        This is the direction a repulsion acts in. It is ``(0.0, 0.0)`` for an
        agent standing exactly on the wall, where no direction exists.
        """
        return self._obj.normal

    def __repr__(self) -> str:
        return f"WallView(distance={self.distance}, normal={self.normal})"


class AgentView:
    """What an agent perceives of its surroundings, relative to where it stands.

    Passed to
    :meth:`~jupedsim.models.custom_model.CustomOperationalModel.check_model_constraint`
    and, as :class:`AgentStep`, to
    :meth:`~jupedsim.models.custom_model.CustomOperationalModel.compute_next_state`.
    Do not store instances beyond the callback they were created in.

    Example — visibility-filtered neighborhood::

        def compute_next_state(self, state, step):
            boundaries = step.walls_nearby()
            neighbors = step.other_agents_in_range(
                5.0,
                lambda n: step.no_geometry_between(n.relative_position, boundaries),
            )
    """

    def __init__(self, obj: py_jps.AgentView) -> None:
        """Do not use.

        Agent views are created by the simulation for the duration of a
        custom-model callback.
        """
        self._obj = obj

    def other_agents_in_range(
        self,
        radius: float,
        predicate: Callable[[NeighborView], bool] | None = None,
    ) -> list[NeighborView]:
        """Return agents within *radius*, excluding the agent itself.

        Args:
            radius: Search radius.
            predicate: Optional callable ``(neighbor) -> bool``. Only neighbors
                for which the predicate returns ``True`` are included. Use
                :meth:`no_geometry_between` to filter by line-of-sight, or
                compose multiple predicates with ``lambda n: p1(n) and p2(n)``.

        Returns:
            List of :class:`NeighborView`. Only valid during the current callback.
        """
        neighbors = [
            NeighborView(n) for n in self._obj.other_agents_in_range(radius)
        ]
        if predicate is None:
            return neighbors
        return [n for n in neighbors if predicate(n)]

    def no_geometry_between(
        self,
        relative_position: tuple[float, float],
        boundaries: list[WallView],
    ) -> bool:
        """Return ``True`` when the straight line from the agent to the point
        *relative_position* away is not intersected by any of *walls*.

        Args:
            relative_position: Offset from the agent as ``(dx, dy)``.
            boundaries: List of geometry boundary segments.
        """

        return self._obj.no_geometry_between(
            relative_position, [b._obj for b in boundaries]
        )

    def inside_geometry(self, relative_position: tuple[float, float]) -> bool:
        """Return ``True`` when the point reached by moving *relative_position*
        lies inside the walkable area.

        Args:
            relative_position: Offset from the agent as ``(dx, dy)``.
        """
        return self._obj.inside_geometry(relative_position)

    def walls_nearby(self) -> list[WallView]:
        """Return the walls in the grid cells around the agent.

        Faster than :meth:`walls_in_range`, but an approximation of proximity
        rather than a radius: it returns whatever shares a cell neighborhood.

        Returns:
            List of :class:`WallView`, each as seen from the agent.
        """
        return [WallView(w) for w in self._obj.walls_nearby()]

    def walls_in_range(self, distance: float) -> list[WallView]:
        """Return the walls within *distance* of the agent.

        Args:
            distance: Maximum distance.

        Returns:
            List of :class:`WallView`, each as seen from the agent.
        """
        return [WallView(w) for w in self._obj.walls_in_range(distance)]


class AgentStep(AgentView):
    """An :class:`AgentView` plus what only holds for one step.

    Passed to
    :meth:`~jupedsim.models.custom_model.CustomOperationalModel.compute_next_state`.
    """

    @property
    def dt(self) -> float:
        """Duration of this simulation step in seconds."""
        return self._obj.dt

    @property
    def to_next_target(self) -> tuple[float, float]:
        """Vector from the agent to its next target."""
        return self._obj.to_next_target
