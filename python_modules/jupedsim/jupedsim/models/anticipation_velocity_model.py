# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass
from random import randint

import jupedsim.native as py_jps


@dataclass(kw_only=True)
class AnticipationVelocityModel:
    """Anticipation Velocity Model (AVM).

    The AVM incorporates pedestrian anticipation, divided into three phases:
    1. Perception of the current situation.
    2. Prediction of future situations.
    3. Strategy selection leading to action.

    This model quantitatively reproduces bidirectional pedestrian flow by accounting for:
    - Anticipation of changes in neighboring pedestrians' positions.
    - The strategy of following others' movement. The AVM is a model that takes into consideration
      the anticipation of pedestrians. For this, the process of anticipation is divided into three parts:
      - perception of the actual situation,
      - prediction of a future situation and
      - selection of a strategy leading to action.


    A general description of the AVM can be found in the originating publication
    https://doi.org/10.1016/j.trc.2021.103464

    Attributes:
        rng_seed: seed value of internally used rng. If not explicitly set this
            value will be choosen randomly.
    """

    rng_seed: int = randint(0, 2**64 - 1)


@dataclass(kw_only=True)
class AnticipationVelocityModelAgentParameters:
    """
    Agent parameters for Anticipation Velocity Model (AVM).

    See publication for more details about this model
    https://doi.org/10.1016/j.trc.2021.103464

    .. note::

        Instances of this type are copied when creating the agent, you can safely
        create one instance of this type and modify it between calls to `add_agent`

        E.g.:

        .. code:: python

            positions = [...] # List of initial agent positions
            params = AnticipationVelocityModelAgentParameters(v0=0.9) # all agents are slower
            for p in positions:
                params.position = p
                sim.add_agent(params)

    Attributes:
        position: Position of the agent.
        time_gap: Time constant that describe how fast pedestrian close gaps.
        v0: Maximum speed of the agent.
        radius: Radius of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
        strength_neighbor_repulsion: Strength of the repulsion from neighbors
        range_neighbor_repulsion: Range of the repulsion from neighbors
        wall_buffer_distance: Buffer distance of agents to the walls.
        anticipation_time: Anticipation time of an agent.
        reaction_time: reaction time of an agent to change its direction.
    """

    position: tuple[float, float] = (0.0, 0.0)
    time_gap: float = 1.06
    v0: float = 1.2
    radius: float = 0.2
    journey_id: int = 0
    stage_id: int = 0
    strength_neighbor_repulsion: float = 8.0
    range_neighbor_repulsion: float = 0.1
    wall_buffer_distance: float = 0.1
    anticipation_time: float = 1.0
    reaction_time: float = 0.3

    def as_native(
        self,
    ) -> py_jps.AnticipationVelocityModelAgentParameters:
        return py_jps.AnticipationVelocityModelAgentParameters(
            position=self.position,
            time_gap=self.time_gap,
            v0=self.v0,
            radius=self.radius,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
            strength_neighbor_repulsion=self.strength_neighbor_repulsion,
            range_neighbor_repulsion=self.range_neighbor_repulsion,
            wall_buffer_distance=self.wall_buffer_distance,
            anticipation_time=self.anticipation_time,
            reaction_time=self.reaction_time,
        )


class AnticipationVelocityModelState:
    def __init__(self, backing):
        self._obj = backing

    @property
    def time_gap(self) -> float:
        return self._obj.time_gap

    @time_gap.setter
    def time_gap(self, time_gap):
        self._obj.time_gap = time_gap

    @property
    def v0(self) -> float:
        """Maximum speed of this agent."""
        return self._obj.v0

    @v0.setter
    def v0(self, v0):
        self._obj.v0 = v0

    @property
    def radius(self) -> float:
        """Radius of this agent."""
        return self._obj.radius

    @radius.setter
    def radius(self, radius):
        self._obj.radius = radius

    @property
    def strength_neighbor_repulsion(self) -> float:
        """Strength of the repulsion from neighbors of this agent."""
        return self._obj.strength_neighbor_repulsion

    @strength_neighbor_repulsion.setter
    def strength_neighbor_repulsion(self, strength_neighbor_repulsion):
        self._obj.strength_neighbor_repulsion = strength_neighbor_repulsion

    @property
    def range_neighbor_repulsion(self) -> float:
        """Range of the repulsion from neighbors of this agent."""
        return self._obj.range_neighbor_repulsion

    @range_neighbor_repulsion.setter
    def range_neighbor_repulsion(self, range_neighbor_repulsion):
        self._obj.range_neighbor_repulsion = range_neighbor_repulsion

    @property
    def wall_buffer_distance(self):
        """Wall buffer distance of agent to walls."""
        return self._obj.wall_buffer_distance

    @wall_buffer_distance.setter
    def wall_buffer_distance(self, wall_buffer_distance):
        self._obj.wall_buffer_distance = wall_buffer_distance

    @property
    def anticipation_time(self) -> float:
        """Anticipation time of this agent."""
        return self._obj.anticipation_time

    @anticipation_time.setter
    def anticipation_time(self, anticipation_time):
        self._obj.anticipation_time = anticipation_time

    @property
    def reaction_time(self) -> float:
        """Reaction time of this agent."""
        return self._obj.reaction_time

    @reaction_time.setter
    def reaction_time(self, reaction_time):
        self._obj.reaction_time = reaction_time
