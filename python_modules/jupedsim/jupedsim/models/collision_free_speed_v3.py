# SPDX-License-Identifier: LGPL-3.0-or-later

import warnings
from dataclasses import dataclass

try:
    from warnings import deprecated
except ImportError:
    from deprecated import deprecated


@dataclass(kw_only=True)
class CollisionFreeSpeedModelV3:
    """Collision Free Speed Model V3

    This variant keeps per-agent interaction parameters and uses bounded rotational steering
    around the desired direction.
    """

    pass


@dataclass(kw_only=True)
class CollisionFreeSpeedModelV3AgentParameters:
    """Agent parameters for Collision Free Speed Model V3."""

    position: tuple[float, float] = (0.0, 0.0)
    time_gap: float = 1.0
    desired_speed: float = 1.2
    radius: float = 0.2
    journey_id: int = 0
    stage_id: int = 0

    strength_neighbor_repulsion: float = 8.0
    range_neighbor_repulsion: float = 0.1
    strength_geometry_repulsion: float = 5.0
    range_geometry_repulsion: float = 0.02

    range_x_scale: float = 20.0
    range_y_scale: float = 8.0
    theta_max_upper_bound: float = 1.0
    agent_buffer: float = 0.0

    def __init__(
        self,
        *,
        position: tuple[float, float] = (0.0, 0.0),
        time_gap: float = 1.0,
        desired_speed: float = 1.2,
        v0: float | None = None,
        radius: float = 0.2,
        journey_id: int = 0,
        stage_id: int = 0,
        strength_neighbor_repulsion: float = 8.0,
        range_neighbor_repulsion: float = 0.1,
        strength_geometry_repulsion: float = 5.0,
        range_geometry_repulsion: float = 0.02,
        range_x_scale: float = 20.0,
        range_y_scale: float = 8.0,
        theta_max_upper_bound: float = 1.0,
        agent_buffer: float = 0.0,
    ):
        self.position = position
        self.time_gap = time_gap
        if v0 is not None:
            warnings.warn(
                "'v0' is deprecated, use 'desired_speed' instead.",
                DeprecationWarning,
                stacklevel=2,
            )
            self.desired_speed = v0
        else:
            self.desired_speed = desired_speed

        self.radius = radius
        self.journey_id = journey_id
        self.stage_id = stage_id

        self.strength_neighbor_repulsion = strength_neighbor_repulsion
        self.range_neighbor_repulsion = range_neighbor_repulsion
        self.strength_geometry_repulsion = strength_geometry_repulsion
        self.range_geometry_repulsion = range_geometry_repulsion

        self.range_x_scale = range_x_scale
        self.range_y_scale = range_y_scale
        self.theta_max_upper_bound = theta_max_upper_bound
        self.agent_buffer = agent_buffer

    @property
    @deprecated("deprecated, use 'desired_speed' instead.")
    def v0(self) -> float:
        return self.desired_speed

    @v0.setter
    @deprecated("deprecated, use 'desired_speed' instead.")
    def v0(self, v0):
        self.desired_speed = v0


class CollisionFreeSpeedModelV3State:
    def __init__(self, backing):
        self._obj = backing

    @property
    def time_gap(self) -> float:
        return self._obj.time_gap

    @time_gap.setter
    def time_gap(self, time_gap):
        self._obj.time_gap = time_gap

    @property
    def desired_speed(self) -> float:
        return self._obj.desired_speed

    @desired_speed.setter
    def desired_speed(self, desired_speed):
        self._obj.desired_speed = desired_speed

    @property
    @deprecated("deprecated, use 'desired_speed' instead.")
    def v0(self) -> float:
        return self._obj.desired_speed

    @v0.setter
    @deprecated("deprecated, use 'desired_speed' instead.")
    def v0(self, v0):
        self._obj.desired_speed = v0

    @property
    def radius(self) -> float:
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius

    @property
    def strength_neighbor_repulsion(self) -> float:
        return self._obj.strength_neighbor_repulsion

    @strength_neighbor_repulsion.setter
    def strength_neighbor_repulsion(self, strength_neighbor_repulsion):
        self._obj.strength_neighbor_repulsion = strength_neighbor_repulsion

    @property
    def range_neighbor_repulsion(self) -> float:
        return self._obj.range_neighbor_repulsion

    @range_neighbor_repulsion.setter
    def range_neighbor_repulsion(self, range_neighbor_repulsion):
        self._obj.range_neighbor_repulsion = range_neighbor_repulsion

    @property
    def strength_geometry_repulsion(self) -> float:
        return self._obj.strength_geometry_repulsion

    @strength_geometry_repulsion.setter
    def strength_geometry_repulsion(self, strength_geometry_repulsion):
        self._obj.strength_geometry_repulsion = strength_geometry_repulsion

    @property
    def range_geometry_repulsion(self) -> float:
        return self._obj.range_geometry_repulsion

    @range_geometry_repulsion.setter
    def range_geometry_repulsion(self, range_geometry_repulsion):
        self._obj.range_geometry_repulsion = range_geometry_repulsion

    @property
    def range_x_scale(self) -> float:
        return self._obj.range_x_scale

    @range_x_scale.setter
    def range_x_scale(self, range_x_scale):
        self._obj.range_x_scale = range_x_scale

    @property
    def range_y_scale(self) -> float:
        return self._obj.range_y_scale

    @range_y_scale.setter
    def range_y_scale(self, range_y_scale):
        self._obj.range_y_scale = range_y_scale

    @property
    def theta_max_upper_bound(self) -> float:
        return self._obj.theta_max_upper_bound

    @theta_max_upper_bound.setter
    def theta_max_upper_bound(self, theta_max_upper_bound):
        self._obj.theta_max_upper_bound = theta_max_upper_bound

    @property
    def agent_buffer(self) -> float:
        return self._obj.agent_buffer

    @agent_buffer.setter
    def agent_buffer(self, agent_buffer):
        self._obj.agent_buffer = agent_buffer
