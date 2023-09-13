# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from jupedsim.distributions import (
    AgentNumberError,
    IncorrectParameterError,
    NegativeValueError,
    OverlappingCirclesError,
    distribute_by_density,
    distribute_by_number,
    distribute_by_percentage,
    distribute_in_circles_by_density,
    distribute_in_circles_by_number,
    distribute_till_full,
)
from jupedsim.native.agent import Agent
from jupedsim.native.geometry import Geometry, GeometryBuilder
from jupedsim.native.journey import JourneyDescription, Transition
from jupedsim.native.library import (
    BuildInfo,
    get_build_info,
    set_debug_callback,
    set_error_callback,
    set_info_callback,
    set_warning_callback,
)
from jupedsim.native.models import (
    GCFMModelAgentParameters,
    GCFMModelBuilder,
    GeneralizedCentrifugalForceModelState,
    VelocityModelAgentParameters,
    VelocityModelBuilder,
    VelocityModelState,
)
from jupedsim.native.routing import RoutingEngine
from jupedsim.native.simulation import Simulation
from jupedsim.native.stages import (
    ExitProxy,
    NotifiableQueueProxy,
    WaitingSetProxy,
    WaitingSetState,
    WaypointProxy,
)
from jupedsim.native.tracing import Trace
from jupedsim.recording import Recording, RecordingAgent, RecordingFrame
from jupedsim.trajectory_writer_sqlite import SqliteTrajectoryWriter
from jupedsim.util import (
    GeometryError,
    geometry_from_coordinates,
    geometry_from_shapely,
    geometry_from_wkt,
)

__version__ = get_build_info().library_version
__commit__ = get_build_info().git_commit_hash
__compiler__ = (
    f"{get_build_info().compiler} ({get_build_info().compiler_version})"
)

__all__ = [
    "AgentNumberError",
    "IncorrectParameterError",
    "NegativeValueError",
    "OverlappingCirclesError",
    "distribute_by_density",
    "distribute_by_number",
    "distribute_by_percentage",
    "distribute_in_circles_by_density",
    "distribute_in_circles_by_number",
    "distribute_till_full",
    "Agent",
    "Geometry",
    "GeometryBuilder",
    "JourneyDescription",
    "Transition",
    "BuildInfo",
    "get_build_info",
    "set_debug_callback",
    "set_error_callback",
    "set_info_callback",
    "set_warning_callback",
    "GCFMModelAgentParameters",
    "GCFMModelBuilder",
    "GeneralizedCentrifugalForceModelState",
    "VelocityModelAgentParameters",
    "VelocityModelBuilder",
    "VelocityModelState",
    "RoutingEngine",
    "Simulation",
    "ExitProxy",
    "NotifiableQueueProxy",
    "WaitingSetProxy",
    "WaitingSetState",
    "WaypointProxy",
    "Trace",
    "Recording",
    "RecordingAgent",
    "RecordingFrame",
    "SqliteTrajectoryWriter",
    "geometry_from_wkt",
    "geometry_from_shapely",
    "geometry_from_coordinates",
    "GeometryError",
    "build_jps_geometry",
    "__version__",
    "__commit__",
    "__compiler__",
]
