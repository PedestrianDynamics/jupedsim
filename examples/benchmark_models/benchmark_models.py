# SPDX-License-Identifier: LGPL-3.0-or-later
"""Benchmark suite: compare all operational models on three scenarios.

Scenarios: bidirectional corridor, bottleneck, crossing.
Models: CollisionFreeSpeedModelV2, AnticipationVelocityModel,
        SocialForceModel, WarpDriverModel.

Produces SQLite trajectory files and a 4-column plot of trajectories.
"""

from __future__ import annotations

import time
from pathlib import Path

import jupedsim as jps
import matplotlib.pyplot as plt
import numpy as np
import pedpy
import shapely
from pedpy import SpeedCalculation
from shapely import wkt

SCRIPT_DIR = Path(__file__).parent
seed = 123
dt = 0.05

MODELS = {
    # "CFSV2": (
    #     lambda: jps.CollisionFreeSpeedModelV2(
    #         strength_neighbor_repulsion=8.0,  # repulsive force magnitude [N]
    #         range_neighbor_repulsion=0.1,  # exponential decay length [m]
    #         strength_geometry_repulsion=5.0,  # wall repulsive force magnitude [N]
    #         range_geometry_repulsion=0.02,  # wall exponential decay length [m]
    #     ),
    #     jps.CollisionFreeSpeedModelV2AgentParameters,
    # ),
    # "AVM": (
    #     lambda: jps.AnticipationVelocityModel(
    #         strength_neighbor_repulsion=8.0,  # repulsive force magnitude [N]
    #         range_neighbor_repulsion=0.1,  # exponential decay length [m]
    #         wall_buffer_distance=0.02,  # min distance to walls [m]
    #         anticipation_time=1.0,  # look-ahead for neighbor prediction [s]
    #         reaction_time=0.3,  # velocity relaxation time [s]
    #     ),
    #     jps.AnticipationVelocityModelAgentParameters,
    # ),
    # "SocialForce": (
    #     lambda: jps.SocialForceModel(
    #         bodyForce=2000,  # contact body force strength [N]
    #         friction=0.08,  # sliding friction coefficient
    #     ),
    #     jps.SocialForceModelAgentParameters,
    # ),
    "WarpDriver": (
        lambda: jps.WarpDriverModel(
            time_horizon=2.0,  # collision prediction horizon [s]
            step_size=0.5,  # gradient descent step size (avoidance strength)
            sigma=0.3,  # Gaussian spread of intrinsic field
            time_uncertainty=0.5,  # temporal spread of collision field
            velocity_uncertainty_x=0.2,  # longitudinal speed uncertainty
            velocity_uncertainty_y=0.2,  # lateral speed uncertainty
            num_samples=20,  # trajectory sample points (cost ~ samples x neighbors)
            rng_seed=42,  # RNG seed for symmetry-breaking perturbations
        ),
        jps.WarpDriverModelAgentParameters,
    ),
}

_HAS_ORIENTATION = {
    jps.SocialForceModelAgentParameters,
    jps.WarpDriverModelAgentParameters,
}


def _make_agent(agent_cls, position, journey_id, stage_id, orientation=None):
    kwargs = dict(
        position=position,
        journey_id=journey_id,
        stage_id=stage_id,
        desired_speed=1.2,
        radius=0.15,
    )
    if agent_cls in _HAS_ORIENTATION and orientation is not None:
        kwargs["orientation"] = orientation
    return agent_cls(**kwargs)


# ── Scenario: Bidirectional corridor ─────────────────────────────────────


def _setup_bidirectional(model, writer, length=50.0, width=4.0):
    area = shapely.Polygon([(0, 0), (length, 0), (length, width), (0, width)])
    sim = jps.Simulation(
        model=model, geometry=area, dt=dt, trajectory_writer=writer
    )
    exit_left = shapely.Polygon([(0, 0), (1, 0), (1, width), (0, width)])
    exit_right = shapely.Polygon(
        [(length - 1, 0), (length, 0), (length, width), (length - 1, width)]
    )
    eid_l = sim.add_exit_stage(exit_left)
    eid_r = sim.add_exit_stage(exit_right)
    jid_r = sim.add_journey(jps.JourneyDescription([eid_r]))
    jid_l = sim.add_journey(jps.JourneyDescription([eid_l]))
    return sim, area, {"right": (jid_r, eid_r), "left": (jid_l, eid_l)}


