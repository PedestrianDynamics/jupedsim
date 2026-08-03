# SPDX-License-Identifier: LGPL-3.0-or-later
from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:
    from jupedsim.agent_view import AgentStep, AgentView


class CustomOperationalModel(ABC):
    """Base class for operational models implemented in Python.

    Subclasses implement :meth:`compute_next_state` and optionally
    :meth:`check_model_constraint`. Constraint violations should be reported by
    raising an exception.

    Models only see their surroundings based on their current position.

    .. warning::

        **Per-agent model state is live and shared -- never mutate it in place.**

        The ``state`` object you receive (and every neighbor's ``.state``
        returned from a neighborhood query) is the agent's *live* state, shared
        by reference with the running simulation for performance. JuPedSim
        advances agents in two phases per step: it first *computes* every
        agent's update from the current state of all agents, then *applies* all
        updates together. Mutating ``state`` (or a neighbor's) during the
        compute phase changes state that other agents are still reading in the
        same step, silently breaking the compute-then-apply ordering and
        producing order-dependent results.

        The only correct way to change state is to return a new state object
        from :meth:`compute_next_state` -- returning ``state`` itself
        (even unchanged) raises an error; use
        ``dataclasses.replace(state, ...)``. Make your state type
        immutable -- a ``@dataclass(frozen=True)`` -- so accidental in-place
        writes raise immediately instead of silently corrupting the
        simulation.

    .. warning::

        The ``step`` object passed to the callbacks (and the neighbor objects
        returned from neighborhood queries) are transient views that are only
        valid for the duration of the callback. Never store them. Calling
        mutating methods on the simulation (``add_agent``,
        ``mark_agent_for_removal``, journey or stage mutation) from within a
        callback raises :class:`~jupedsim.SimulationError`.
    """

    @abstractmethod
    def compute_next_state(
        self,
        state: Any,
        step: AgentStep,
    ) -> tuple[Any, tuple[float, float]]:
        """Compute one update for the agent this model is called for.

        Args:
            state: The agent's current model state, the object you handed to
                :meth:`~jupedsim.simulation.Simulation.add_agent`.
            step: What the agent perceives during this step, relative to where
                it stands. Only valid for the duration of this call.

        Returns:
            A pair ``(new_state, movement)``. ``new_state`` must be a new
            object -- returning ``state`` unchanged raises an error.
            ``movement`` is the offset ``(dx, dy)`` the agent wants
            to move by; the simulation applies it as given.
        """

    def check_model_constraint(
        self,
        state: Any,
        view: AgentView,
    ) -> None:
        """Raise an exception when *state* violates this model's constraints.

        Args:
            state: The model state to validate.
            view: What the agent perceives of its surroundings.
        """
        pass

    def _compute_next_state(
        self,
        state,
        step,
    ) -> tuple[Any, tuple[float, float]]:
        from jupedsim.agent_view import AgentStep

        return self.compute_next_state(state, AgentStep(step))

    def _check_model_constraint(
        self,
        state,
        view,
    ) -> None:
        from jupedsim.agent_view import AgentView

        self.check_model_constraint(state, AgentView(view))
