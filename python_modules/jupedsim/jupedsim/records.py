# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass

from jupedsim.pyjupedsim import Location
from jupedsim.target import RouteTarget


@dataclass(frozen=True, slots=True)
class TargetReachedRecord:
    """An agent reached its target, see :meth:`~jupedsim.Simulation.reached_targets`."""

    agent_id: int
    """Id of the agent."""
    target: RouteTarget | Location
    """The target on the agent's route, or the location it was steered to."""
    time: float
    """Simulation time in seconds at which the agent reached it."""
    location: Location
    """Where the agent stood."""


@dataclass(frozen=True, slots=True)
class RemovalRecord:
    """An agent left the simulation, see :meth:`~jupedsim.Simulation.removed_agents`."""

    agent_id: int
    """Id of the removed agent."""
    exit: RouteTarget | None
    """The exit it left through, ``None`` after
    :meth:`~jupedsim.Simulation.mark_agent_for_removal`."""
    time: float
    """Simulation time in seconds of the removal."""
