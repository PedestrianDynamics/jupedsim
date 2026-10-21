# SPDX-License-Identifier: LGPL-3.0-or-later

from jupedsim.agent import Agent
from jupedsim.convenience import circle_to_polygon
from jupedsim.custom_model import CustomOperationalModel
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
    distribute_until_filled,
)
from jupedsim.hdf5_serialization import (
    Hdf5TrajectoryLoader,
    Hdf5TrajectoryWriter,
)
from jupedsim.internal.tracing import Timer, trace_event
from jupedsim.journey import JourneyDescription
from jupedsim.pyjupedsim import (
    AgentStep,
    AgentView,
    AnticipationVelocityModel,
    AnticipationVelocityModelState,
    BuildInfo,
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3,
    CollisionFreeSpeedModelV3State,
    DirectSteeringStage,
    ExitStage,
    FloorfieldOptions,
    GeneralizedCentrifugalForceModel,
    GeneralizedCentrifugalForceModelState,
    Geometry,
    Location,
    NeighborView,
    SimulationError,
    SocialForceModel,
    SocialForceModelState,
    SurfaceMeshShortestPathRoutingEngine,
    Transition,
    TravelTimeGrid,
    WalkableSurface,
    WallView,
    WarpDriverModel,
    WarpDriverModelState,
    WaypointStage,
    disable_tracing,
    dump_traces,
    enable_tracing,
    end_trace_event,
    get_build_info,
    is_tracing_enabled,
    make_portal_boundary_index,
    set_debug_callback,
    set_error_callback,
    set_info_callback,
    set_warning_callback,
    start_trace_event,
)
from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation

__version__ = get_build_info().library_version
"""
The version of this module.
"""

__commit__ = get_build_info().git_commit_hash
"""
Commit id that was used to build this module.
"""

__compiler__ = (
    f"{get_build_info().compiler} ({get_build_info().compiler_version})"
)
"""
Id of the compiler used to build the native portion of this module.
"""


__all__ = [
    "Agent",
    "AgentNumberError",
    "AgentStep",
    "AgentView",
    "AnticipationVelocityModel",
    "AnticipationVelocityModelState",
    "BuildInfo",
    "CollisionFreeSpeedModel",
    "CollisionFreeSpeedModelState",
    "CollisionFreeSpeedModelV2",
    "CollisionFreeSpeedModelV2State",
    "CollisionFreeSpeedModelV3",
    "CollisionFreeSpeedModelV3State",
    "CustomOperationalModel",
    "DirectSteeringStage",
    "ExitStage",
    "FloorfieldOptions",
    "GeneralizedCentrifugalForceModel",
    "GeneralizedCentrifugalForceModelState",
    "Geometry",
    "Hdf5TrajectoryLoader",
    "Hdf5TrajectoryWriter",
    "IncorrectParameterError",
    "JourneyDescription",
    "Location",
    "NegativeValueError",
    "NeighborView",
    "OverlappingCirclesError",
    "Simulation",
    "SimulationError",
    "SocialForceModel",
    "SocialForceModelState",
    "SurfaceMeshShortestPathRoutingEngine",
    "Timer",
    "TrajectoryWriter",
    "Transition",
    "TravelTimeGrid",
    "WalkableSurface",
    "WallView",
    "WarpDriverModel",
    "WarpDriverModelState",
    "WaypointStage",
    "__commit__",
    "__compiler__",
    "__version__",
    "circle_to_polygon",
    "disable_tracing",
    "distribute_by_density",
    "distribute_by_number",
    "distribute_by_percentage",
    "distribute_in_circles_by_density",
    "distribute_in_circles_by_number",
    "distribute_until_filled",
    "dump_traces",
    "enable_tracing",
    "end_trace_event",
    "get_build_info",
    "is_tracing_enabled",
    "make_portal_boundary_index",
    "set_debug_callback",
    "set_error_callback",
    "set_info_callback",
    "set_warning_callback",
    "start_trace_event",
    "trace_event",
]
