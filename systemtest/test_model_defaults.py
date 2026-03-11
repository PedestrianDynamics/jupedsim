# SPDX-License-Identifier: LGPL-3.0-or-later
import dataclasses

import pytest

import jupedsim.native as py_jps
from jupedsim.models.anticipation_velocity_model import (
    AnticipationVelocityModelAgentParameters,
)
from jupedsim.models.collision_free_speed import (
    CollisionFreeSpeedModelAgentParameters,
)
from jupedsim.models.collision_free_speed_v2 import (
    CollisionFreeSpeedModelV2AgentParameters,
)
from jupedsim.models.generalized_centrifugal_force import (
    GeneralizedCentrifugalForceModelAgentParameters,
)
from jupedsim.models.social_force import (
    SocialForceModelAgentParameters,
)

MODELS = [
    (
        CollisionFreeSpeedModelAgentParameters,
        py_jps.CollisionFreeSpeedModelState,
    ),
    (
        CollisionFreeSpeedModelV2AgentParameters,
        py_jps.CollisionFreeSpeedModelV2State,
    ),
    (
        AnticipationVelocityModelAgentParameters,
        py_jps.AnticipationVelocityModelState,
    ),
    (
        GeneralizedCentrifugalForceModelAgentParameters,
        py_jps.GeneralizedCentrifugalForceModelState,
    ),
    (
        SocialForceModelAgentParameters,
        py_jps.SocialForceModelState,
    ),
]

# Fields that exist only in Python (not in C++ model data)
PYTHON_ONLY_FIELDS = {"position", "journey_id", "stage_id", "orientation"}


@pytest.mark.parametrize("py_params_cls, cpp_state_cls", MODELS)
def test_cpp_defaults_match_python(py_params_cls, cpp_state_cls):
    """C++ struct defaults must match Python AgentParameters defaults."""
    py_defaults = py_params_cls()
    cpp_defaults = cpp_state_cls._defaults()

    for field in dataclasses.fields(py_defaults):
        if field.name in PYTHON_ONLY_FIELDS:
            continue
        py_val = getattr(py_defaults, field.name)
        cpp_val = getattr(cpp_defaults, field.name, None)
        if cpp_val is None:
            continue
        assert py_val == pytest.approx(cpp_val), (
            f"{py_params_cls.__name__}.{field.name}: "
            f"Python={py_val}, C++={cpp_val}"
        )
