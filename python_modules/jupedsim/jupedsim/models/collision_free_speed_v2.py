# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from dataclasses import dataclass

import jupedsim.native as py_jps


@dataclass(kw_only=True)
class CollisionFreeSpeedModelv2:
    """Collision Free Speed Model V2

    This is a variation of the Collision Free Speed Model where geometry and neighbor repulsion are individual
    agent parameters instead of global parameters.

    A general description of the Collision Free Speed Model can be found in the originating publication
    https://arxiv.org/abs/1512.05597

    A more detailed description can be found at https://pedestriandynamics.org/models/collision_free_speed_model/
    """

    pass


@dataclass(kw_only=True)
class CollisionFreeSpeedModelv2AgentParameters:
    """
    Agent parameters for Collision Free Speed Model v2.

    See the scientific publication for more details about this model
    https://arxiv.org/abs/1512.05597

    .. note::

        Instances of this type are copied when creating the agent, you can safely
        create one instance of this type and modify it between calls to `add_agent`

        E.g.:

        .. code:: python

            positions = [...] # List of initial agent positions
            params = CollisionFreeSpeedModelv2AgentParameters(v0=0.9) # all agents are slower
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
        strength_geometry_repulsion: Strength of the repulsion from geometry boundaries
        range_geometry_repulsion: Range of the repulsion from geometry boundaries
    """

    position: tuple[float, float] = (0.0, 0.0)
    time_gap: float = 1.0
    v0: float = 1.2
    radius: float = 0.2
    journey_id: int = 0
    stage_id: int = 0
    strength_neighbor_repulsion: float = 8.0
    range_neighbor_repulsion: float = 0.1
    strength_geometry_repulsion: float = 5.0
    range_geometry_repulsion: float = 0.02

    def as_native(
        self,
    ) -> py_jps.CollisionFreeSpeedModelv2AgentParameters:
        return py_jps.CollisionFreeSpeedModelv2AgentParameters(
            position=self.position,
            time_gap=self.time_gap,
            v0=self.v0,
            radius=self.radius,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
            strength_neighbor_repulsion=self.strength_neighbor_repulsion,
            range_neighbor_repulsion=self.range_neighbor_repulsion,
            strength_geometry_repulsion=self.strength_geometry_repulsion,
            range_geometry_repulsion=self.range_geometry_repulsion,
        )


class CollisionFreeSpeedModelv2State:
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
    def strength_geometry_repulsion(self) -> float:
        """Strength of the repulsion from geometry boundaries of this agent."""
        return self._obj.strength_geometry_repulsion

    @strength_geometry_repulsion.setter
    def strength_geometry_repulsion(self, strength_geometry_repulsion):
        self._obj.strength_geometry_repulsion = strength_geometry_repulsion

    @property
    def range_geometry_repulsion(self) -> float:
        """Range of the repulsion from geometry boundaries of this agent."""
        return self._obj.range_geometry_repulsion

    @range_geometry_repulsion.setter
    def range_geometry_repulsion(self, range_geometry_repulsion):
        self._obj.range_geometry_repulsion = range_geometry_repulsion
