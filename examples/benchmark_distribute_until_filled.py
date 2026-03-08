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


def mean_nearest_neighbor_distance(
    src: list[tuple[float, float]],
    dst: list[tuple[float, float]],
    cell_size: float,
) -> float:
    if not src or not dst:
        return float("inf")

    cells = defaultdict(list)
    for x, y in dst:
        cx = int(math.floor(x / cell_size))
        cy = int(math.floor(y / cell_size))
        cells[(cx, cy)].append((x, y))

    total = 0.0
    for x1, y1 in src:
        cx = int(math.floor(x1 / cell_size))
        cy = int(math.floor(y1 / cell_size))

        best_d2 = float("inf")
        radius = 0
        while True:
            found_any = False
            for dx in range(-radius, radius + 1):
                for dy in range(-radius, radius + 1):
                    if max(abs(dx), abs(dy)) != radius:
                        continue
                    items = cells.get((cx + dx, cy + dy), [])
                    if not items:
                        continue
                    found_any = True
                    for x2, y2 in items:
                        d2 = (x1 - x2) ** 2 + (y1 - y2) ** 2
                        if d2 < best_d2:
                            best_d2 = d2

            if best_d2 < float("inf"):
                best_d = math.sqrt(best_d2)
                max_possible_better = max(0.0, radius - 1) * cell_size
                if best_d <= max_possible_better:
                    break

            if found_any and radius > 1_000_000:
                break

            radius += 1

        total += math.sqrt(best_d2)

    return total / len(src)


def mean_bidirectional_nn_distance(
    a: list[tuple[float, float]], b: list[tuple[float, float]], cell_size: float
) -> float:
    if not a or not b:
        return float("inf")
    return 0.5 * (
        mean_nearest_neighbor_distance(a, b, cell_size)
        + mean_nearest_neighbor_distance(b, a, cell_size)
    )


def nearest_neighbor_distances(
    points: list[tuple[float, float]], cell_size: float
) -> list[float]:
    if len(points) < 2:
        return []
    cells = defaultdict(list)
    for x, y in points:
        cx = int(math.floor(x / cell_size))
        cy = int(math.floor(y / cell_size))
        cells[(cx, cy)].append((x, y))

    out = []
    for x1, y1 in points:
        cx = int(math.floor(x1 / cell_size))
        cy = int(math.floor(y1 / cell_size))
        best_d2 = float("inf")
        radius = 0

        while True:
            found_any = False
            for dx in range(-radius, radius + 1):
                for dy in range(-radius, radius + 1):
                    if max(abs(dx), abs(dy)) != radius:
                        continue
                    items = cells.get((cx + dx, cy + dy), [])
                    if not items:
                        continue
                    found_any = True
                    for x2, y2 in items:
                        if x1 == x2 and y1 == y2:
                            continue
                        d2 = (x1 - x2) ** 2 + (y1 - y2) ** 2
                        if d2 < best_d2:
                            best_d2 = d2

            if best_d2 < float("inf"):
                best_d = math.sqrt(best_d2)
                max_possible_better = max(0.0, radius - 1) * cell_size
                if best_d <= max_possible_better:
                    break

            if found_any and radius > 1_000_000:
                break

            radius += 1

        out.append(math.sqrt(best_d2))
    return out


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
    parser.add_argument(
        "--nn-hist-file",
        type=pathlib.Path,
        default=pathlib.Path("benchmark_nn_distance_hist.png"),
    )
    parser.add_argument(
        "--points-per-seed-file",
        type=pathlib.Path,
        default=pathlib.Path("benchmark_points_per_seed.png"),
    )
    parser.add_argument(
        "--seed-difference-heatmap-file",
        type=pathlib.Path,
        default=pathlib.Path("benchmark_seed_difference_heatmap.png"),
    )
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--no-assert", action="store_true")
    parser.add_argument(
        "--count-relative-range-limit", type=float, default=0.01
    )
    parser.add_argument("--min-seed-diversity", type=float, default=0.1)
    parser.add_argument("--tolerance", type=float, default=1e-6)
    return parser.parse_args()


def _import_pyplot():
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    return plt


def save_point_plot(
    points: list[tuple[float, float]],
    polygon: Polygon,
    output_path: pathlib.Path,
) -> None:
    plt = _import_pyplot()

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


def save_nn_histogram(
    trial_nn_all: list[list[float]],
    target_distance: float,
    output_path: pathlib.Path,
) -> None:
    plt = _import_pyplot()

    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig, ax = plt.subplots(figsize=(7, 4.5))

    for idx, dists in enumerate(trial_nn_all):
        if not dists:
            continue
        ax.hist(dists, bins=80, alpha=0.35, label=f"trial {idx}")

    ax.axvline(target_distance, linestyle="--", linewidth=1.5, color="black")
    ax.set_xlabel("nearest neighbor distance")
    ax.set_ylabel("count")
    ax.set_title("Nearest neighbor distance distribution")
    if len(trial_nn_all) <= 10:
        ax.legend()
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)


