# SPDX-License-Identifier: LGPL-3.0-or-later
from __future__ import annotations

from abc import ABC, abstractmethod
from typing import (
    TYPE_CHECKING,
    Protocol,
    runtime_checkable,
)

if TYPE_CHECKING:
    from jupedsim.agent import _TransientAgent
    from jupedsim.environment_query import EnvironmentQuery


@runtime_checkable
class CustomModelAgentState(Protocol):
    """Structural interface for per-agent model state of custom models.

    Any object exposing a ``position`` attribute of type ``tuple[float, float]``
    satisfies this protocol -- explicitly subclassing it is supported but not
    required. The runtime check performed when adding an agent verifies
    attribute presence only; value types are validated by the simulation
    itself.

    Objects satisfying this protocol are passed directly to
    :meth:`~jupedsim.simulation.Simulation.add_agent` as the agent's initial
    state. It should be an immutable object -- a ``@dataclass(frozen=True)``
    is strongly recommended -- because the simulation shares it live with your
    model during each step (see :class:`CustomOperationalModel`).
    """

    position: tuple[float, float]


class CustomOperationalModel(ABC):
    """Base class for operational models implemented in Python.

    Subclasses implement :meth:`compute_next_state` and optionally
    :meth:`check_model_constraint`. Constraint violations should be reported by
    raising an exception.

    .. warning::

        **Per-agent model state is live and shared -- never mutate it in place.**

        The ``ped.model`` object you receive (and every neighbor's ``.model``
        returned from a neighborhood query) is the agent's *live* state, shared
        by reference with the running simulation for performance. JuPedSim
        advances agents in two phases per step: it first *computes* every
        agent's update from the current state of all agents, then *applies* all
        updates together. Mutating ``ped.model`` (or a neighbor's) during the
        compute phase changes state that other agents are still reading in the
        same step, silently breaking the compute-then-apply ordering and
        producing order-dependent results.

        The only correct way to change state is to return a new state object
        from :meth:`compute_next_state` -- returning ``ped.model`` itself
        (even unchanged) raises an error; use
        ``dataclasses.replace(ped.model, ...)``. Make your state type
        immutable -- a ``@dataclass(frozen=True)`` -- so accidental in-place
        writes raise immediately instead of silently corrupting the
        simulation.

    .. warning::

        The ``ped`` object passed to the callbacks (and the neighbor objects
        returned from neighborhood queries) are transient views that are only
        valid for the duration of the callback. Never store them. Calling
        mutating methods on the simulation (``add_agent``,
        ``mark_agent_for_removal``, journey or stage mutation) from within a
        callback raises :class:`~jupedsim.SimulationError`.
    """

    @abstractmethod
    def compute_next_state(
        self,
        dt: float,
        ped: _TransientAgent,
        env_query: EnvironmentQuery,
    ) -> CustomModelAgentState:
        """Compute one update for ``ped``.

        Args:
            dt: Simulation time step in seconds.
            ped: The agent being updated. Only valid for the duration of this call.
            env_query: Spatial query object for the current step. Use it to
                find neighboring agents and query geometry. Only valid for the
                duration of this call.

        Returns:
            New agent state. Must be a new object -- returning ``ped.model``
            unchanged raises an error.
        """

    def check_model_constraint(
        self,
        ped: _TransientAgent,
        env_query: EnvironmentQuery,
    ) -> None:
        """Raise an exception when ``ped`` violates this model's constraints.

        Args:
            ped: The agent to validate.
            env_query: Spatial query object for the current step.
        """
        pass

    def _compute_next_state(
        self,
        dt,
        ped,
        env_query,
    ) -> CustomModelAgentState:
        from jupedsim.agent import _TransientAgent
        from jupedsim.environment_query import EnvironmentQuery

        return self.compute_next_state(dt, _TransientAgent(ped), EnvironmentQuery(env_query))

    def _check_model_constraint(
        self,
        ped,
        env_query,
    ) -> None:
        from jupedsim.agent import _TransientAgent
        from jupedsim.environment_query import EnvironmentQuery

        self.check_model_constraint(_TransientAgent(ped), EnvironmentQuery(env_query))
