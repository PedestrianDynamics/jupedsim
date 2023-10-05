# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
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
    distribute_till_full,
)
from jupedsim.geometry import Geometry
from jupedsim.geometry_utils import (
    _geometry_from_coordinates,
    _geometry_from_shapely,
    _geometry_from_wkt,
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
from jupedsim.models import (
    GeneralizedCentrifugalForceModelAgentParameters,
    GeneralizedCentrifugalForceModelParameters,
    GeneralizedCentrifugalForceModelState,
    VelocityModelAgentParameters,
    VelocityModelParameters,
    VelocityModelState,
)
from jupedsim.recording import Recording, RecordingAgent, RecordingFrame
from jupedsim.routing import RoutingEngine
from jupedsim.serialization import TrajectoryWriter
from jupedsim.simulation import Simulation
from jupedsim.sqlite_serialization import SqliteTrajectoryWriter
from jupedsim.stages import (
    ExitProxy,
    NotifiableQueueProxy,
    WaitingSetProxy,
    WaitingSetState,
    WaypointProxy,
)
from jupedsim.tracing import Trace

__version__ = get_build_info().library_version
__commit__ = get_build_info().git_commit_hash
__compiler__ = (
    f"{get_build_info().compiler} ({get_build_info().compiler_version})"
)

__all__ = [
    "Agent",
    "AgentNumberError",
    "BuildInfo",
    "ExitProxy",
    "GeneralizedCentrifugalForceModelAgentParameters",
    "GeneralizedCentrifugalForceModelParameters",
    "GeneralizedCentrifugalForceModelState",
    "Geometry",
    "IncorrectParameterError",
    "JourneyDescription",
    "NegativeValueError",
    "NotifiableQueueProxy",
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
    "VelocityModelAgentParameters",
    "VelocityModelParameters",
    "VelocityModelState",
    "WaitingSetProxy",
    "WaitingSetState",
    "WaypointProxy",
    "__commit__",
    "__compiler__",
    "__version__",
    "distribute_by_density",
    "distribute_by_number",
    "distribute_by_percentage",
    "distribute_in_circles_by_density",
    "distribute_in_circles_by_number",
    "distribute_till_full",
    "_geometry_from_coordinates",
    "_geometry_from_shapely",
    "_geometry_from_wkt",
    "get_build_info",
    "set_debug_callback",
    "set_error_callback",
    "set_info_callback",
    "set_warning_callback",
]
