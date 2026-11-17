# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass

from jupedsim.pyjupedsim import Location
from jupedsim.target import RouteTarget


@dataclass(frozen=True, slots=True)
class TargetCompletionRecord:
    """An agent completed its target, see :meth:`~jupedsim.Simulation.target_completions`."""

    agent_id: int
    """Id of the agent."""
    target: RouteTarget | Location
    """The target on the agent's route, or the location it was steered to."""
    time: float
    """Simulation time in seconds of the completion."""
    location: Location
    """Where the agent stood at the completion."""


@dataclass(frozen=True, slots=True)
class RemovalRecord:
    """An agent left the simulation, see :meth:`~jupedsim.Simulation.removed_agents`."""

    agent_id: int
    """Id of the removed agent."""
    exit: RouteTarget | None
    """The exit it left through, ``None`` after
    :meth:`~jupedsim.Simulation.mark_agent_for_removal`."""
    time: float
    """Simulation time in seconds of the exit completion, or of the removal."""
