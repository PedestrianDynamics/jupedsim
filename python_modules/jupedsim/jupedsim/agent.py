# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps
from jupedsim.models.collision_free_speed import CollisionFreeSpeedModelState
from jupedsim.models.collision_free_speed_v2 import (
    CollisionFreeSpeedModelV2State,
)
from jupedsim.models.anticipation_velocity_model import (
    AnticipationVelocityModelState,
)
from jupedsim.models.generalized_centrifugal_force import (
    GeneralizedCentrifugalForceModelState,
)
from jupedsim.models.social_force import SocialForceModelState


class Agent:
    """Represents an Agent in the simulation.

    Agent objects are always retrieved from the simulation and never created directly.

    Agents can be accessed with:

    .. code:: python

        # a specific agent
        sim.agent(id)

        # all agents as iterator
        sim.agents()

        # agents in a specific distance to a point as iterator
        sim.agents_in_range(position, distance)

        # agents in a polygon as iterator
        sim.agents_in_polygon(polygon)

    .. note ::

        You need to be aware that currently there are no checks done when setting
        properties on an Agent instance. For example it is possible to set an Agent position
        outside the walkable area of the Simulation resulting in a crash.
    """

    def __init__(self, backing) -> None:
        """Do not use.

        Retrieve agents from the simulation.
        """
        self._obj = backing

    @property
    def id(self) -> int:
        """Numeric id of the agent in this simulation."""
        return self._obj.id

    @property
    def journey_id(self) -> int:
        """Id of the :class:`~jupedsim.journey.JourneyDescription` the agent is currently following."""
        return self._obj.journey_id

    @property
    def stage_id(self) -> int:
        """Id of the :class:`Stage` the Agent is currently targeting."""
        return self._obj.stage_id

    @property
    def position(self) -> tuple[float, float]:
        """Position of the agent."""
        return self._obj.position

    @property
    def orientation(self) -> tuple[float, float]:
        """Orientation of the agent."""
        return self._obj.orientation

    @property
    def target(self) -> tuple[float, float]:
        """Current target of the agent.

        Can be used to directly steer an agent towards the given coordinate.
        This will bypass the strategical and tactical level, but the operational level
        will still be active.

        .. important::

            If the agent is not in a Journey with a DirectSteering stage, any change will be
            ignored.

        .. important::

            When setting the target, the given coordinates must lie within the walkable area.
            Otherwise, an error will be thrown at the next iteration call.

        Returns:
            Current target of the agent.
        """
        return self._obj.target

    @target.setter
    def target(self, target: tuple[float, float]):
        self._obj.target = target

    @property
    def model(
        self,
    ) -> (
        GeneralizedCentrifugalForceModelState
        | CollisionFreeSpeedModelState
        | CollisionFreeSpeedModelV2State
        | AnticipationVelocityModelState
        | SocialForceModelState
    ):
        """Access model specific state of this agent."""
        model = self._obj.model
        if isinstance(model, py_jps.GeneralizedCentrifugalForceModelState):
            return GeneralizedCentrifugalForceModelState(model)
        elif isinstance(model, py_jps.CollisionFreeSpeedModelState):
            return CollisionFreeSpeedModelState(model)
        elif isinstance(model, py_jps.CollisionFreeSpeedModelV2State):
            return CollisionFreeSpeedModelV2State(model)
        elif isinstance(model, py_jps.AnticipationVelocityModelState):
            return AnticipationVelocityModelState(model)
        elif isinstance(model, py_jps.SocialForceModelState):
            return SocialForceModelState(model)
        else:
            raise Exception("Internal error")
