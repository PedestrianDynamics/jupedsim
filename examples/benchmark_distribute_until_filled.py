#!/usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later
import argparse
import importlib.util
import json
import math
import pathlib
import statistics
import sys
import time
import types
from collections import defaultdict

from shapely import Point, Polygon


def _load_distributions_module():
    root = pathlib.Path(__file__).resolve().parents[1]
    dist_path = (
        root / "python_modules" / "jupedsim" / "jupedsim" / "distributions.py"
    )
    grid_path = (
        root
        / "python_modules"
        / "jupedsim"
        / "jupedsim"
        / "internal"
        / "grid.py"
    )

    if "jupedsim" not in sys.modules:
        pkg = types.ModuleType("jupedsim")
        pkg.__path__ = []  # type: ignore[attr-defined]
        sys.modules["jupedsim"] = pkg
    if "jupedsim.internal" not in sys.modules:
        internal_pkg = types.ModuleType("jupedsim.internal")
        internal_pkg.__path__ = []  # type: ignore[attr-defined]
        sys.modules["jupedsim.internal"] = internal_pkg

    grid_spec = importlib.util.spec_from_file_location(
        "jupedsim.internal.grid", grid_path
    )
    if grid_spec is None or grid_spec.loader is None:
        raise RuntimeError(f"Could not load grid module from {grid_path}")
    grid_module = importlib.util.module_from_spec(grid_spec)
    sys.modules["jupedsim.internal.grid"] = grid_module
    grid_spec.loader.exec_module(grid_module)

    dist_spec = importlib.util.spec_from_file_location(
        "local_distributions", dist_path
    )
    if dist_spec is None or dist_spec.loader is None:
        raise RuntimeError(
            f"Could not load distributions module from {dist_path}"
        )
    dist_module = importlib.util.module_from_spec(dist_spec)
    dist_spec.loader.exec_module(dist_module)
    return dist_module


dist = _load_distributions_module()


def build_square(size: float) -> Polygon:
    return Polygon([(0.0, 0.0), (size, 0.0), (size, size), (0.0, size)])


def min_pair_distance(
    points: list[tuple[float, float]], cell_size: float
) -> float:
    if len(points) < 2:
        return float("inf")
    cells = defaultdict(list)
    for x, y in points:
        cx = int(math.floor(x / cell_size))
        cy = int(math.floor(y / cell_size))
        cells[(cx, cy)].append((x, y))

    min_d2 = float("inf")
    for (cx, cy), items in cells.items():
        neighbors = []
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                neighbors.extend(cells.get((cx + dx, cy + dy), []))
        for x1, y1 in items:
            for x2, y2 in neighbors:
                if x1 == x2 and y1 == y2:
                    continue
                d2 = (x1 - x2) ** 2 + (y1 - y2) ** 2
                if d2 < min_d2:
                    min_d2 = d2
    return math.sqrt(min_d2)


def min_boundary_distance(
    points: list[tuple[float, float]], polygon: Polygon
) -> float:
    boundary = polygon.boundary
    if not points:
        return float("inf")
    return min(boundary.distance(Point(x, y)) for x, y in points)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Benchmark distribute_until_filled on a square."
    )
    parser.add_argument("--size", type=float, default=100.0)
    parser.add_argument("--distance-to-agents", type=float, default=0.4)
    parser.add_argument("--distance-to-polygon", type=float, default=0.2)
    parser.add_argument("--max-iterations", type=int, default=10_000)
    parser.add_argument("--k", type=int, default=30)
    parser.add_argument("--seed", type=int, default=1234)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument(
        "--plot-file",
        type=pathlib.Path,
        default=pathlib.Path("benchmark_distribute_until_filled_points.png"),
    )
    parser.add_argument("--json", action="store_true")
    return parser.parse_args()


