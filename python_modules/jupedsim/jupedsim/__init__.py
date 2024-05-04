# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

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
from jupedsim.internal.tracing import Trace
from jupedsim.journey import JourneyDescription, Transition
from jupedsim.library import (
    BuildInfo,
    get_build_info,
    set_debug_callback,
    set_error_callback,
    set_info_callback,
    set_warning_callback,
)
from jupedsim.models.collision_free_speed import (
    CollisionFreeSpeedModel,
    CollisionFreeSpeedModelAgentParameters,
    CollisionFreeSpeedModelState,
)
from jupedsim.models.collision_free_speed_v2 import (
    CollisionFreeSpeedModelV2,
    CollisionFreeSpeedModelV2AgentParameters,
    CollisionFreeSpeedModelV2State,
)
from jupedsim.models.generalized_centrifugal_force import (
    GeneralizedCentrifugalForceModel,
    GeneralizedCentrifugalForceModelAgentParameters,
    GeneralizedCentrifugalForceModelState,
)
from jupedsim.models.social_force import (
    SocialForceModel,
    SocialForceModelAgentParameters,
    SocialForceModelState,
)
from jupedsim.recording import Recording, RecordingAgent, RecordingFrame
from jupedsim.routing import RoutingEngine
from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation
from jupedsim.sqlite_serialization import SqliteTrajectoryWriter
from jupedsim.stages import (
    ExitStage,
    NotifiableQueueStage,
    WaitingSetStage,
    WaitingSetState,
    WaypointStage,
)

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
    "BuildInfo",
    "ExitStage",
    "GeneralizedCentrifugalForceModelAgentParameters",
    "GeneralizedCentrifugalForceModel",
    "GeneralizedCentrifugalForceModelState",
    "Geometry",
    "IncorrectParameterError",
    "JourneyDescription",
    "NegativeValueError",
    "NotifiableQueueStage",
    "OverlappingCirclesError",
    "Recording",
    "RecordingAgent",
    "RecordingFrame",
    "RoutingEngine",
    "Simulation",
    "SqliteTrajectoryWriter",
    "Trace",
    "TrajectoryWriter",
    "Transition",
    "CollisionFreeSpeedModelAgentParameters",
    "CollisionFreeSpeedModel",
    "CollisionFreeSpeedModelState",
    "CollisionFreeSpeedModelV2AgentParameters",
    "CollisionFreeSpeedModelV2",
    "CollisionFreeSpeedModelV2State",
    "SocialForceModelAgentParameters",
    "SocialForceModel",
    "SocialForceModelState",
    "WaitingSetStage",
    "WaitingSetState",
    "WaypointStage",
    "__commit__",
    "__compiler__",
    "__version__",
    "distribute_by_density",
    "distribute_by_number",
    "distribute_by_percentage",
    "distribute_in_circles_by_density",
    "distribute_in_circles_by_number",
    "distribute_until_filled",
    "get_build_info",
    "set_debug_callback",
    "set_error_callback",
    "set_info_callback",
    "set_warning_callback",
]