def _add_bidirectional(sim, agent_cls, routes, n=50):
    jid_r, eid_r = routes["right"]
    jid_l, eid_l = routes["left"]
    spawn_left = shapely.Polygon([(2, 0.3), (8, 0.3), (8, 3.7), (2, 3.7)])
    spawn_right = shapely.Polygon([(42, 0.3), (48, 0.3), (48, 3.7), (42, 3.7)])
    streams = {"right": [], "left": []}
    pos_r = jps.distributions.distribute_by_number(
        polygon=spawn_left,
        number_of_agents=n,
        distance_to_agents=0.4,
        distance_to_polygon=0.2,
        seed=seed,
    )
    for pos in pos_r:
        aid = sim.add_agent(
            _make_agent(agent_cls, pos, jid_r, eid_r, orientation=(1, 0))
        )
        streams["right"].append(aid)
    pos_l = jps.distributions.distribute_by_number(
        polygon=spawn_right,
        number_of_agents=n,
        distance_to_agents=0.4,
        distance_to_polygon=0.2,
        seed=seed,
    )
    for pos in pos_l:
        aid = sim.add_agent(
            _make_agent(agent_cls, pos, jid_l, eid_l, orientation=(-1, 0))
        )
        streams["left"].append(aid)
    return streams


# ── Scenario: Bottleneck ────────────────────────────────────────────────


def _load_wkt(name):
    with open(SCRIPT_DIR / name, encoding="utf-8") as f:
        return wkt.load(f)


def _setup_bottleneck(model, writer):
    geometry = _load_wkt("bottleneck.wkt")
    sim = jps.Simulation(
        model=model, geometry=geometry, dt=dt, trajectory_writer=writer
    )
    exit_area = geometry.intersection(shapely.box(13, -5, 15, 5))
    eid = sim.add_exit_stage(exit_area)
    jid = sim.add_journey(jps.JourneyDescription([eid]))
    return sim, geometry, {"exit": (jid, eid)}


def _add_bottleneck(sim, agent_cls, routes, geometry, n=100):
    jid, eid = routes["exit"]
    spawn = geometry.intersection(shapely.box(-14, -4, -6, 4))
    positions = jps.distributions.distribute_by_number(
        polygon=spawn,
        number_of_agents=n,
        distance_to_agents=0.4,
        distance_to_polygon=0.2,
        seed=seed,
    )
    for pos in positions:
        sim.add_agent(_make_agent(agent_cls, pos, jid, eid, orientation=(1, 0)))
    return None  # single stream, no color splitting


# ── Scenario: Crossing ──────────────────────────────────────────────────


def _setup_crossing(model, writer):
    geometry = _load_wkt("crossing.wkt")
    sim = jps.Simulation(
        model=model, geometry=geometry, dt=dt, trajectory_writer=writer
    )
    exits = {
        "north": shapely.Polygon(
            [(-1.5, 6.0), (1.5, 6.0), (1.5, 6.5), (-1.5, 6.5)]
        ),
        "west": shapely.Polygon(
            [(-7.0, -1.5), (-6.0, -1.5), (-6.0, 1.5), (-7.0, 1.5)]
        ),
        "south": shapely.Polygon(
            [(-1.5, -6.5), (1.5, -6.5), (1.5, -6.0), (-1.5, -6.0)]
        ),
        "east": shapely.Polygon(
            [(6.0, -1.5), (7.0, -1.5), (7.0, 1.5), (6.0, 1.5)]
        ),
    }
    routes = {}
    for name, poly in exits.items():
        eid = sim.add_exit_stage(poly)
        jid = sim.add_journey(jps.JourneyDescription([eid]))
        routes[name] = (jid, eid)
    return sim, geometry, routes


def _add_crossing(sim, agent_cls, routes, n=15):
    sources = {
        "S→N": (
            shapely.Polygon(
                [(-1.5, -9.5), (1.5, -9.5), (1.5, -7.0), (-1.5, -7.0)]
            ),
            "north",
            (0, 1),
        ),
        "E→W": (
            shapely.Polygon([(8.0, -1.5), (9.5, -1.5), (9.5, 1.5), (8.0, 1.5)]),
            "west",
            (-1, 0),
        ),
        "N→S": (
            shapely.Polygon([(-1.5, 9.5), (1.5, 9.5), (1.5, 7.0), (-1.5, 7.0)]),
            "south",
            (0, -1),
        ),
        "W→E": (
            shapely.Polygon(
                [(-9.5, -1.5), (-8.0, -1.5), (-8.0, 1.5), (-9.5, 1.5)]
            ),
            "east",
            (1, 0),
        ),
    }
    streams = {}
    for stream_name, (spawn_poly, dest, orient) in sources.items():
        jid, eid = routes[dest]
        positions = jps.distributions.distribute_by_number(
            polygon=spawn_poly,
            number_of_agents=n,
            distance_to_agents=0.4,
            distance_to_polygon=0.2,
            seed=seed,
        )
        ids = []
        for pos in positions:
            aid = sim.add_agent(
                _make_agent(agent_cls, pos, jid, eid, orientation=orient)
            )
            ids.append(aid)
        streams[stream_name] = ids
    return streams


