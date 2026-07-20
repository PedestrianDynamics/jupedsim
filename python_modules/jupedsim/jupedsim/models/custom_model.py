# SPDX-License-Identifier: LGPL-3.0-or-later
from __future__ import annotations

from abc import ABC, abstractmethod
from typing import (
    TYPE_CHECKING,
    Protocol,
    runtime_checkable,
)

if TYPE_CHECKING:
    from jupedsim.geometry import Geometry
    from jupedsim.neighborhood import NeighborhoodSearch


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

        The ``state`` object you receive (and every neighbor state returned
        from a neighborhood query) is the agent's *live* state, shared by
        reference with the running simulation for performance. JuPedSim
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

        The neighbor states returned from neighborhood queries are only valid
        for the duration of the callback. Never store them. Calling mutating
        methods on the simulation (``add_agent``, ``mark_agent_for_removal``,
        journey or stage mutation) from within a callback raises
        :class:`~jupedsim.SimulationError`.
    """

    @abstractmethod
    def compute_next_state(
        self,
        dt: float,
        state: CustomModelAgentState,
        destination: tuple[float, float],
        geometry: Geometry,
        neighborhood_search: NeighborhoodSearch,
    ) -> CustomModelAgentState:
        """Compute one update for the agent in ``state``.

        ``state`` is the agent's own state object as passed to ``add_agent``
        (respectively as returned from the previous call). ``destination`` is
        the agent's current routing waypoint. ``neighborhood_search`` queries
        the frozen states of the current generation; results exclude the agent
        itself and are filtered by line-of-sight visibility.
        """

    def check_model_constraint(
        self,
        state: CustomModelAgentState,
        neighborhood_search: NeighborhoodSearch,
        geometry: Geometry,
    ) -> None:
        """Raise an exception when ``state`` violates this model's constraints.

        ``neighborhood_search`` is bound to the agent being validated and is
        deliberately not visibility-filtered: overlap checks must see agents
        through walls.
        """
        pass

    def _compute_next_state(
        self,
        dt,
        state,
        destination,
        geometry,
        neighbor_query,
    ) -> CustomModelAgentState:
        from jupedsim.geometry import Geometry
        from jupedsim.neighborhood import NeighborhoodSearch

        return self.compute_next_state(
            dt,
            state,
            destination,
            Geometry(geometry),
            NeighborhoodSearch(neighbor_query),
        )

    def _check_model_constraint(
        self,
        state,
        neighbor_query,
        geometry,
    ) -> None:
        from jupedsim.geometry import Geometry
        from jupedsim.neighborhood import NeighborhoodSearch

        self.check_model_constraint(
            state,
            NeighborhoodSearch(neighbor_query),
            Geometry(geometry),
        )
