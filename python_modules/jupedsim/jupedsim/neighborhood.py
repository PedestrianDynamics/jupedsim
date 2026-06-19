# SPDX-License-Identifier: LGPL-3.0-or-later

"""
Pure Python wrapper for neighborhood search C++ bindings.

This module provides a Pythonic interface to the underlying C++ NeighborhoodSearch
implementation, making it easier to work with spatial queries in operational models.
"""

from __future__ import annotations

from typing import TYPE_CHECKING, List, Tuple

import jupedsim.native as py_jps

if TYPE_CHECKING:
    from jupedsim.agent import Agent


class NeighborhoodSearch:
    """
    Pure Python wrapper for the C++ NeighborhoodSearch class.

    Provides efficient spatial queries for finding neighboring agents within
    a given radius. Uses a grid-based data structure for O(1) cell lookups
    with configurable cell size.


    Example:
        >>> neighbors = neighborhood.get_neighboring_agents(
        ...     position=(5.0, 5.0),
        ...     radius=2.0
        ... )
    """

    def __init__(self, obj: py_jps.NeighborhoodSearch):
        """
        Wrap an existing C++
        """
        self._obj = obj

    def get_neighboring_agents(
        self, position: Tuple[float, float], radius: float
    ) -> "List[Agent]":
        """
        Get all agents within a certain radius of a position.

        Uses the underlying spatial grid for efficient O(1) average-case
        lookup time (worst case depends on number of agents per cell).

        Args:
            position: Query position as (x, y) tuple [m]
            radius: Search radius [m]

        Returns:
            List of GenericAgent objects within the radius.
            Empty list if no agents found.

        Raises:
            ValueError: If radius < 0

        Example:
            >>> neighbors = neighborhood.get_neighboring_agents(
            ...     position=(5.0, 5.0),
            ...     radius=2.5
            ... )
            >>> print(f"Found {len(neighbors)} neighbors")
        """
        from jupedsim.agent import Agent

        if radius < 0:
            raise ValueError(f"radius must be non-negative, got {radius}")

        neighbors = self._obj.get_neighboring_agents(position, radius)
        return [Agent(x) for x in neighbors]
