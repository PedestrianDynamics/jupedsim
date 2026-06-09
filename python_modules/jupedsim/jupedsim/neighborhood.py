# SPDX-License-Identifier: LGPL-3.0-or-later

"""
Pure Python wrapper for neighborhood search C++ bindings.

This module provides a Pythonic interface to the underlying C++ NeighborhoodSearch
implementation, making it easier to work with spatial queries in operational models.
"""

from typing import TYPE_CHECKING, List, Tuple

import jupedsim.native as py_jps


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

    # def add_agent(self, agent: Agent) -> None:
    # """
    # Add a single agent to the search structure.

    # Args:
    # agent: GenericAgent object to add

    # Note:
    # - The agent's position must be set before calling this method
    # - Multiple calls with the same agent will add it multiple times
    # - Use update() for bulk updates of multiple agents
    # """
    # self._obj.add_agent(agent._obj)

    # def remove_agent(self, agent: Agent) -> None:
    # """
    # Remove a single agent from the search structure.

    # Args:
    # agent: GenericAgent object to remove

    # Note:
    # Only removes one instance of the agent. If add_agent was called
    # multiple times with the same agent, you need to call remove_agent
    # the same number of times.
    # """
    # self._obj.remove_agent(agent._obj)

    # def update(self, agents: List[Agent]) -> None:
    # """
    # Update the search structure with a new list of agents.

    # This is the preferred method for updating multiple agents at once.
    # Clears the old structure and rebuilds it with the given agents.

    # Args:
    # agents: List of GenericAgent objects

    # Example:
    # >>> agents = [agent1, agent2, agent3]
    # >>> neighborhood.update(agents)
    # """
    # self._obj.update(agents)

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

        unwrapped_list = self._obj.get_neighboring_agents(position, radius)

        native_agent_list = []
        for cpp_agent in unwrapped_list:
            native_agent_list.append(Agent(cpp_agent))

        return native_agent_list
