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
        geometry: Geometry,
        neighborhood_search: NeighborhoodSearch,
    ) -> CustomModelAgentState:
        """Compute one update for ``ped``."""

    def check_model_constraint(
        self,
        ped: _TransientAgent,
        neighborhood_search: NeighborhoodSearch,
        geometry: Geometry,
    ) -> None:
        """Raise an exception when ``ped`` violates this model's constraints."""
        pass

    def _compute_next_state(
        self,
        dt,
        ped,
        geometry,
        neighborhood_search,
    ) -> CustomModelAgentState:
        from jupedsim.agent import _TransientAgent
        from jupedsim.geometry import Geometry
        from jupedsim.neighborhood import NeighborhoodSearch

        return self.compute_next_state(
            dt,
            _TransientAgent(ped),
            Geometry(geometry),
            NeighborhoodSearch(neighborhood_search),
        )

    def _check_model_constraint(
        self,
        ped,
        neighborhood_search,
        geometry,
    ) -> None:
        from jupedsim.agent import _TransientAgent
        from jupedsim.geometry import Geometry
        from jupedsim.neighborhood import NeighborhoodSearch

        self.check_model_constraint(
            _TransientAgent(ped),
            NeighborhoodSearch(neighborhood_search),
            Geometry(geometry),
        )
