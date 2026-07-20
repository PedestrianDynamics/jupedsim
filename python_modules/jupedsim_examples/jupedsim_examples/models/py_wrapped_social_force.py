# SPDX-License-Identifier: LGPL-3.0-or-later
"""Custom model that forwards every computation to the native SocialForceModel.

This model exists as a machinery confirmation and as a template for multimodal
simulations: the per-agent state embeds a concrete native model state
(:class:`jupedsim.SocialForceModelState`), and the callbacks delegate to the
exposed native model. For identical setups it produces results bit-identical to
running the native :class:`jupedsim.SocialForceModel` directly. A multimodal
model can follow the same pattern while holding different concrete states per
agent and dispatching to different native models dynamically.

The neighborhood is forwarded, not pre-collected: the native model calls the
forwarding query with its own cutoff radius, so this code needs no per-model
knowledge; it only translates each wrapped neighbor state to the embedded
native state.
"""

from dataclasses import dataclass
from typing import Callable

import jupedsim as jps
from jupedsim.geometry import Geometry
from jupedsim.models.custom_model import (
    CustomModelAgentState,
    CustomOperationalModel,
)
from jupedsim.neighborhood import NeighborhoodSearch


@dataclass(frozen=True)
class WrappedSocialForceModelState(CustomModelAgentState):
    """Per-agent state: the native SFM state plus the mirrored position.

    ``position`` must always equal ``sfm_state.position``; the framework reads
    the agent position from ``position`` while the native model computes on
    ``sfm_state``.
    """

    position: tuple[float, float]
    sfm_state: jps.SocialForceModelState

    @staticmethod
    def create(**kwargs) -> "WrappedSocialForceModelState":
        """Build a wrapped state from `jupedsim.SocialForceModelState` kwargs."""
        sfm_state = jps.SocialForceModelState(**kwargs)
        return WrappedSocialForceModelState(
            position=sfm_state.position, sfm_state=sfm_state
        )


def _forwarding_query(
    neighborhood_search: NeighborhoodSearch,
) -> Callable[[tuple[float, float], float], list]:
    """Forward the simulation's neighbor query, translating wrapped states.

    The native model invokes the returned callable with its own query radius;
    each wrapped neighbor state is mapped to the embedded native SFM state.
    """

    def query(position: tuple[float, float], radius: float) -> list:
        return [
            neighbor.sfm_state
            for neighbor in neighborhood_search.get_neighboring_agents(
                position, radius
            )
        ]

    return query


class WrappedSocialForceModel(CustomOperationalModel):
    """Pure forwarding wrapper around the native C++ SocialForceModel."""

    def __init__(self, *, body_force: float = 120000, friction: float = 240000):
        CustomOperationalModel.__init__(self)
        self._native = jps.SocialForceModel(
            body_force=body_force, friction=friction
        )

    def compute_next_state(
        self,
        dt: float,
        state: WrappedSocialForceModelState,
        destination: tuple[float, float],
        geometry: Geometry,
        neighborhood_search: NeighborhoodSearch,
    ) -> WrappedSocialForceModelState:
        next_sfm_state = self._native.compute_next_state(
            dt=dt,
            state=state.sfm_state,
            destination=destination,
            neighbor_query=_forwarding_query(neighborhood_search),
            geometry=geometry._obj,
        )
        return WrappedSocialForceModelState(
            position=next_sfm_state.position, sfm_state=next_sfm_state
        )

    def check_model_constraint(
        self,
        state: WrappedSocialForceModelState,
        neighborhood_search: NeighborhoodSearch,
        geometry: Geometry,
    ) -> None:
        self._native.check_model_constraint(
            state=state.sfm_state,
            neighbor_query=_forwarding_query(neighborhood_search),
            geometry=geometry._obj,
        )
