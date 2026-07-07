# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps
from jupedsim.agent import Agent
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
from jupedsim.geometry import Geometry
from jupedsim.internal.tracing import (
    Timer,
    disable_tracing,
    dump_traces,
    enable_tracing,
    end_trace_event,
    start_trace_event,
    trace_event,
)
from jupedsim.journey import JourneyDescription, Transition
from jupedsim.library import (
    BuildInfo,
    get_build_info,
    set_debug_callback,
    set_error_callback,
    set_info_callback,
    set_warning_callback,
)
from jupedsim.linesegment import LineSegment
from jupedsim.models.anticipation_velocity_model import (
    AnticipationVelocityModel,
    AnticipationVelocityModelState,
)
from jupedsim.models.collision_free_speed import (
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelState,
)
from jupedsim.models.collision_free_speed_v2 import (
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV2State,
)
from jupedsim.models.collision_free_speed_v3 import (
    CollisionFreeSpeedModelV3,
    CollisionFreeSpeedModelV3State,
)
from jupedsim.models.custom_model import (
    CustomModelAgentState,
    CustomOperationalModel,
)
from jupedsim.models.generalized_centrifugal_force import (
    GeneralizedCentrifugalForceModel,
    GeneralizedCentrifugalForceModelState,
)
from jupedsim.models.social_force import (
    SocialForceModel,
    SocialForceModelState,
)
from jupedsim.models.warp_driver import (
    WarpDriverModel,
    WarpDriverModelState,
)
from jupedsim.neighborhood import NeighborhoodSearch
from jupedsim.recording import Recording, RecordingAgent, RecordingFrame
from jupedsim.routing import RoutingEngine
from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation
from jupedsim.sqlite_serialization import SqliteTrajectoryWriter

try:
    from jupedsim.hdf5_serialization import Hdf5TrajectoryWriter
except ImportError:  # h5py not installed; HDF5 writer remains unavailable.
    Hdf5TrajectoryWriter = None  # type: ignore[assignment, misc]
from jupedsim.stages import (
    ExitStage,
    NotifiableQueueStage,
    WaitingSetStage,
    WaitingSetState,
    WaypointStage,
)

SimulationError = py_jps.SimulationError
"""Raised for simulation errors, e.g. when accessing an agent handle whose
agent no longer exists or when calling mutating simulation methods from a
custom-model callback."""

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
    "AnticipationVelocityModel",
    "AnticipationVelocityModelState",
    "BuildInfo",
    "CollisionFreeSpeedModel",
    "CollisionFreeSpeedModelState",
    "CollisionFreeSpeedModelV2",
    "CollisionFreeSpeedModelV2State",
    "CollisionFreeSpeedModelV3",
    "CollisionFreeSpeedModelV3State",
    "CustomModelAgentState",
    "CustomOperationalModel",
    "ExitStage",
    "GeneralizedCentrifugalForceModel",
    "GeneralizedCentrifugalForceModelState",
    "Geometry",
    "Hdf5TrajectoryWriter",
    "IncorrectParameterError",
    "JourneyDescription",
    "LineSegment",
    "NegativeValueError",
    "NeighborhoodSearch",
    "NotifiableQueueStage",
    "OverlappingCirclesError",
    "Recording",
    "RecordingAgent",
    "RecordingFrame",
    "RoutingEngine",
    "Simulation",
    "SimulationError",
    "SocialForceModel",
    "SocialForceModelState",
    "SqliteTrajectoryWriter",
    "Timer",
    "TrajectoryWriter",
    "Transition",
    "WaitingSetStage",
    "WaitingSetState",
    "WarpDriverModel",
    "WarpDriverModelState",
    "WaypointStage",
    "__commit__",
    "__compiler__",
    "__version__",
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
    "set_debug_callback",
    "set_error_callback",
    "set_info_callback",
    "set_warning_callback",
    "start_trace_event",
    "trace_event",
]