# ── Scenario registry ───────────────────────────────────────────────────

SCENARIOS = {
    "bidirectional": {
        "setup": _setup_bidirectional,
        "add": lambda sim, cls, routes, **kw: _add_bidirectional(
            sim, cls, routes, n=50
        ),
        "max_steps": 100000,
        "title": "Bidirectional (50+50)",
    },
    "bottleneck": {
        "setup": _setup_bottleneck,
        "add": None,  # needs geometry, handled below
        "max_steps": 100000,
        "title": "Bottleneck (200)",
    },
    "crossing": {
        "setup": _setup_crossing,
        "add": lambda sim, cls, routes, **kw: _add_crossing(
            sim, cls, routes, n=15
        ),
        "max_steps": 100000,
        "title": "Crossing (4x15)",
    },
}


# ── Run & plot ───────────────────────────────────────────────────────────


def run_single(scenario_name, model_name):
    scen = SCENARIOS[scenario_name]
    model_factory, agent_cls = MODELS[model_name]
    model = model_factory()

    sqlite_file = f"{scenario_name}_{model_name.lower()}.sqlite"
    writer = jps.SqliteTrajectoryWriter(output_file=sqlite_file)

    sim, geometry, routes = scen["setup"](model, writer)

    if scenario_name == "bottleneck":
        streams = _add_bottleneck(sim, agent_cls, routes, geometry, n=200)
    else:
        streams = scen["add"](sim, agent_cls, routes)

    initial = sim.agent_count()
    max_steps = scen["max_steps"]

    t0 = time.perf_counter()
    steps = 0
    for steps in range(1, max_steps + 1):
        sim.iterate()
        if sim.agent_count() == 0:
            break
    elapsed = time.perf_counter() - t0

    remaining = sim.agent_count()
    ms_step = elapsed / steps * 1000 if steps else 0
    print(
        f"  {model_name:<14s} | {initial:>4d} agents | {steps:>5d} steps "
        f"| {sim.elapsed_time():>6.1f}s sim | {elapsed:>7.3f}s exec time "
        f"| {ms_step:>6.2f} ms/step | {remaining:>3d} left"
    )

    return {
        "sqlite": sqlite_file,
        "geometry": geometry,
        "elapsed": elapsed,
        "steps": steps,
        "remaining": remaining,
        "streams": streams,
    }


def run_scenario(scenario_name):
    scen = SCENARIOS[scenario_name]
    print(f"\n{'=' * 78}")
    print(f"  {scen['title']}")
    print(f"{'=' * 78}")

    results = {}
    for model_name in MODELS:
        results[model_name] = run_single(scenario_name, model_name)
    return results


STREAM_COLORS = ["#1f77b4", "#d62728", "#2ca02c", "#ff7f0e"]


def plot_scenario(results, axes_row):
    """Plot trajectories for one scenario across a row of 4 axes."""
    geometry = next(iter(results.values()))["geometry"]
    walkable_area = pedpy.WalkableArea(geometry)

    for ax, (model_name, r) in zip(axes_row, results.items()):
        traj = pedpy.load_trajectory_from_jupedsim_sqlite(Path(r["sqlite"]))
        streams = r["streams"]

        if streams is None:
            pedpy.plot_trajectories(
                traj=traj,
                walkable_area=walkable_area,
                axes=ax,
                traj_color=STREAM_COLORS[0],
                traj_alpha=0.5,
                traj_width=0.1,
            )
        else:
            pedpy.plot_walkable_area(walkable_area=walkable_area, axes=ax)
            df = traj.data
            for i, (stream_name, agent_ids) in enumerate(streams.items()):
                id_set = set(agent_ids)
                stream_df = df[df["id"].isin(id_set)]
                if stream_df.empty:
                    continue
                stream_traj = pedpy.TrajectoryData(
                    data=stream_df, frame_rate=traj.frame_rate
                )
                color = STREAM_COLORS[i % len(STREAM_COLORS)]
                pedpy.plot_trajectories(
                    traj=stream_traj,
                    axes=ax,
                    traj_color=color,
                    traj_alpha=0.5,
                    traj_width=0.1,
                )

        remaining = r["remaining"]
        label = f"{model_name} ({r['steps'] * 0.01:.0f}s)"
        if remaining > 0:
            label += f" [{remaining} stuck]"
        ax.set_title(label, fontsize=9)
        ax.set_aspect("equal")


