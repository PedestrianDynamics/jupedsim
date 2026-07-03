# SPDX-License-Identifier: LGPL-3.0-or-later
"""QA baseline: run one deterministic scenario per operational model and dump
trajectories as '<iteration>,<agent_id>,<repr(x)>,<repr(y)>' lines."""

import os
import sys

import jupedsim as jps
import shapely
from jupedsim_examples.models.pysocial_force import (
    PythonSocialForceModel,
    PythonSocialForceModelState,
)

ROOM = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
EXIT_POLY = [(19, 9), (19, 11), (20, 11), (20, 9)]
DT = 0.05
MAX_ITERATIONS = 500


def grid_positions():
    # 4x4 grid, 2 m spacing (GCFM requires generous inter-agent distance).
    return [
        (2.0 + col * 2.0, 7.0 + row * 2.0)
        for row in range(4)
        for col in range(4)
    ]


def run(model, make_agent_params, out_path):
    sim = jps.Simulation(model=model, geometry=ROOM, dt=DT)
    exit_id = sim.add_exit_stage(EXIT_POLY)
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    for pos in grid_positions():
        sim.add_agent(make_agent_params(pos, journey_id, exit_id))

    lines = []
    for it in range(1, MAX_ITERATIONS + 1):
        if sim.agent_count() == 0:
            break
        sim.iterate()
        for agent in sim.agents():
            x, y = agent.position
            lines.append(f"{it},{int(agent.id)},{x!r},{y!r}\n")
    with open(out_path, "w") as f:
        f.writelines(lines)


def scenarios():
    yield (
        "CollisionFreeSpeedModel",
        jps.CollisionFreeSpeedModel(),
        lambda pos, j, s: jps.CollisionFreeSpeedModelAgentParameters(
            position=pos, journey_id=j, stage_id=s
        ),
    )
    yield (
        "CollisionFreeSpeedModelV2",
        jps.CollisionFreeSpeedModelV2(),
        lambda pos, j, s: jps.CollisionFreeSpeedModelV2AgentParameters(
            position=pos, journey_id=j, stage_id=s
        ),
    )
    yield (
        "CollisionFreeSpeedModelV3",
        jps.CollisionFreeSpeedModelV3(),
        lambda pos, j, s: jps.CollisionFreeSpeedModelV3AgentParameters(
            position=pos, journey_id=j, stage_id=s
        ),
    )
    yield (
        "GeneralizedCentrifugalForceModel",
        jps.GeneralizedCentrifugalForceModel(),
        lambda pos, j, s: jps.GeneralizedCentrifugalForceModelAgentParameters(
            position=pos, journey_id=j, stage_id=s
        ),
    )
    yield (
        "AnticipationVelocityModel",
        jps.AnticipationVelocityModel(rng_seed=1234),
        lambda pos, j, s: jps.AnticipationVelocityModelAgentParameters(
            position=pos, journey_id=j, stage_id=s
        ),
    )
    yield (
        "SocialForceModel",
        jps.SocialForceModel(),
        lambda pos, j, s: jps.SocialForceModelAgentParameters(
            position=pos, journey_id=j, stage_id=s
        ),
    )
    yield (
        "WarpDriverModel",
        jps.WarpDriverModel(),
        lambda pos, j, s: jps.WarpDriverModelAgentParameters(
            position=pos, journey_id=j, stage_id=s
        ),
    )
    yield (
        "PythonSocialForceModel",
        PythonSocialForceModel(),
        lambda pos, j, s: jps.CustomModelAgentParameters(
            journey_id=j,
            stage_id=s,
            model=PythonSocialForceModelState(
                position=pos, velocity=(0.0, 0.0)
            ),
        ),
    )


def main():
    out_dir = sys.argv[1]
    os.makedirs(out_dir, exist_ok=True)
    for name, model, make_params in scenarios():
        out_path = os.path.join(out_dir, f"{name}.txt")
        run(model, make_params, out_path)
        print(f"{name}: wrote {out_path}")


if __name__ == "__main__":
    main()
