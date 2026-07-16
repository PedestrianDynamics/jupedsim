# SPDX-License-Identifier: LGPL-3.0-or-later

"""
Pure Python wrapper for the neighbor query C++ bindings.

This module provides a Pythonic interface to the underlying C++ NeighborQuery
implementation, making it easier to work with spatial queries in operational models.
"""

from __future__ import annotations

from typing import Any, List, Tuple

import jupedsim.native as py_jps


class NeighborhoodSearch:
    """
    Pure Python wrapper for the C++ NeighborQuery interface.

    Provides efficient spatial queries for finding the model states of
    neighboring agents within a given radius. The query is bound to the agent
    whose callback received it: results never include that agent itself. In
    :meth:`~jupedsim.models.custom_model.CustomOperationalModel.compute_next_state`
    results are additionally filtered by line-of-sight visibility; in
    ``check_model_constraint`` they are not.

    Example:
        >>> neighbor_states = neighborhood.get_neighboring_agents(
        ...     position=(5.0, 5.0),
        ...     radius=2.0
        ... )
    """

    def __init__(self, obj: py_jps.NeighborQuery):
        """
        Wrap an existing C++ NeighborQuery.
        """
        self._obj = obj

    def get_neighboring_agents(
        self, position: Tuple[float, float], radius: float
    ) -> List[Any]:
        """
        Get the model states of all agents within a certain radius of a position.

        Uses the underlying spatial grid for efficient O(1) average-case
        lookup time (worst case depends on number of agents per cell).

        Args:
            position: Query position as (x, y) tuple [m]
            radius: Search radius [m]

        Returns:
            List of per-agent model states within the radius; for custom-model
            simulations these are the user-defined state objects the neighbors
            were added with. Empty list if no agents found. The returned states
            belong to the frozen current generation and are only valid for the
            duration of the custom-model callback; never store or mutate them.

        Raises:
            ValueError: If radius < 0

        Example:
            >>> neighbor_states = neighborhood.get_neighboring_agents(
            ...     position=(5.0, 5.0),
            ...     radius=2.5
            ... )
            >>> print(f"Found {len(neighbor_states)} neighbors")
        """
        if radius < 0:
            raise ValueError(f"radius must be non-negative, got {radius}")

        neighbors = self._obj.get_neighboring_agents(position, radius)
        return [
            x.model if isinstance(x, py_jps._CustomModelState) else x
            for x in neighbors
        ]
