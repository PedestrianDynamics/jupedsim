# SPDX-License-Identifier: LGPL-3.0-or-later
"""Helpers of :class:`~jupedsim.simulation.Simulation` and
:class:`~jupedsim.agent.Agent` that turn handles into native arguments."""

import math
import re
from typing import Any

from jupedsim.pyjupedsim import (
    AnticipationVelocityModel,
    AnticipationVelocityModelState,
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3,
    CollisionFreeSpeedModelV3State,
    GeneralizedCentrifugalForceModel,
    GeneralizedCentrifugalForceModelState,
    SimulationError,
    SocialForceModel,
    SocialForceModelState,
    WarpDriverModel,
    WarpDriverModelState,
    _CustomModelState,
)
from jupedsim.pyjupedsim import (
    Location as _NativeLocation,
)
from jupedsim.pyjupedsim import Rule as _NativeRule
from jupedsim.route import Route, _label, _require_stage
from jupedsim.stages import Exit, Waypoint, _check_region
from jupedsim.surface import Location

STATE_TYPES = (
    GeneralizedCentrifugalForceModelState,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3State,
    AnticipationVelocityModelState,
    SocialForceModelState,
    WarpDriverModelState,
)

DEFAULT_STATES = {
    CollisionFreeSpeedModel: CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2: CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3: CollisionFreeSpeedModelV3State,
    AnticipationVelocityModel: AnticipationVelocityModelState,
    SocialForceModel: SocialForceModelState,
    WarpDriverModel: WarpDriverModelState,
    GeneralizedCentrifugalForceModel: lambda: (
        GeneralizedCentrifugalForceModelState(orientation=(1.0, 0.0))
    ),
}


def default_state_factory(model: Any) -> Any:
    """Return the callable that builds the model's default agent state, or
    None for custom models."""
    return DEFAULT_STATES.get(type(model))


def native_location(simulation: Any, value: Any, name: str) -> _NativeLocation:
    if not isinstance(value, Location):
        raise TypeError(f"{name} must be a Location, e.g. floor0.at(x, y)")
    _check_region(
        value.region, name, simulation._surface, simulation._region_count
    )
    return simulation._obj.get_location(value.x, value.y, value.region.id)


def first_stage(
    simulation: Any, route: Route, start: Waypoint | Exit | None
) -> Waypoint | Exit:
    if not isinstance(route, Route):
        raise TypeError("route must be a Route, e.g. jps.Route([wp, exit])")
    route._check_simulation(simulation)
    stage = route.start if start is None else _require_stage(start)
    if stage is None:
        raise ValueError(
            "route given as pairs has no default start; "
            "pass start= here or to Route"
        )
    route._check_member(stage)
    return stage


# The native travel time query raises this while iterating or once closed.
_NOT_ALLOWED = re.compile(r"travel_time is not allowed in state ")


def check_reachable(
    simulation: Any, stage: Waypoint | Exit, location: Location
) -> None:
    """Raise if ``stage`` cannot be reached from ``location``.

    While the simulation iterates or after it is closed nothing is checked:
    the native call that follows raises for its own operation.
    """
    try:
        time = simulation.travel_time(stage, location)
    except SimulationError as error:
        if _NOT_ALLOWED.match(str(error)):
            return
        raise
    if math.isinf(time):
        raise ValueError(f"{_label(stage)} cannot be reached from {location!r}")


def agent_state(simulation: Any, state: Any) -> Any:
    if isinstance(state, STATE_TYPES):
        return state
    if state is not None:
        return _CustomModelState(state)
    if simulation._default_state is None:
        raise ValueError(
            "custom models need state=; pass your model's agent state"
        )
    return simulation._default_state()


def direct_steering(simulation: Any) -> tuple[int, int]:
    """Return the route and stage ids of the hidden direct steering
    route, creating them on first use."""
    if simulation._direct is None:
        stage_id = simulation._obj.create_direct_steering_stage()
        route_id = simulation._obj.add_route(
            {stage_id: _NativeRule.create_none_transition()}
        )
        simulation._direct = (route_id, stage_id)
    return simulation._direct