def save_points_per_seed_plot(
    counts: list[int],
    seeds: list[int],
    output_path: pathlib.Path,
) -> None:
    plt = _import_pyplot()

    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig, ax = plt.subplots(figsize=(7, 4.5))
    x = list(range(len(counts)))
    ax.bar(x, counts)
    ax.set_xticks(x)
    ax.set_xticklabels([str(seed) for seed in seeds], rotation=45, ha="right")
    ax.set_xlabel("seed")
    ax.set_ylabel("points")
    ax.set_title("Points per seed")
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)


def save_seed_difference_heatmap(
    seeds: list[int],
    seed_differences: list[dict[str, float | int]],
    output_path: pathlib.Path,
) -> None:
    plt = _import_pyplot()

    n = len(seeds)
    matrix = [[0.0] * n for _ in range(n)]
    for item in seed_differences:
        i = int(item["trial_i"])
        j = int(item["trial_j"])
        d = float(item["distance"])
        matrix[i][j] = d
        matrix[j][i] = d

    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig, ax = plt.subplots(figsize=(6, 5))
    im = ax.imshow(matrix)
    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label("bidirectional NN distance")
    ax.set_xticks(list(range(n)))
    ax.set_yticks(list(range(n)))
    ax.set_xticklabels([str(seed) for seed in seeds], rotation=45, ha="right")
    ax.set_yticklabels([str(seed) for seed in seeds])
    ax.set_xlabel("seed")
    ax.set_ylabel("seed")
    ax.set_title("Seed layout difference matrix")
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
    trial_seeds = []
    all_points: list[list[tuple[float, float]]] = []
    trial_nn_all: list[list[float]] = []
    trial_nn_mean = []

    for i in range(args.trials):
        print(f"{i}/{args.trials}")

        trial_seed = args.seed + i
        t0 = time.perf_counter()
        points = dist.distribute_until_filled(
            polygon=polygon,
            distance_to_agents=args.distance_to_agents,
            distance_to_polygon=args.distance_to_polygon,
            seed=trial_seed,
            max_iterations=args.max_iterations,
            k=args.k,
        )
        dt = time.perf_counter() - t0

        nn_dists = nearest_neighbor_distances(
            points, cell_size=args.distance_to_agents
        )

        trial_seeds.append(trial_seed)
        runtimes.append(dt)
        counts.append(len(points))
        min_pairs.append(min_pair_distance(points, args.distance_to_agents))
        min_boundaries.append(min_boundary_distance(points, polygon))
        all_points.append(points)
        trial_nn_all.append(nn_dists)
        trial_nn_mean.append(
            statistics.fmean(nn_dists) if nn_dists else float("inf")
        )

    seed_differences = []
    for i in range(len(all_points)):
        for j in range(i + 1, len(all_points)):
            distance = mean_bidirectional_nn_distance(
                all_points[i], all_points[j], args.distance_to_agents
            )
            seed_differences.append(
                {
                    "trial_i": i,
                    "trial_j": j,
                    "seed_i": trial_seeds[i],
                    "seed_j": trial_seeds[j],
                    "distance": distance,
                }
            )

    usable_area = polygon.buffer(-args.distance_to_polygon).area
    mean_count = statistics.fmean(counts)
    mean_density = mean_count / usable_area if usable_area > 0 else 0.0
    hex_upper = 2.0 / (math.sqrt(3.0) * args.distance_to_agents**2)
    count_relative_range = (
        (max(counts) - min(counts)) / mean_count if mean_count > 0 else 0.0
    )

    seed_difference_values = [item["distance"] for item in seed_differences]

    out = {
        "config": {
            "size": args.size,
            "distance_to_agents": args.distance_to_agents,
            "distance_to_polygon": args.distance_to_polygon,
            "max_iterations": args.max_iterations,
            "k": args.k,
            "seed": args.seed,
            "trials": args.trials,
            "trial_seeds": trial_seeds,
        },
        "runtime_seconds": {
            "mean": statistics.fmean(runtimes),
            "median": statistics.median(runtimes),
            "min": min(runtimes),
            "max": max(runtimes),
            "per_trial": runtimes,
        },
        "capacity": {
            "counts_per_trial": counts,
            "count_mean": mean_count,
            "count_min": min(counts),
            "count_max": max(counts),
            "count_stdev": statistics.stdev(counts)
            if len(counts) >= 2
            else 0.0,
            "count_relative_range": count_relative_range,
            "density_per_m2_mean": mean_density,
            "hex_upper_bound_per_m2": hex_upper,
            "density_vs_hex_upper_bound": mean_density / hex_upper
            if hex_upper > 0
            else 0.0,
        },
        "quality": {
            "min_pair_distance_per_trial": min_pairs,
            "min_boundary_distance_per_trial": min_boundaries,
            "min_pair_distance_min": min(min_pairs),
            "min_boundary_distance_min": min(min_boundaries),
            "target_distance_to_agents": args.distance_to_agents,
            "target_distance_to_polygon": args.distance_to_polygon,
            "nearest_neighbor_distance_mean_per_trial": trial_nn_mean,
        },
        "seed_sensitivity": {
            "pairwise": seed_differences,
            "pairwise_mean_bidirectional_nn_distance_mean": (
                statistics.fmean(seed_difference_values)
                if seed_difference_values
                else 0.0
            ),
            "pairwise_mean_bidirectional_nn_distance_min": (
                min(seed_difference_values) if seed_difference_values else 0.0
            ),
            "pairwise_mean_bidirectional_nn_distance_max": (
                max(seed_difference_values) if seed_difference_values else 0.0
            ),
        },
        "plots": {
            "distribution": str(args.plot_file),
            "nn_histogram": str(args.nn_hist_file),
            "points_per_seed": str(args.points_per_seed_file),
            "seed_difference_heatmap": str(args.seed_difference_heatmap_file),
        },
    }

    save_point_plot(
        all_points[0] if all_points else [], polygon, args.plot_file
    )
    save_nn_histogram(trial_nn_all, args.distance_to_agents, args.nn_hist_file)
    save_points_per_seed_plot(counts, trial_seeds, args.points_per_seed_file)
    save_seed_difference_heatmap(
        trial_seeds, seed_differences, args.seed_difference_heatmap_file
    )

    if not args.no_assert:
        tol = args.tolerance
        assert min(min_pairs) >= args.distance_to_agents - tol, (
            f"Min pair distance {min(min_pairs):.6f} is below target "
            f"{args.distance_to_agents:.6f} with tolerance {tol:.6f}"
        )
        assert min(min_boundaries) >= args.distance_to_polygon - tol, (
            f"Min boundary distance {min(min_boundaries):.6f} is below target "
            f"{args.distance_to_polygon:.6f} with tolerance {tol:.6f}"
        )
        assert count_relative_range <= args.count_relative_range_limit, (
            f"Count relative range {count_relative_range:.6f} exceeds limit "
            f"{args.count_relative_range_limit:.6f}"
        )
        assert mean_density <= hex_upper + tol, (
            f"Mean density {mean_density:.6f} exceeds hex upper bound "
            f"{hex_upper:.6f} with tolerance {tol:.6f}"
        )
        if len(seed_difference_values) > 0:
            assert min(seed_difference_values) >= args.min_seed_diversity, (
                f"Seed diversity minimum {min(seed_difference_values):.6f} is below "
                f"required threshold {args.min_seed_diversity:.6f}"
            )

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
        f"  count stdev / relative range: "
        f"{out['capacity']['count_stdev']:.3f}/"
        f"{100.0 * out['capacity']['count_relative_range']:.4f}%"
    )
    print(
        f"  density [1/m2]: {out['capacity']['density_per_m2_mean']:.4f} "
        f"(vs hex upper bound: {out['capacity']['hex_upper_bound_per_m2']:.4f}, "
        f"ratio: {out['capacity']['density_vs_hex_upper_bound']:.4f})"
    )
    print(
        f"  min pair distance: {out['quality']['min_pair_distance_min']:.4f} "
        f"(target {out['quality']['target_distance_to_agents']:.4f})"
    )
    print(
        f"  min boundary distance: {out['quality']['min_boundary_distance_min']:.4f} "
        f"(target {out['quality']['target_distance_to_polygon']:.4f})"
    )
    if seed_difference_values:
        print(
            f"  seed sensitivity mean/min/max [bidirectional NN distance]: "
            f"{out['seed_sensitivity']['pairwise_mean_bidirectional_nn_distance_mean']:.4f}/"
            f"{out['seed_sensitivity']['pairwise_mean_bidirectional_nn_distance_min']:.4f}/"
            f"{out['seed_sensitivity']['pairwise_mean_bidirectional_nn_distance_max']:.4f}"
        )
    print(f"  plot: {args.plot_file}")
    print(f"  nn histogram: {args.nn_hist_file}")
    print(f"  points per seed: {args.points_per_seed_file}")
    print(f"  seed difference heatmap: {args.seed_difference_heatmap_file}")


if __name__ == "__main__":
    main()
