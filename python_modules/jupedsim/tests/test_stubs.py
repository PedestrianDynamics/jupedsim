# SPDX-License-Identifier: LGPL-3.0-or-later
import re

from mypy import stubtest

MODULE = "jupedsim.pyjupedsim"

# Every bound class has metaclass pybind11_builtins.pybind11_type, which the
# stubs do not declare.
CLASSES = [
    "AgentStep",
    "AgentView",
    "AnticipationVelocityModel",
    "AnticipationVelocityModelState",
    "BoundaryIndex",
    "BuildInfo",
    "CollisionFreeSpeedModel",
    "CollisionFreeSpeedModelState",
    "CollisionFreeSpeedModelV2",
    "CollisionFreeSpeedModelV2State",
    "CollisionFreeSpeedModelV3",
    "CollisionFreeSpeedModelV3State",
    "FloorfieldOptions",
    "GeneralizedCentrifugalForceModel",
    "GeneralizedCentrifugalForceModelState",
    "Geometry",
    "Location",
    "NeighborView",
    "OperationalModel",
    "Polygon2D",
    "SocialForceModel",
    "SocialForceModelState",
    "SpeedGrid",
    "SurfaceMeshShortestPathRoutingEngine",
    "Rule",
    "TravelTimeGrid",
    "WalkableSurface",
    "WallView",
    "WarpDriverModel",
    "WarpDriverModelState",
    "_Agent",
    "_CustomModelState",
    "_PythonModel",
    "_Simulation",
]

# Classes without a bound constructor inherit pybind11_object.__init__(*args,
# **kwargs), which only raises; stubgen emits no __init__ for them.
CLASSES_WITHOUT_CONSTRUCTOR = [
    "AgentStep",
    "AgentView",
    "BoundaryIndex",
    "Geometry",
    "Location",
    "NeighborView",
    "OperationalModel",
    "Polygon2D",
    "SpeedGrid",
    "Rule",
    "TravelTimeGrid",
    "WallView",
    "_Agent",
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
