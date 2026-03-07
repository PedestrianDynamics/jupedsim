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
    module_path = (
        root
        / "python_modules"
        / "jupedsim"
        / "jupedsim"
        / "distributions.py"
    )
    grid_path = (
        root
        / "python_modules"
        / "jupedsim"
        / "jupedsim"
        / "internal"
        / "grid.py"
    )

    # Create lightweight package shells so distributions.py can import
    # `jupedsim.internal.grid` without importing native bindings.
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

    spec = importlib.util.spec_from_file_location("local_distributions", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load distributions module from {module_path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


dist = _load_distributions_module()


def build_square(size: float) -> Polygon:
    return Polygon([(0.0, 0.0), (size, 0.0), (size, size), (0.0, size)])


def build_rings(max_radius: float, ring_width: float) -> list[tuple[float, float]]:
    rings = []
    inner = 0.0
    while inner < max_radius:
        outer = min(max_radius, inner + ring_width)
        rings.append((inner, outer))
        inner = outer
    return rings


def ring_areas(polygon: Polygon, center: tuple[float, float], rings):
    center_pt = Point(center)
    areas = []
    for inner, outer in rings:
        outer_area = polygon.intersection(center_pt.buffer(outer)).area
        inner_area = polygon.intersection(center_pt.buffer(inner)).area
        areas.append(max(0.0, outer_area - inner_area))
    return areas


def numbers_for_density(areas, distance_to_agents: float, factor: float):
    scale = factor / (distance_to_agents**2)
    return [max(0, int(a * scale)) for a in areas]


def run_once(args, polygon, center, rings, numbers, seed):
    t0 = time.perf_counter()
    points = dist.distribute_in_circles_by_number(
        polygon=polygon,
        distance_to_agents=args.distance_to_agents,
        distance_to_polygon=args.distance_to_polygon,
        center_point=center,
        circle_segment_radii=rings,
        numbers_of_agents=numbers,
        seed=seed,
        max_iterations=args.max_iterations,
    )
    return time.perf_counter() - t0, points


def feasible(args, polygon, center, rings, areas, factor, seed):
    numbers = numbers_for_density(areas, args.distance_to_agents, factor)
    try:
        elapsed, points = run_once(
            args, polygon, center, rings, numbers, seed=seed
        )
        return True, elapsed, len(points), numbers
    except dist.AgentNumberError:
        return False, 0.0, 0, numbers


def find_capacity_factor(args, polygon, center, rings, areas):
    low, high = 0.0, 1.0
    for _ in range(12):
        ok, _, _, _ = feasible(
            args, polygon, center, rings, areas, high, seed=args.seed
        )
        if not ok:
            break
        high *= 1.4
    for _ in range(args.search_steps):
        mid = (low + high) / 2.0
        ok, _, _, _ = feasible(
            args, polygon, center, rings, areas, mid, seed=args.seed
        )
        if ok:
            low = mid
        else:
            high = mid
    return low


def ring_index(point, center, rings):
    r = math.hypot(point[0] - center[0], point[1] - center[1])
    for i, (inner, outer) in enumerate(rings):
        if inner <= r <= outer:
            return i
    return -1


def min_pair_distance(points, cell_size):
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


def min_boundary_distance(points, polygon):
    boundary = polygon.boundary
    return min(boundary.distance(Point(x, y)) for x, y in points) if points else float("inf")


def quality_metrics(points, polygon, center, rings, areas, requested, d_agents, d_polygon):
    counts = [0] * len(rings)
    for p in points:
        idx = ring_index(p, center, rings)
        if idx >= 0:
            counts[idx] += 1

    densities = [counts[i] / areas[i] if areas[i] > 0 else 0.0 for i in range(len(rings))]
    dens_nonzero = [d for d in densities if d > 0]
    dens_mean = statistics.fmean(dens_nonzero) if dens_nonzero else 0.0
    dens_std = statistics.pstdev(dens_nonzero) if len(dens_nonzero) > 1 else 0.0
    dens_cv = dens_std / dens_mean if dens_mean > 0 else 0.0

    usable_area = polygon.buffer(-d_polygon).area
    global_density = len(points) / usable_area if usable_area > 0 else 0.0
    hex_upper = 2.0 / (math.sqrt(3.0) * d_agents * d_agents)
    fill_ratios = [counts[i] / requested[i] for i in range(len(rings)) if requested[i] > 0]

    return {
        "generated_points": len(points),
        "requested_points": int(sum(requested)),
        "global_density_per_m2": global_density,
        "density_vs_hex_upper_bound": global_density / hex_upper if hex_upper > 0 else 0.0,
        "min_pair_distance": min_pair_distance(points, d_agents),
        "min_boundary_distance": min_boundary_distance(points, polygon),
        "target_distance_to_agents": d_agents,
        "target_distance_to_polygon": d_polygon,
        "ring_density_cv": dens_cv,
        "ring_fill_ratio_mean": statistics.fmean(fill_ratios) if fill_ratios else 0.0,
    }


def parse_args():
    p = argparse.ArgumentParser(
        description="Benchmark distribute_in_circles_by_number on a 100x100 square."
    )
    p.add_argument("--size", type=float, default=100.0)
    p.add_argument("--ring-width", type=float, default=2.0)
    p.add_argument("--distance-to-agents", type=float, default=0.4)
    p.add_argument("--distance-to-polygon", type=float, default=0.2)
    p.add_argument("--max-iterations", type=int, default=10_000)
    p.add_argument("--seed", type=int, default=1234)
    p.add_argument("--search-steps", type=int, default=12)
    p.add_argument("--trials", type=int, default=5)
    p.add_argument("--json", action="store_true")
    return p.parse_args()


def main():
    args = parse_args()

    polygon = build_square(args.size)
    center = (args.size / 2.0, args.size / 2.0)
    max_radius = math.hypot(args.size / 2.0, args.size / 2.0)
    rings = build_rings(max_radius, args.ring_width)
    areas = ring_areas(polygon, center, rings)

    cap_factor = find_capacity_factor(args, polygon, center, rings, areas)
    requested = numbers_for_density(areas, args.distance_to_agents, cap_factor)

    runtimes = []
    sample_points = []
    for i in range(args.trials):
        rt, pts = run_once(
            args, polygon, center, rings, requested, seed=args.seed + i
        )
        runtimes.append(rt)
        if i == 0:
            sample_points = pts

    out = {
        "config": {
            "size": args.size,
            "ring_width": args.ring_width,
            "distance_to_agents": args.distance_to_agents,
            "distance_to_polygon": args.distance_to_polygon,
            "max_iterations": args.max_iterations,
            "trials": args.trials,
            "seed": args.seed,
            "ring_count": len(rings),
        },
        "capacity": {
            "density_factor_per_d2": cap_factor,
            "requested_total_points": int(sum(requested)),
        },
        "runtime_seconds": {
            "mean": statistics.fmean(runtimes),
            "median": statistics.median(runtimes),
            "min": min(runtimes),
            "max": max(runtimes),
            "per_trial": runtimes,
        },
        "quality": quality_metrics(
            sample_points,
            polygon,
            center,
            rings,
            areas,
            requested,
            args.distance_to_agents,
            args.distance_to_polygon,
        ),
    }

    if args.json:
        print(json.dumps(out, indent=2, sort_keys=True))
        return

    print("Benchmark result:")
    print(f"  requested points: {out['capacity']['requested_total_points']}")
    rt = out["runtime_seconds"]
    print(
        f"  runtime [s] mean/median/min/max: "
        f"{rt['mean']:.4f}/{rt['median']:.4f}/{rt['min']:.4f}/{rt['max']:.4f}"
    )
    q = out["quality"]
    print(
        f"  generated points: {q['generated_points']}, "
        f"ring fill ratio mean: {q['ring_fill_ratio_mean']:.4f}"
    )
    print(
        f"  global density [1/m2]: {q['global_density_per_m2']:.4f}, "
        f"vs hex upper bound: {q['density_vs_hex_upper_bound']:.4f}"
    )
    print(
        f"  min pair distance: {q['min_pair_distance']:.4f} "
        f"(target {q['target_distance_to_agents']:.4f})"
    )
    print(
        f"  min boundary distance: {q['min_boundary_distance']:.4f} "
        f"(target {q['target_distance_to_polygon']:.4f})"
    )
    print(f"  ring density CV (lower is better): {q['ring_density_cv']:.4f}")


if __name__ == "__main__":
    main()
