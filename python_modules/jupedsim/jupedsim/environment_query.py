# SPDX-License-Identifier: LGPL-3.0-or-later
from __future__ import annotations

from typing import TYPE_CHECKING, Callable

import jupedsim.native as py_jps

if TYPE_CHECKING:
    from jupedsim.agent import _TransientAgent
    from jupedsim.linesegment import LineSegment


class EnvironmentQuery:
    """Read-only spatial query object valid for the duration of one model callback.

    Passed to :meth:`~jupedsim.models.custom_model.CustomOperationalModel.compute_next_state`
    and :meth:`~jupedsim.models.custom_model.CustomOperationalModel.check_model_constraint`.
    Do not store instances beyond the callback they were created in.

    Example — visibility-filtered neighborhood::

        def compute_next_state(self, dt, ped, env_query):
            neighbors = env_query.other_agents_in_range(
                ped, 5.0,
                lambda n: env_query.no_wall_between(ped.position, n.position)
            )
    """

    def __init__(self, obj: py_jps.EnvironmentQuery) -> None:
        self._obj = obj

    def other_agents_in_range(
        self,
        agent: _TransientAgent,
        radius: float,
        predicate: Callable[[_TransientAgent], bool] | None = None,
    ) -> list[_TransientAgent]:
        """Return agents within *radius* of *agent*, excluding *agent* itself.

        Args:
            agent: The querying agent (automatically excluded from results).
            radius: Search radius in metres.
            predicate: Optional callable ``(neighbor) -> bool``. Only agents for
                which the predicate returns ``True`` are included. Use
                :meth:`no_wall_between` to filter by line-of-sight, or compose
                multiple predicates with ``lambda n: p1(n) and p2(n)``.

        Returns:
            List of transient agent views. Only valid during the current callback.
        """
        from jupedsim.agent import _TransientAgent

        raw_agent = agent._native if hasattr(agent, "_native") else agent
        neighbors = [
            _TransientAgent(a)
            for a in self._obj.other_agents_in_range(raw_agent, radius)
        ]
        if predicate is None:
            return neighbors
        return [n for n in neighbors if predicate(n)]

    def no_wall_between(
        self,
        from_pos: tuple[float, float],
        to_pos: tuple[float, float],
    ) -> bool:
        """Return ``True`` when the straight line from *from_pos* to *to_pos*
        is not intersected by any geometry boundary (i.e. no wall blocks it).

        Args:
            from_pos: Observer position as ``(x, y)`` in metres.
            to_pos: Target position as ``(x, y)`` in metres.

        Returns:
            ``True`` when line-of-sight is unobstructed.

        Example::

            neighbors = env_query.other_agents_in_range(
                ped, 5.0,
                lambda n: env_query.no_wall_between(ped.position, n.position)
            )
        """
        return self._obj.no_wall_between(from_pos, to_pos)

    def line_segments_in_grid_cell_distance(
        self,
        position: tuple[float, float],
    ) -> list[LineSegment]:
        """Return geometry boundary segments near *position* using the spatial grid index.

        Faster than :meth:`line_segments_in_range` but may include segments
        slightly beyond the exact grid-cell boundary.

        Args:
            position: Query point as ``(x, y)`` in metres.

        Returns:
            List of :class:`~jupedsim.linesegment.LineSegment` objects.
        """
        from jupedsim.linesegment import LineSegment

        return [
            LineSegment(ls)
            for ls in self._obj.line_segments_in_grid_cell_distance(position)
        ]

    def line_segments_in_range(
        self,
        position: tuple[float, float],
        distance: float,
    ) -> list[LineSegment]:
        """Return geometry boundary segments within *distance* of *position*.

        Args:
            position: Query point as ``(x, y)`` in metres.
            distance: Maximum distance in metres.

        Returns:
            List of :class:`~jupedsim.linesegment.LineSegment` objects.
        """
        from jupedsim.linesegment import LineSegment

        return [
            LineSegment(ls)
            for ls in self._obj.line_segments_in_range(position, distance)
        ]

    def intersects_any(self, line_segment) -> bool:
        """Return ``True`` when *line_segment* intersects any geometry boundary.

        Args:
            line_segment: A :class:`~jupedsim.linesegment.LineSegment` or the
                underlying ``py_jps.LineSegment``.
        """
        raw = (
            line_segment._obj if hasattr(line_segment, "_obj") else line_segment
        )
        return self._obj.intersects_any(raw)

    def inside_geometry(self, position: tuple[float, float]) -> bool:
        """Return ``True`` when *position* lies inside the walkable area.

        Args:
            position: Point to test as ``(x, y)`` in metres.
        """
        return self._obj.inside_geometry(position)
