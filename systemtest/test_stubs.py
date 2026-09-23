# SPDX-License-Identifier: LGPL-3.0-or-later
import re

from mypy import stubtest

MODULE = "jupedsim.py_jupedsim"

# Every bound class has metaclass pybind11_builtins.pybind11_type, which the
# stubs do not declare.
CLASSES = [
    "Agent",
    "AgentStep",
    "AgentView",
    "AnticipationVelocityModel",
    "AnticipationVelocityModelState",
    "BoundaryIndex",
    "CollisionFreeSpeedModel",
    "CollisionFreeSpeedModelState",
    "CollisionFreeSpeedModelV2",
    "CollisionFreeSpeedModelV2State",
    "CollisionFreeSpeedModelV3",
    "CollisionFreeSpeedModelV3State",
    "DirectSteeringProxy",
    "ExitProxy",
    "FixedTransitionDescription",
    "GeneralizedCentrifugalForceModel",
    "GeneralizedCentrifugalForceModelState",
    "Geometry",
    "GeometryBuilder",
    "LeastTargetedTransitionDescription",
    "Location",
    "NeighborView",
    "NonTransitionDescription",
    "NotifiableQueueProxy",
    "OperationalModel",
    "Polygon2D",
    "Profiler",
    "RoundRobinTransitionDescription",
    "RoutingEngine",
    "Simulation",
    "SocialForceModel",
    "SocialForceModelState",
    "SurfaceMeshShortestPathRoutingEngine",
    "WaitingSetProxy",
    "WaitingSetState",
    "WalkableSurface",
    "WallView",
    "WarpDriverModel",
    "WarpDriverModelState",
    "WaypointProxy",
    "_CustomModelState",
    "_NeighborStateMapper",
    "_PythonModel",
]

# Classes without a bound constructor inherit pybind11_object.__init__(*args,
# **kwargs), which only raises; stubgen emits no __init__ for them.
CLASSES_WITHOUT_CONSTRUCTOR = [
    "Agent",
    "AgentStep",
    "AgentView",
    "BoundaryIndex",
    "DirectSteeringProxy",
    "ExitProxy",
    "FixedTransitionDescription",
    "Geometry",
    "LeastTargetedTransitionDescription",
    "Location",
    "NeighborView",
    "NonTransitionDescription",
    "NotifiableQueueProxy",
    "OperationalModel",
    "Polygon2D",
    "RoundRobinTransitionDescription",
    "RoutingEngine",
    "WaitingSetProxy",
    "WallView",
    "WaypointProxy",
]


def test_native_stubs_match_runtime(tmp_path, capsys):
    entries = [f"{MODULE}.{c}" for c in CLASSES] + [
        f"{MODULE}.{c}.__init__" for c in CLASSES_WITHOUT_CONSTRUCTOR
    ]
    allowlist = tmp_path / "allowlist.txt"
    allowlist.write_text("\n".join(re.escape(e) for e in entries))
    options = stubtest.parse_options([MODULE, "--allowlist", str(allowlist)])
    rc = stubtest.test_stubs(options)
    assert rc == 0, capsys.readouterr().out