def compute_fluctuation(sqlite_file):
    """Compute velocity fluctuation time series from a trajectory file."""
    traj = pedpy.load_trajectory_from_jupedsim_sqlite(Path(sqlite_file))
    fps = traj.frame_rate
    dt = 1.0 / fps

    individual_velocity = pedpy.compute_individual_speed(
        traj_data=traj,
        frame_step=5,
        compute_velocity=True,
        speed_calculation=SpeedCalculation.BORDER_ADAPTIVE,
    )
    df = traj.data.merge(individual_velocity, on=["id", "frame"])

    # Average velocity vector per frame
    v_avg = df.groupby("frame")[["v_x", "v_y"]].mean()
    v_prev = v_avg.shift(1)
    diff = v_avg - v_prev
    fluctuation = np.sqrt(diff["v_x"] ** 2 + diff["v_y"] ** 2) / dt
    time_s = fluctuation.index / fps

    return time_s, fluctuation


def plot_fluctuation(results, axes_row):
    """Plot velocity fluctuation over time for one scenario across 4 axes."""
    model_colors = {
        name: STREAM_COLORS[i % len(STREAM_COLORS)]
        for i, name in enumerate(MODELS)
    }
    for ax, (model_name, r) in zip(axes_row, results.items()):
        time_s, fluctuation = compute_fluctuation(r["sqlite"])
        color = model_colors[model_name]
        ax.plot(time_s, fluctuation, color=color, alpha=0.7, linewidth=0.5)
        ax.set_xlabel("Time (s)", fontsize=8)
        ax.set_ylabel("$\\phi_v$ (m/s²)", fontsize=8)
        ax.set_title(model_name, fontsize=9)
        ax.set_ylim(bottom=0)
        ax.grid(True, linestyle="--", alpha=0.5)


def compute_order_parameter(sqlite_file):
    """Compute alignment parameter (order parameter) time series."""
    traj = pedpy.load_trajectory_from_jupedsim_sqlite(Path(sqlite_file))
    fps = traj.frame_rate

    individual_velocity = pedpy.compute_individual_speed(
        traj_data=traj,
        frame_step=5,
        compute_velocity=True,
        speed_calculation=SpeedCalculation.BORDER_ADAPTIVE,
    )
    df = traj.data.merge(individual_velocity, on=["id", "frame"])

    # Goal = last position of each agent
    goals = (
        df.groupby("id")[["x", "y"]]
        .last()
        .rename(columns={"x": "g_x", "y": "g_y"})
    )
    df = df.merge(goals, on="id")

    # Unit vector toward goal
    df["dx_g"] = df["g_x"] - df["x"]
    df["dy_g"] = df["g_y"] - df["y"]
    df["dist_g"] = np.sqrt(df["dx_g"] ** 2 + df["dy_g"] ** 2)
    df["u_x"] = df["dx_g"] / df["dist_g"]
    df["u_y"] = df["dy_g"] / df["dist_g"]

    # Speed and alignment
    df["speed"] = np.sqrt(df["v_x"] ** 2 + df["v_y"] ** 2)
    df["alignment"] = np.where(
        df["speed"] > 0,
        (df["v_x"] * df["u_x"] + df["v_y"] * df["u_y"]) / df["speed"],
        0,
    )

    alignment_over_time = df.groupby("frame")["alignment"].mean()
    time_s = alignment_over_time.index / fps

    return time_s, alignment_over_time.values


def plot_order_parameter(results, axes_row):
    """Plot alignment parameter over time for one scenario across 4 axes."""
    model_colors = {
        name: STREAM_COLORS[i % len(STREAM_COLORS)]
        for i, name in enumerate(MODELS)
    }
    for ax, (model_name, r) in zip(axes_row, results.items()):
        time_s, alignment = compute_order_parameter(r["sqlite"])
        color = model_colors[model_name]
        ax.plot(time_s, alignment, color=color, alpha=0.7, linewidth=0.5)
        ax.set_xlabel("Time (s)", fontsize=8)
        ax.set_ylabel("$\\phi_d$", fontsize=8)
        ax.set_title(model_name, fontsize=9)
        ax.set_ylim([0, 1.1])
        ax.axhline(y=1, color="r", linestyle="--", alpha=0.3)
        ax.grid(True, linestyle="--", alpha=0.5)


