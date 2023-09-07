# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

import py_jupedsim as py_jps

from jupedsim.native.geometry import Geometry
from jupedsim.native.journey import JourneyDescription
from jupedsim.native.models import (
    GCFMModelAgentParameters,
    VelocityModelAgentParameters,
)
from jupedsim.native.stages import (
    ExitProxy,
    NotifiableQueueProxy,
    WaitingSetProxy,
    WaypointProxy,
)
from jupedsim.native.tracing import Trace


class Simulation:
    def __init__(self, model, geometry: Geometry, dt: float) -> None:
        self._sim = py_jps.Simulation(
            model=model, geometry=geometry._obj, dt=dt
        )

    def add_waypoint_stage(
        self, position: tuple[float, float], distance
    ) -> int:
        return self._sim.add_waypoint_stage(position, distance)

    def add_queue_stage(self, positions: list[tuple[float, float]]) -> int:
        return self._sim.add_queue_stage(positions)

    def add_waiting_set_stage(
        self, positions: list[tuple[float, float]]
    ) -> int:
        return self._sim.add_waiting_set_stage(positions)

    def add_exit_stage(self, polygon: list[tuple[float, float]]) -> int:
        return self._sim.add_exit_stage(polygon)

    def add_journey(self, journey: JourneyDescription) -> int:
        return self._sim.add_journey(journey._obj)

    def add_agent(
        self,
        parameters: GCFMModelAgentParameters | VelocityModelAgentParameters,
    ) -> int:
        return self._sim.add_agent(parameters._obj)

    def remove_agent(self, agent_id: int) -> bool:
        return self._sim.remove_agent(agent_id)

    def removed_agents(self) -> list[int]:
        return self._sim.removed_agents()

    def iterate(self, count: int = 1) -> None:
        self._sim.iterate(count)

    def switch_agent_profile(self, agent_id: int, profile_id: int) -> None:
        self._sim.switch_agent_profile(
            agent_id=agent_id, profile_id=profile_id
        )

    def switch_agent_journey(
        self, agent_id: int, journey_id: int, stage_index: int
    ) -> None:
        self._sim.switch_agent_journey(
            agent_id=agent_id, journey_id=journey_id, stage_index=stage_index
        )

    def agent_count(self) -> int:
        return self._sim.agent_count()

    def elapsed_time(self) -> float:
        return self._sim.elapsed_time()

    def delta_time(self) -> float:
        return self._sim.delta_time()

    def iteration_count(self) -> int:
        return self._sim.iteration_count()

    def agents(self):
        return self._sim.agents()

    def agents_in_range(self, pos: tuple[float, float], distance: float):
        return self._sim.agents_in_range(pos, distance)

    def agents_in_polygon(self, poly: list[tuple[float, float]]):
        return self._sim.agents_in_polygon(poly)

    def get_stage_proxy(self, stage_id: int):
        stage = self._sim.get_stage_proxy(stage_id)
        match stage:
            case py_jps.WaypointProxy():
                return WaypointProxy(stage)
            case py_jps.ExitProxy():
                return ExitProxy(stage)
            case py_jps.NotifiableQueueProxy():
                return NotifiableQueueProxy(stage)
            case py_jps.WaitingSetProxy():
                return WaitingSetProxy(stage)
            case _:
                raise Exception(
                    f"Internal error, unexpected type: {type(stage)}"
                )

    def set_tracing(self, status: bool) -> None:
        self._sim.set_tracing(status)

    def get_last_trace(self) -> Trace:
        return self._sim.get_last_trace()