def save_plot(
    points: list[tuple[float, float]],
    polygon: Polygon,
    output_path: pathlib.Path,
) -> None:
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    output_path.parent.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(8, 8))
    if points:
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        ax.scatter(xs, ys, s=2, alpha=0.6, color="#0b4f6c")

    outer_x, outer_y = polygon.exterior.xy
    ax.plot(outer_x, outer_y, color="#c1121f", linewidth=1.5)
    for interior in polygon.interiors:
        hole_x, hole_y = interior.xy
        ax.plot(hole_x, hole_y, color="#c1121f", linewidth=1.0)

    ax.set_aspect("equal", adjustable="box")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_title("distribute_until_filled samples")
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)


def main():
    args = parse_args()
    polygon = build_square(args.size)

    runtimes = []
    counts = []
    min_pairs = []
    min_boundaries = []
    first_trial_points: list[tuple[float, float]] = []

    for i in range(args.trials):
        t0 = time.perf_counter()
        points = dist.distribute_until_filled(
            polygon=polygon,
            distance_to_agents=args.distance_to_agents,
            distance_to_polygon=args.distance_to_polygon,
            seed=args.seed + i,
            max_iterations=args.max_iterations,
            k=args.k,
        )
        dt = time.perf_counter() - t0

        runtimes.append(dt)
        counts.append(len(points))
        min_pairs.append(min_pair_distance(points, args.distance_to_agents))
        min_boundaries.append(min_boundary_distance(points, polygon))
        if i == 0:
            first_trial_points = points

    usable_area = polygon.buffer(-args.distance_to_polygon).area
    mean_count = statistics.fmean(counts)
    mean_density = mean_count / usable_area if usable_area > 0 else 0.0
    hex_upper = 2.0 / (math.sqrt(3.0) * args.distance_to_agents**2)

    out = {
        "config": {
            "size": args.size,
            "distance_to_agents": args.distance_to_agents,
            "distance_to_polygon": args.distance_to_polygon,
            "max_iterations": args.max_iterations,
            "k": args.k,
            "seed": args.seed,
            "trials": args.trials,
        },
        "runtime_seconds": {
            "mean": statistics.fmean(runtimes),
            "median": statistics.median(runtimes),
            "min": min(runtimes),
            "max": max(runtimes),
            "per_trial": runtimes,
        },
        "capacity": {
            "count_mean": mean_count,
            "count_min": min(counts),
            "count_max": max(counts),
            "density_per_m2_mean": mean_density,
            "density_vs_hex_upper_bound": mean_density / hex_upper
            if hex_upper > 0
            else 0.0,
        },
        "quality": {
            "min_pair_distance_min": min(min_pairs),
            "min_boundary_distance_min": min(min_boundaries),
            "target_distance_to_agents": args.distance_to_agents,
            "target_distance_to_polygon": args.distance_to_polygon,
        },
        "plot_file": str(args.plot_file),
    }

    save_plot(first_trial_points, polygon, args.plot_file)

    if args.json:
        print(json.dumps(out, indent=2, sort_keys=True))
        return

    print("Benchmark result:")
    print(
        f"  runtime [s] mean/median/min/max: "
        f"{out['runtime_seconds']['mean']:.4f}/"
        f"{out['runtime_seconds']['median']:.4f}/"
        f"{out['runtime_seconds']['min']:.4f}/"
        f"{out['runtime_seconds']['max']:.4f}"
    )
    print(
        f"  points mean/min/max: "
        f"{out['capacity']['count_mean']:.1f}/"
        f"{out['capacity']['count_min']}/"
        f"{out['capacity']['count_max']}"
    )
    print(
        f"  density [1/m2]: {out['capacity']['density_per_m2_mean']:.4f} "
        f"(vs hex upper bound: {out['capacity']['density_vs_hex_upper_bound']:.4f})"
    )
    print(
        f"  min pair distance: {out['quality']['min_pair_distance_min']:.4f} "
        f"(target {out['quality']['target_distance_to_agents']:.4f})"
    )
    print(
        f"  min boundary distance: {out['quality']['min_boundary_distance_min']:.4f} "
        f"(target {out['quality']['target_distance_to_polygon']:.4f})"
    )
    print(f"  plot: {args.plot_file}")


if __name__ == "__main__":
    main()
