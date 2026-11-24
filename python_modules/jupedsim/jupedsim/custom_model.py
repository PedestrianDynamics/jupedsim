# SPDX-License-Identifier: LGPL-3.0-or-later
from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:
    from jupedsim.pyjupedsim import AgentStep, AgentView


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

        The ``step`` and ``view`` objects passed to the callbacks (and every
        view derived from them) are transient views that are only valid for the
        duration of the callback; once it has returned, every access raises
        :class:`~jupedsim.SimulationError`. The neighbor objects returned from
        neighborhood queries are transient as well; never store them. Calling
        mutating methods on the simulation (``add_agent``,
        ``mark_agent_for_removal``, ``add_target``, ``add_route``,
        ``agent.reroute``) from within a callback raises
        :class:`~jupedsim.SimulationError`.
    """

    @abstractmethod
    def compute_next_state(
        self,
        state: Any,
        step: "AgentStep",
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

    def seed_agent_state(self, state: Any) -> Any:
        """Prepare the state of an agent that is being added.

        Called once for every agent added to the simulation, after
        :meth:`check_model_constraint` accepted it, in the order the agents are
        added. A model delegating to a built-in one that draws random numbers
        (e.g. :class:`~jupedsim.AnticipationVelocityModel` or
        :class:`~jupedsim.WarpDriverModel`) has to seed the delegated state
        here with :meth:`~jupedsim.OperationalModel.seed_state`; otherwise the
        built-in model raises on its first step.

        .. code:: python

            def seed_agent_state(self, state):
                return replace(state, sub=self._inner.seed_state(state.sub))

        Args:
            state: The agent's model state, the object you handed to
                :meth:`~jupedsim.simulation.Simulation.add_agent`.

        Returns:
            The state the agent starts with, of the same type as ``state``.
            The default returns ``state`` unchanged.

        Raises:
            SimulationError: Raised by the simulation if the returned object
                is not of the type of ``state``, e.g. ``None``.
        """
        return state

    def check_model_constraint(
        self,
        state: Any,
        view: "AgentView",
    ) -> None:
        """Raise an exception when *state* violates this model's constraints.

        Args:
            state: The model state to validate.
            view: What the agent perceives of its surroundings.
        """
        pass