def compute_min_distance(sqlite_file):
    """Compute minimum pairwise agent distance per frame."""
    traj = pedpy.load_trajectory_from_jupedsim_sqlite(Path(sqlite_file))
    fps = traj.frame_rate
    df = traj.data

    frames = sorted(df["frame"].unique())
    time_s = []
    min_dists = []
    for frame in frames:
        fdf = df[df["frame"] == frame]
        if len(fdf) < 2:
            continue
        xs = fdf["x"].values
        ys = fdf["y"].values
        # Pairwise distances via broadcasting
        dx = xs[:, None] - xs[None, :]
        dy = ys[:, None] - ys[None, :]
        dists = np.sqrt(dx * dx + dy * dy)
        np.fill_diagonal(dists, np.inf)
        min_dists.append(dists.min())
        time_s.append(frame / fps)

    return np.array(time_s), np.array(min_dists)


def plot_min_distance(results, axes_row):
    """Plot minimum pairwise distance over time for one scenario."""
    model_colors = {
        name: STREAM_COLORS[i % len(STREAM_COLORS)]
        for i, name in enumerate(MODELS)
    }
    for ax, (model_name, r) in zip(axes_row, results.items()):
        time_s, min_dists = compute_min_distance(r["sqlite"])
        color = model_colors[model_name]
        ax.plot(time_s, min_dists, color=color, alpha=0.7, linewidth=0.5)
        ax.axhline(y=0.3, color="r", linestyle="--", alpha=0.3, label="2r")
        ax.set_xlabel("Time (s)", fontsize=8)
        ax.set_ylabel("$d_{min}$ (m)", fontsize=8)
        ax.set_title(model_name, fontsize=9)
        ax.set_ylim(bottom=0)
        ax.legend(fontsize=7)
        ax.grid(True, linestyle="--", alpha=0.5)


def main():
    all_results = {}
    for scenario_name in SCENARIOS:
        all_results[scenario_name] = run_scenario(scenario_name)

    print(f"\n{'=' * 78}")
    print("  Summary")
    print(f"{'=' * 78}")
    for scenario_name, results in all_results.items():
        scen = SCENARIOS[scenario_name]
        print(f"\n  {scen['title']}:")
        for model_name, r in results.items():
            ms = r["elapsed"] / r["steps"] * 1000 if r["steps"] else 0
            print(
                f"    {model_name:<14s}: {r['elapsed']:>7.3f}s "
                f"({r['steps']} steps, {ms:.2f} ms/step, "
                f"{r['remaining']} remaining)"
            )

    n_scenarios = len(SCENARIOS)
    n_models = len(MODELS)
    # 4 rows per scenario: trajectories + fluctuation + order parameter + min distance
    n_rows = n_scenarios * 4
    fig, axes = plt.subplots(
        nrows=n_rows,
        ncols=max(n_models, 1),
        figsize=(4 * max(n_models, 1), 3 * n_rows),
        gridspec_kw={"height_ratios": [3, 1, 1, 1] * n_scenarios},
        squeeze=False,
    )

    for i, (scenario_name, results) in enumerate(all_results.items()):
        scen = SCENARIOS[scenario_name]
        traj_row = i * 4
        fluct_row = i * 4 + 1
        order_row = i * 4 + 2
        mindist_row = i * 4 + 3
        plot_scenario(results, axes[traj_row])
        axes[traj_row][0].set_ylabel(scen["title"], fontsize=10)
        plot_fluctuation(results, axes[fluct_row])
        axes[fluct_row][0].set_ylabel("$\\phi_v$", fontsize=9)
        plot_order_parameter(results, axes[order_row])
        axes[order_row][0].set_ylabel("$\\phi_d$", fontsize=9)
        plot_min_distance(results, axes[mindist_row])
        axes[mindist_row][0].set_ylabel("$d_{min}$ (m)", fontsize=9)

    fig.suptitle("Model Comparison", fontsize=14, y=1.01)
    fig.tight_layout()
    fig.savefig("benchmark_models.pdf", bbox_inches="tight")
    print("\nPlot saved to benchmark_models.pdf")


if __name__ == "__main__":
    main()
