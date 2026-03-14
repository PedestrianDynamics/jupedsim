# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass


@dataclass(kw_only=True)
class WarpDriverModel:
    r"""Parameters for WarpDriver Model.

    Based on Wolinski, Lin, and Pettré (2016) — probabilistic collision
    avoidance using warped intrinsic fields.

    Attributes:
        time_horizon: look-ahead time for collision prediction [s].
        step_size: gradient descent step size (alpha).
        sigma: Gaussian spread of the intrinsic field.
        time_uncertainty: time uncertainty parameter (lambda).
        velocity_uncertainty: velocity uncertainty parameter (mu).
        num_samples: number of trajectory sample points.
        jam_speed_threshold: speed below which an agent is considered jammed [m/s].
        jam_step_count: consecutive jammed steps before entering chill mode.
    """

    time_horizon: float = 2.0
    step_size: float = 0.5
    sigma: float = 0.3
    time_uncertainty: float = 0.5
    velocity_uncertainty: float = 0.2
    num_samples: int = 20
    jam_speed_threshold: float = 0.1
    jam_step_count: int = 10


@dataclass(kw_only=True)
class WarpDriverModelAgentParameters:
    """Parameters required to create an Agent in the WarpDriver Model.

    Attributes:
        position: Position of the agent.
        orientation: Orientation of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
        desired_speed: desired speed of the agent [m/s].
        radius: radius of the agent [m].
    """

    position: tuple[float, float] = (0.0, 0.0)
    orientation: tuple[float, float] = (0.0, 0.0)
    journey_id: int = -1
    stage_id: int = -1
    desired_speed: float = 1.2
    radius: float = 0.15


class WarpDriverModelState:
    """State of an agent using the WarpDriver Model."""

    def __init__(self, backing) -> None:
        self._obj = backing

    @property
    def radius(self) -> float:
        """Radius of this agent [m]."""
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius

    @property
    def desired_speed(self) -> float:
        """Desired speed of this agent [m/s]."""
        return self._obj.desired_speed

    @desired_speed.setter
    def desired_speed(self, desired_speed):
        self._obj.desired_speed = desired_speed

    @property
    def jam_counter(self) -> int:
        """Current jam counter (read-only)."""
        return self._obj.jam_counter
