# SPDX-License-Identifier: LGPL-3.0-or-later
"""The route API example scripts run to completion."""

import subprocess
import sys
from pathlib import Path

import pytest

EXAMPLES = Path(__file__).resolve().parents[3] / "examples"
SCRIPTS = [
    "route_api_evacuation",
    "route_api_two_floors",
    "route_api_weighted_split",
    "route_api_phased_release",
    "route_api_closure_rerouting",
    "route_api_last_train",
]


@pytest.mark.parametrize("name", SCRIPTS)
def test_example_runs(name, tmp_path):
    result = subprocess.run(
        [sys.executable, str(EXAMPLES / f"{name}.py")],
        cwd=tmp_path,
        capture_output=True,
        text=True,
        timeout=300,
    )
    assert result.returncode == 0, result.stderr
