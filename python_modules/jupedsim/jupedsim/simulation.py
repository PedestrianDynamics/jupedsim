# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Any

import shapely

import jupedsim.native as py_jps
from jupedsim.geometry import Geometry
from jupedsim.geometry_utils import build_geometry
from jupedsim.journey import JourneyDescription
from jupedsim.models import (
    GeneralizedCentrifugalForceModelAgentParameters,
    GeneralizedCentrifugalForceModelParameters,
    VelocityModelAgentParameters,
    VelocityModelParameters,
)
from jupedsim.serialization import TrajectoryWriter
from jupedsim.stages import (
    ExitProxy,
    NotifiableQueueProxy,
    WaitingSetProxy,
    WaypointProxy,
)
from jupedsim.tracing import Trace


class Simulation:
    def __init__(
        self,
        *,
        model: VelocityModelParameters
        | GeneralizedCentrifugalForceModelParameters,
        geometry: str
        | shapely.GeometryCollection
        | shapely.Polygon
        | shapely.MultiPolygon
        | shapely.MultiPoint
        | list[tuple[float, float]],
        dt: float = 0.01,
        trajectory_writer: TrajectoryWriter | None = None,
        **kwargs: Any,
    ) -> None:
        if isinstance(model, VelocityModelParameters):
            model_builder = py_jps.VelocityModelBuilder(
                a_ped=model.a_ped,
                d_ped=model.d_ped,
                a_wall=model.a_wall,
                d_wall=model.d_wall,
            )
            py_jps_model = model_builder.build()
        elif isinstance(model, GeneralizedCentrifugalForceModelParameters):
            model_builder = py_jps.GCFMModelBuilder(
                nu_ped=model.nu_ped,
                nu_wall=model.nu_wall,
                dist_eff_ped=model.dist_eff_ped,
                dist_eff_wall=model.dist_eff_wall,
                intp_width_ped=model.intp_width_ped,
                intp_width_wall=model.intp_width_wall,
                maxf_ped=model.maxf_ped,
                maxf_wall=model.maxf_wall,
            )
            py_jps_model = model_builder.build()
            pass
        else:
            raise Exception("Unknown model type supplied")
        self._writer = trajectory_writer
        self._obj = py_jps.Simulation(
            model=py_jps_model, geometry=build_geometry(geometry)._obj, dt=dt
        )

    def add_waypoint_stage(
        self, position: tuple[float, float], distance
    ) -> int:
        return self._obj.add_waypoint_stage(position, distance)

    def add_queue_stage(self, positions: list[tuple[float, float]]) -> int:
        return self._obj.add_queue_stage(positions)

    def add_waiting_set_stage(
        self, positions: list[tuple[float, float]]
    ) -> int:
        return self._obj.add_waiting_set_stage(positions)

    def add_exit_stage(self, polygon: list[tuple[float, float]]) -> int:
        return self._obj.add_exit_stage(polygon)

    def add_journey(self, journey: JourneyDescription) -> int:
        return self._obj.add_journey(journey._obj)

    def add_agent(
        self,
        parameters: GeneralizedCentrifugalForceModelAgentParameters
        | VelocityModelAgentParameters,
    ) -> int:
        return self._obj.add_agent(parameters._obj)

    def remove_agent(self, agent_id: int) -> bool:
        return self._obj.remove_agent(agent_id)

    def removed_agents(self) -> list[int]:
        return self._obj.removed_agents()

    def iterate(self, count: int = 1) -> None:
        if self._writer and self.iteration_count() == 0:
            self._writer.begin_writing(self)
            self._writer.write_iteration_state(self)

        for _ in range(0, count):
            self._obj.iterate()
            if self._writer:
                self._writer.write_iteration_state(self)

    def switch_agent_journey(
        self, agent_id: int, journey_id: int, stage_id: int
    ) -> None:
        self._obj.switch_agent_journey(
            agent_id=agent_id, journey_id=journey_id, stage_id=stage_id
        )

    def agent_count(self) -> int:
        return self._obj.agent_count()

    def elapsed_time(self) -> float:
        return self._obj.elapsed_time()

    def delta_time(self) -> float:
        return self._obj.delta_time()

    def iteration_count(self) -> int:
        return self._obj.iteration_count()

    def agents(self):
        return self._obj.agents()

    def agent(self, agent_id):
        return self._obj.agent(agent_id)

    def agents_in_range(self, pos: tuple[float, float], distance: float):
        return self._obj.agents_in_range(pos, distance)

    def agents_in_polygon(self, poly: list[tuple[float, float]]):
        return self._obj.agents_in_polygon(poly)

    def get_stage_proxy(self, stage_id: int):
        stage = self._obj.get_stage_proxy(stage_id)
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
        self._obj.set_tracing(status)

    def get_last_trace(self) -> Trace:
        return self._obj.get_last_trace()

    def get_geometry(self) -> Geometry:
        return Geometry(self._obj.get_geometry())
