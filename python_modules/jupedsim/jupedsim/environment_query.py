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

    Example — visibility-filtered neighborhood with a custom extra predicate::

        def compute_next_state(self, dt, ped, env_query):
            visible = env_query.visible_from(ped.position)

            def fast_enough(neighbor):
                return neighbor.model.desired_speed > 0.5

            neighbors = env_query.agents_in_range(
                ped, 5.0, lambda n: visible(n) and fast_enough(n)
            )
    """

    def __init__(self, obj: py_jps.EnvironmentQuery) -> None:
        self._obj = obj

    def agents_in_range(
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
                :meth:`visible_from` to filter by line-of-sight, or compose
                multiple predicates with ``lambda n: p1(n) and p2(n)``.

        Returns:
            List of transient agent views. Only valid during the current callback.
        """
        from jupedsim.agent import _TransientAgent

        raw_agent = agent._raw if isinstance(agent, _TransientAgent) else agent

        if predicate is None:
            return [
                _TransientAgent(a)
                for a in self._obj.agents_in_range(raw_agent, radius)
            ]

        def _wrapped(raw):
            return predicate(_TransientAgent(raw))

        return [
            _TransientAgent(a)
            for a in self._obj.agents_in_range(raw_agent, radius, _wrapped)
        ]

    def visible_from(
        self, position: tuple[float, float]
    ) -> Callable[[_TransientAgent], bool]:
        """Return a predicate that is ``True`` when *position* has unobstructed
        line-of-sight to the candidate agent.

        The returned callable is backed by the simulation geometry (stable for
        the simulation lifetime) and can be passed directly as the *predicate*
        argument to :meth:`agents_in_range`, or composed with other predicates.

        Args:
            position: Observer position as ``(x, y)`` in metres.

        Returns:
            Callable ``(neighbor: _TransientAgent) -> bool``.

        Example::

            visible = env_query.visible_from(ped.position)
            neighbors = env_query.agents_in_range(ped, 5.0, visible)
        """
        from jupedsim.agent import _TransientAgent

        raw_pred = self._obj.visible_from(position)

        def _pred(agent: _TransientAgent) -> bool:
            raw = agent._raw if isinstance(agent, _TransientAgent) else agent
            return raw_pred(raw)

        return _pred

    def line_segments_in_range(
        self,
        position: tuple[float, float],
        distance: float | None = None,
    ) -> list[LineSegment]:
        """Return geometry boundary segments near *position*.

        Args:
            position: Query point as ``(x, y)`` in metres.
            distance: If given, only segments within this exact distance are
                returned. If omitted, uses an approximate spatial index query
                (faster but may include slightly farther segments).

        Returns:
            List of :class:`~jupedsim.linesegment.LineSegment` objects.
        """
        from jupedsim.linesegment import LineSegment

        if distance is None:
            return [
                LineSegment(ls)
                for ls in self._obj.line_segments_in_range(position)
            ]
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
