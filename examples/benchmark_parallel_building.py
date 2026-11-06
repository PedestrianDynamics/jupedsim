# SPDX-License-Identifier: LGPL-3.0-or-later
"""Parallel scaling benchmark: five-storey building, all operational models.

Five 60 x 40 m floors, 3 m apart and stacked in plan, each with a grid of
1 x 1 m pillars. Stairs connect each floor with the next through a 6 x 4 m
stairwell; the wells alternate between two sides of the building, so leaving
it zig-zags down floor by floor to the only exit, on the ground floor.

Every model is run once per thread count; the script prints the run time, the
speedup over the first thread count and a hash of the final positions, which
must be the same for every thread count of a model.

Thread counts need `Simulation._obj.set_num_threads` (parallel-spike branch);
without it only one thread is run.

Usage:
    python benchmark_parallel_building.py
    python benchmark_parallel_building.py --threads 1,16,64 --models cfs,gcfm
"""

import argparse
import hashlib
import os
import struct
import time

import jupedsim as jps
import shapely

FLOORS = 5
WIDTH, DEPTH, STOREY = 60.0, 40.0, 3.0
# Stairwells as (x0, x1, y0, y1): the stairs from floor k to k + 1 run along
# +x through band A for even k and band B for odd k.
WELL_A = (20.0, 26.0, 5.0, 9.0)
WELL_B = (34.0, 40.0, 31.0, 35.0)
EXIT_AREA = shapely.Polygon([(58, 15), (60, 15), (60, 25), (58, 25)])

# Per model: model and agent state classes, agents per floor, start spacing
# [m] and iterations (dt = 0.01 s).
MODELS = {
    "cfs": (
        jps.CollisionFreeSpeedModel,
        jps.CollisionFreeSpeedModelState,
        2000,
        0.5,
        40,
    ),
    "cfs2": (
        jps.CollisionFreeSpeedModelV2,
        jps.CollisionFreeSpeedModelV2State,
        2000,
        0.5,
        40,
    ),
    "cfs3": (
        jps.CollisionFreeSpeedModelV3,
        jps.CollisionFreeSpeedModelV3State,
        2000,
        0.5,
        40,
    ),
    "avm": (
        jps.AnticipationVelocityModel,
        jps.AnticipationVelocityModelState,
        2000,
        0.5,
        40,
    ),
    # ISSUE: with the dense 0.5 m start (10,000 agents) the social force model
    # pushes agents through walls within 5 iterations ("move_on_surface(): path
    # hit a wall"), independent of the thread count. It runs with a sparser
    # start of 1 m (3,000 agents), which runs for at least 300 iterations.
    "sfm": (
        jps.SocialForceModel,
        jps.SocialForceModelState,
        600,
        1.0,
        40,
    ),
    "gcfm": (
        jps.GeneralizedCentrifugalForceModel,
        jps.GeneralizedCentrifugalForceModelState,
        2000,
        0.5,
        40,
    ),
    "warp": (
        jps.WarpDriverModel,
        jps.WarpDriverModelState,
        2000,
        0.5,
        40,
    ),
}


def stairs_well(k):
    """Stairwell of the stairs from floor k to floor k + 1."""
    return WELL_A if k % 2 == 0 else WELL_B


def rectangle(x0, x1, y0, y1):
    return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]


def floor_polygon(k):
    """Floor k: the slab minus its stairwells and pillars."""
    holes = []
    if k < FLOORS - 1:
        holes.append(rectangle(*stairs_well(k)))  # stairs going up
    if k > 0:
        holes.append(rectangle(*stairs_well(k - 1)))  # stairs from below
    # Keep pillars clear of both wells and of the exit.
    keep_clear = [
        shapely.box(w[0] - 2, w[2] - 2, w[1] + 2, w[3] + 2)
        for w in (WELL_A, WELL_B)
    ] + [EXIT_AREA.buffer(3)]
    for x in range(8, 56, 8):
        for y in range(6, 38, 7):
            pillar = shapely.box(x, y, x + 1, y + 1)
            if not any(pillar.intersects(area) for area in keep_clear):
                holes.append(rectangle(x, x + 1, y, y + 1))
    return shapely.Polygon(rectangle(0, WIDTH, 0, DEPTH), holes)


def building():
    """The walkable surface and the region ids of its floors."""
    surface = jps.WalkableSurface()
    floors = [
        surface.add_region(polygon=floor_polygon(k), height=STOREY * k)
        for k in range(FLOORS)
    ]
    for k in range(FLOORS - 1):
        x0, x1, y0, y1 = stairs_well(k)
        surface.connect_regions(
            from_region=floors[k],
            from_edge=((x0, y0), (x0, y1)),
            to_region=floors[k + 1],
            to_edge=((x1, y0), (x1, y1)),
        )
    return surface, floors


def start_positions(per_floor, spacing):
    """Start positions per floor, clear of walls, pillars and wells."""
    return [
        jps.distribute_by_number(
            polygon=floor_polygon(k).buffer(-0.2),
            number_of_agents=per_floor,
            distance_to_agents=spacing,
            distance_to_polygon=max(0.3, spacing / 2),
            seed=11 + k,
        )
        for k in range(FLOORS)
    ]


def run(name, threads):
    """Runs one model at one thread count; returns (seconds, hash, agents)."""
    model, state, per_floor, spacing, iterations = MODELS[name]
    surface, floors = building()
    simulation = jps.Simulation(model=model(), geometry=surface, dt=0.01)
    if hasattr(simulation._obj, "set_num_threads"):
        simulation._obj.set_num_threads(threads)
    exit_id = simulation.add_exit_stage([(EXIT_AREA, floors[0])])
    journey = simulation.add_journey(jps.JourneyDescription([exit_id]))
    for k, positions in enumerate(start_positions(per_floor, spacing)):
        for position in positions:
            simulation.add_agent(
                journey_id=journey,
                stage_id=exit_id,
                position=position,
                state=state(),
                region_id=floors[k],
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
    probe_surface = jps.WalkableSurface()
    probe_surface.add_region(polygon=shapely.box(0, 0, 1, 1))
    probe = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=probe_surface, dt=0.01
    )
    if not hasattr(probe._obj, "set_num_threads"):
        print("this jupedsim has no threading support: running 1 thread")
        threads = [1]

    for name in args.models.split(","):
        _, _, per_floor, _, iterations = MODELS[name]
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
            f"{name:5} {per_floor * FLOORS:6d} agents {iterations:3d} it  "
            + "  ".join(cells)
            + f"  positions {same}: {sorted(hashes)}",
            flush=True,
        )


if __name__ == "__main__":
    main()
