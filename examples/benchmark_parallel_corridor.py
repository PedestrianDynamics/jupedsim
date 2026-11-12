# SPDX-License-Identifier: LGPL-3.0-or-later
"""Parallel scaling benchmark: corridor with obstacles, all operational models.

A 120 x 30 m corridor with twelve 2 x 2 m obstacles. Agents start in its first
28 m and walk to an exit at the far end. Every model is run once per thread
count; the script prints the run time, the speedup over the first thread count
and a hash of the final positions, which must be the same for every thread
count of a model.

Thread counts need `Simulation._obj.set_num_threads` (parallel-spike branch);
without it only one thread is run.

Usage:
    python benchmark_parallel_corridor.py
    python benchmark_parallel_corridor.py --threads 1,4,16 --models cfs,avm
"""

import argparse
import hashlib
import os
import struct
import time

import jupedsim as jps
import shapely

# The walkable area: the corridor minus two rows of square obstacles.
CORRIDOR = shapely.Polygon(
    [(0, 0), (120, 0), (120, 30), (0, 30)],
    [
        [(x, y), (x + 2, y), (x + 2, y + 2), (x, y + 2)]
        for x in range(30, 110, 15)
        for y in (6, 20)
    ],
)
START_AREA = shapely.Polygon([(1, 1), (28, 1), (28, 29), (1, 29)])
EXIT_AREA = shapely.Polygon([(118, 5), (120, 5), (120, 25), (118, 25)])

# Per model: model and agent state classes, number of agents, start spacing
# [m] and iterations (dt = 0.01 s).
MODELS = {
    "cfs": (
        jps.CollisionFreeSpeedModel,
        jps.CollisionFreeSpeedModelState,
        1500,
        0.45,
        200,
    ),
    "cfs2": (
        jps.CollisionFreeSpeedModelV2,
        jps.CollisionFreeSpeedModelV2State,
        1500,
        0.45,
        200,
    ),
    "cfs3": (
        jps.CollisionFreeSpeedModelV3,
        jps.CollisionFreeSpeedModelV3State,
        1500,
        0.45,
        200,
    ),
    "avm": (
        jps.AnticipationVelocityModel,
        jps.AnticipationVelocityModelState,
        1500,
        0.45,
        200,
    ),
    # ISSUE: with the dense 0.45 m start the social force model pushes agents
    # through walls within a few iterations ("move_on_surface(): path hit a
    # wall"), independent of the thread count. It runs with a sparse start.
    "sfm": (
        jps.SocialForceModel,
        jps.SocialForceModelState,
        300,
        1.0,
        200,
    ),
    "gcfm": (
        jps.GeneralizedCentrifugalForceModel,
        jps.GeneralizedCentrifugalForceModelState,
        300,
        0.45,
        200,
    ),
    "warp": (
        jps.WarpDriverModel,
        jps.WarpDriverModelState,
        300,
        0.45,
        60,
    ),
}


def start_positions(count, spacing):
    return jps.distribute_by_number(
        polygon=START_AREA,
        number_of_agents=count,
        distance_to_agents=spacing,
        distance_to_polygon=max(0.3, spacing / 2),
        seed=7,
    )


def run(name, threads):
    """Runs one model at one thread count; returns (seconds, hash, agents)."""
    model, state, count, spacing, iterations = MODELS[name]
    surface = jps.WalkableSurface()
    room = surface.add_region(polygon=CORRIDOR)
    simulation = jps.Simulation(model=model(), geometry=surface, dt=0.01)
    if hasattr(simulation._obj, "set_num_threads"):
        simulation._obj.set_num_threads(threads)
    exit_id = simulation.add_exit_stage([(EXIT_AREA, room)])
    journey = simulation.add_journey(jps.JourneyDescription([exit_id]))
    for position in start_positions(count, spacing):
        simulation.add_agent(
            journey_id=journey,
            stage_id=exit_id,
            position=position,
            state=state(),
            region_id=room,
        )
    # Floor fields are solved on first use: keep that out of the timing.
    simulation.iterate(2)
    start = time.perf_counter()
    try:
        simulation.iterate(iterations)
    except jps.SimulationError as error:
        elapsed = time.perf_counter() - start
        return elapsed, f"ERROR: {error}", simulation.agent_count()
    elapsed = time.perf_counter() - start
    # Agent ids come from a process-wide counter, so hash positions only.
    digest = hashlib.sha256()
    for agent in sorted(simulation.agents(), key=lambda a: a.id):
        digest.update(struct.pack("<2d", *agent.position))
    return elapsed, digest.hexdigest()[:16], simulation.agent_count()


def main():
    cores = os.cpu_count() or 1
    default_threads = [t for t in (1, 4, 8, 16, 32, 64) if t <= cores]
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--threads",
        default=",".join(str(t) for t in default_threads),
        help="comma-separated thread counts (default: %(default)s)",
    )
    parser.add_argument(
        "--models",
        default=",".join(MODELS),
        help="comma-separated models (default: %(default)s)",
    )
    args = parser.parse_args()
    threads = [int(t) for t in args.threads.split(",")]
    probe = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=_probe_surface(), dt=0.01
    )
    if not hasattr(probe._obj, "set_num_threads"):
        print("this jupedsim has no threading support: running 1 thread")
        threads = [1]

    for name in args.models.split(","):
        _, _, count, _, iterations = MODELS[name]
        cells = []
        hashes = set()
        base = None
        for t in threads:
            elapsed, digest, _ = run(name, t)
            hashes.add(digest)
            base = base or elapsed
            cells.append(f"{t}:{elapsed:6.2f}s x{base / elapsed:4.1f}")
        same = "same" if len(hashes) == 1 else "DIFFERENT"
        print(
            f"{name:5} {count:5d} agents {iterations:4d} it  "
            + "  ".join(cells)
            + f"  positions {same}: {sorted(hashes)}",
            flush=True,
        )


def _probe_surface():
    surface = jps.WalkableSurface()
    surface.add_region(polygon=shapely.box(0, 0, 1, 1))
    return surface


if __name__ == "__main__":
    main()
