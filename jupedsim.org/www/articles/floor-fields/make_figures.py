# /// script
# requires-python = ">=3.11"
# dependencies = ["matplotlib", "numpy", "pillow", "seaborn", "shapely"]
# ///
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Figures of the article floor-fields.md, written next to this script.

Runs the congestion example of the article with and without the speed-field
update, prints the numbers the article quotes, and writes
  travel-time.png   the field of "Looking at the fields"
  congestion.png    why agents switch exit areas
  congestion.gif    both runs side by side
jupedsim is not on PyPI in this version; put a build on the path, e.g.
    PYTHONPATH=build/stage uv run jupedsim.org/www/articles/floor-fields/make_figures.py
"""

from pathlib import Path

import jupedsim as jps
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import shapely
from matplotlib.animation import FuncAnimation, PillowWriter
from matplotlib.patches import Rectangle

RECORD_EVERY = 20  # iterations, 0.2 s at the default dt
SNAPSHOT_IT = 800  # 8 s: the update at 8 s has just been applied


def weidmann(density):
    """Walking speed in m/s at a density in persons/m² (Weidmann 1993)."""
    rho = np.maximum(density, 1e-3)
    return np.clip(
        1.34 * (1 - np.exp(-1.913 * (1 / rho - 1 / 5.4))), 0.01, 1.34
    )


def run(with_update):
    builder = jps.GeometryBuilder()
    room = builder.add_region(polygon=shapely.box(0, 0, 20, 10), height=0.0)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=builder.build()
    )
    exit_ = simulation.add_target(
        jps.Exit(
            [
                (shapely.box(19, 8, 20, 10), room),
                (shapely.box(19, 0, 20, 2), room),
            ]
        )
    )
    route = simulation.add_route(
        [jps.FixedTransition(jps.AGENT_LOCATION, exit_)]
    )
    for x in np.arange(10.5, 16.5, 0.5):
        for y in np.arange(6.25, 9.8, 0.5):
            simulation.add_agent(
                room.at(x, y),
                route=route,
                state=jps.CollisionFreeSpeedModelState(),
            )

    default = simulation.default_speed_fields()[room]
    cx, cy = default.cell_centers

    def update_speed_field(radius=1.0):
        pos = np.array([agent.location.xy for agent in simulation.agents()])
        if len(pos) == 0:
            return
        dist2 = (cx[..., None] - pos[:, 0]) ** 2 + (
            cy[..., None] - pos[:, 1]
        ) ** 2
        density = (dist2 < radius**2).sum(axis=-1) / (np.pi * radius**2)
        factor = weidmann(density) / weidmann(0.0)
        simulation.set_default_speed_field(
            room, np.maximum(default.values * factor, 0.01)
        )

    rows = []  # (time, agent id, x, y)
    last_y = {}
    snapshot = {}
    speeds = []  # speed field after every update, one per 2 s
    while simulation.agent_count() > 0:
        it = simulation.iteration_count()
        if with_update and it % 200 == 0:
            update_speed_field()
            speeds.append(
                np.asarray(simulation.default_speed_fields()[room].values)
            )
        if it == SNAPSHOT_IT:
            grid = simulation.travel_times(exit_)[room]
            speed = simulation.default_speed_fields()[room]
            snapshot = dict(
                travel=np.asarray(grid.values),
                speed=np.asarray(speed.values),
                extent=np.asarray(grid.extent),
            )
        if it % RECORD_EVERY == 0:
            for agent in simulation.agents():
                rows.append((it * 0.01, agent.id, *agent.location.xy))
        for agent in simulation.agents():
            last_y[agent.id] = agent.location.y
        simulation.iterate()
    t_end = simulation.iteration_count() * 0.01
    south = sum(1 for y in last_y.values() if y < 5)
    return np.array(rows), last_y, snapshot, t_end, south, speeds


def run_both():
    """Both runs, in the form the figure functions read."""
    d = {}
    for name, flag in [("static", False), ("updated", True)]:
        rows, last_y, snap, t_end, south, speeds = run(flag)
        print(
            f"{name}: north-east {len(last_y) - south}, south-east {south}, "
            f"empty after {t_end:.1f} s"
        )
        ids = np.array(sorted(last_y))
        d[f"{name}_rows"] = rows
        d[f"{name}_ids"] = ids
        d[f"{name}_south"] = np.array([last_y[i] < 5 for i in ids])
        d[f"{name}_tend"] = t_end
        if speeds:
            d[f"{name}_speeds"] = np.array(speeds)
        for k, v in snap.items():
            d[f"{name}_{k}"] = v
    return d


NORTH_C, SOUTH_C = "#4575b4", "#d73027"
TT_CMAP = sns.cubehelix_palette(rot=-0.25, light=0.95, dark=0.15, as_cmap=True)
SP_CMAP = sns.cubehelix_palette(
    start=2.8, rot=0.1, light=0.95, dark=0.25, reverse=True, as_cmap=True
)


def style_room(ax, areas=True):
    ax.set_xlim(0, 20)
    ax.set_ylim(0, 10)
    ax.set_aspect("equal")
    ax.grid(False)
    ax.set_xticks([0, 5, 10, 15, 20])
    ax.set_yticks([0, 5, 10])
    ax.tick_params(axis="both", which="both", length=0, labelcolor="dimgrey")
    ax.add_patch(Rectangle((0, 0), 20, 10, fill=False, ec="dimgrey", lw=1.0))
    if not areas:
        return
    for y0, c, name in [(8, NORTH_C, "NE"), (0, SOUTH_C, "SE")]:
        ax.add_patch(Rectangle((19, y0), 1, 2, fc=c, ec=c, alpha=0.35, lw=1.2))
        ax.text(
            18.8,
            y0 + 1,
            name,
            ha="right",
            va="center",
            fontsize=9,
            color="dimgrey",
        )


def agents_at(d, run, t):
    rows = d[f"{run}_rows"]
    sel = np.isclose(rows[:, 0], t)
    ids, xy = rows[sel, 1].astype(int), rows[sel, 2:]
    south_of = dict(zip(d[f"{run}_ids"], d[f"{run}_south"]))
    south = np.array([south_of[i] for i in ids], dtype=bool)
    return xy, south


def draw_agents(ax, xy, south, s=22):
    ax.scatter(
        *xy[~south].T, s=s, c=NORTH_C, marker="o", ec="white", lw=0.5, zorder=5
    )
    ax.scatter(
        *xy[south].T,
        s=s * 1.3,
        c=SOUTH_C,
        marker="^",
        ec="white",
        lw=0.5,
        zorder=5,
    )


def travel_time_figure(out):
    builder = jps.GeometryBuilder()
    room = builder.add_region(polygon=shapely.box(0, 0, 20, 10), height=0.0)
    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(), geometry=builder.build()
    )
    north = simulation.add_target(
        jps.Exit([(shapely.box(19, 8, 20, 10), room)])
    )
    simulation.add_target(jps.Exit([(shapely.box(19, 0, 20, 2), room)]))
    grid = simulation.travel_times(north)[room]
    values = np.ma.masked_invalid(grid.values)
    dirs = grid.directions
    cx = grid.origin[0] + (np.arange(grid.width) + 0.5) * grid.cell_size
    cy = grid.origin[1] + (np.arange(grid.height) + 0.5) * grid.cell_size

    fig, ax = plt.subplots(figsize=(9, 4.6))
    im = ax.imshow(values, origin="lower", extent=grid.extent, cmap=TT_CMAP)
    cs = ax.contour(
        cx,
        cy,
        values,
        levels=np.arange(2, 24, 2),
        colors="white",
        linewidths=0.7,
        alpha=0.8,
    )
    ax.clabel(cs, fmt="%d s", fontsize=7, colors="white")
    k = 5  # one arrow per 1 m
    qx, qy = np.meshgrid(cx[k // 2 :: k], cy[k // 2 :: k])
    qu, qv = (
        dirs[k // 2 :: k, k // 2 :: k, 0],
        dirs[k // 2 :: k, k // 2 :: k, 1],
    )
    ax.quiver(
        qx, qy, qu, qv, color="dimgrey", scale=32, width=0.0028, headwidth=4
    )
    style_room(ax, areas=False)
    ax.add_patch(Rectangle((19, 8), 1, 2, fc=NORTH_C, ec=NORTH_C, alpha=0.6))
    ax.text(
        18.7,
        9.2,
        "north exit",
        ha="right",
        va="center",
        fontsize=9,
        color="dimgrey",
        bbox=dict(boxstyle="round,pad=0.25", fc="white", ec="none", alpha=0.85),
    )
    ax.annotate(
        "slower within 0.5 m of a wall:\ncontours bend, arrows turn away",
        xy=(9.5, 0.3),
        xytext=(4.0, 2.2),
        fontsize=8.5,
        color="dimgrey",
        arrowprops=dict(arrowstyle="-", color="dimgrey", lw=0.8),
        bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="none", alpha=0.85),
    )
    cb = fig.colorbar(im, ax=ax, shrink=0.85, pad=0.02)
    cb.set_label("travel time to the north exit [s]", color="dimgrey")
    cb.outline.set_visible(False)
    cb.ax.tick_params(length=0, labelcolor="dimgrey")
    ax.set_xlabel("x [m]", color="dimgrey")
    ax.set_ylabel("y [m]", color="dimgrey")
    sns.despine(left=True, bottom=True)
    fig.savefig(out / "travel-time.png", dpi=150, bbox_inches="tight")
    plt.close(fig)


def congestion_figure(d, out):
    t_snap = 8.0
    ext = d["static_extent"]
    vmax = float(
        np.nanmax(
            np.where(
                np.isfinite(d["updated_travel"]), d["updated_travel"], np.nan
            )
        )
    )
    fig, axes = plt.subplots(
        2, 2, figsize=(12, 6.4), gridspec_kw=dict(hspace=0.22, wspace=0.28)
    )
    (a, b), (c, e) = axes

    for ax, run, title in [
        (a, "static", "(a) without update"),
        (b, "updated", "(b) with update"),
    ]:
        tt = np.ma.masked_invalid(d[f"{run}_travel"])
        im_tt = ax.imshow(
            tt, origin="lower", extent=ext, cmap=TT_CMAP, vmin=0, vmax=vmax
        )
        n, m = tt.shape
        cx = np.linspace(ext[0], ext[1], m + 1)[:-1] + (ext[1] - ext[0]) / m / 2
        cy = np.linspace(ext[2], ext[3], n + 1)[:-1] + (ext[3] - ext[2]) / n / 2
        ax.contour(
            cx,
            cy,
            tt,
            levels=np.arange(2, 40, 2),
            colors="white",
            linewidths=0.6,
            alpha=0.8,
        )
        style_room(ax)
        draw_agents(ax, *agents_at(d, run, t_snap))
        ax.set_title(
            f"{title}: travel time to the exit at {t_snap:.0f} s",
            fontsize=10,
            color="dimgrey",
            loc="left",
        )
    b.annotate(
        "queue slows the field,\nthe SE area is now quicker",
        xy=(12.8, 5.6),
        xytext=(1.0, 1.4),
        fontsize=8.5,
        color="dimgrey",
        arrowprops=dict(arrowstyle="-", color="dimgrey", lw=0.8),
        bbox=dict(boxstyle="round,pad=0.25", fc="white", ec="none", alpha=0.8),
    )
    cb = fig.colorbar(im_tt, ax=[a, b], shrink=0.9, pad=0.015)
    cb.set_label("travel time [s]", color="dimgrey")
    cb.outline.set_visible(False)
    cb.ax.tick_params(length=0, labelcolor="dimgrey")

    im_sp = c.imshow(
        np.ma.masked_invalid(d["updated_speed"]),
        origin="lower",
        extent=ext,
        cmap=SP_CMAP,
        vmin=0,
        vmax=1.0,
    )
    style_room(c)
    draw_agents(c, *agents_at(d, "updated", t_snap))
    c.set_title(
        f"(c) speed field used for (b) at {t_snap:.0f} s",
        fontsize=10,
        color="dimgrey",
        loc="left",
    )
    cb2 = fig.colorbar(im_sp, ax=c, shrink=0.9, pad=0.03)
    cb2.set_label("speed field [m/s]", color="dimgrey")
    cb2.outline.set_visible(False)
    cb2.ax.tick_params(length=0, labelcolor="dimgrey")

    for run, color, ls, label in [
        ("static", "dimgrey", "--", "without update"),
        ("updated", NORTH_C, "-", "with update"),
    ]:
        rows = d[f"{run}_rows"]
        times, counts = np.unique(rows[:, 0], return_counts=True)
        t_end = float(d[f"{run}_tend"])
        times, counts = np.append(times, t_end), np.append(counts, 0)
        e.step(
            times,
            counts,
            where="post",
            color=color,
            ls=ls,
            lw=1.8,
            label=f"{label}, empty at {t_end:.1f} s",
        )
        e.plot(t_end, 0, "o", color=color, ms=5, clip_on=False, zorder=6)
    e.set_xlim(0, 24)
    e.set_ylim(0, 100)
    e.set_xlabel("time [s]", color="dimgrey")
    e.set_ylabel("agents in the room", color="dimgrey")
    e.set_title(
        "(d) agents in the room", fontsize=10, color="dimgrey", loc="left"
    )
    e.grid(alpha=0.7, linewidth=1, axis="y")
    e.tick_params(axis="both", which="both", length=0, labelcolor="dimgrey")
    handles = [
        plt.Line2D(
            [], [], marker="o", ls="", color=NORTH_C, label="leaves by NE"
        ),
        plt.Line2D(
            [], [], marker="^", ls="", color=SOUTH_C, label="leaves by SE"
        ),
    ]
    leg1 = e.legend(
        loc="upper right",
        frameon=True,
        facecolor="white",
        framealpha=0.8,
        edgecolor="lightgrey",
        labelcolor="dimgrey",
        fontsize=8.5,
    )
    e.add_artist(leg1)
    e.legend(
        handles=handles,
        loc="center right",
        frameon=True,
        facecolor="white",
        framealpha=0.8,
        edgecolor="lightgrey",
        labelcolor="dimgrey",
        fontsize=8.5,
    )
    sns.despine(left=True, bottom=True)
    fig.savefig(out / "congestion.png", dpi=150, bbox_inches="tight")
    plt.close(fig)


def congestion_gif(d, out):
    ext = d["static_extent"]
    speeds = d["updated_speeds"]
    default = d["static_speed"]
    t_max = float(max(d["static_tend"], d["updated_tend"]))
    times = np.round(np.arange(0, t_max + 0.2, 0.2), 2)
    fig, axes = plt.subplots(
        2, 1, figsize=(6.4, 6.4), gridspec_kw=dict(hspace=0.22)
    )
    fig.subplots_adjust(left=0.07, right=0.98, top=0.95, bottom=0.1)
    bgs, scat = [], []
    for ax, run, title in [
        (axes[0], "static", "without update"),
        (axes[1], "updated", "with update every 2 s"),
    ]:
        bgs.append(
            ax.imshow(
                np.ma.masked_invalid(default),
                origin="lower",
                extent=ext,
                cmap=SP_CMAP,
                vmin=0,
                vmax=1.0,
            )
        )
        style_room(ax)
        n = ax.scatter(
            [], [], s=20, c=NORTH_C, marker="o", ec="white", lw=0.4, zorder=5
        )
        s = ax.scatter(
            [], [], s=26, c=SOUTH_C, marker="^", ec="white", lw=0.4, zorder=5
        )
        txt = ax.text(
            0.3,
            0.4,
            "",
            fontsize=9,
            color="dimgrey",
            bbox=dict(fc="white", ec="none", alpha=0.8),
        )
        ax.set_title(title, fontsize=10, color="dimgrey", loc="left")
        scat.append((run, n, s, txt))
    fig.text(
        0.5,
        0.025,
        "background: speed field [m/s], dark = slow     ●  leaves by NE     ▲  leaves by SE",
        ha="center",
        fontsize=8.5,
        color="dimgrey",
    )
    sns.despine(left=True, bottom=True)

    def frame(i):
        t = times[i]
        bgs[1].set_data(
            np.ma.masked_invalid(speeds[min(int(t // 2), len(speeds) - 1)])
        )
        artists = [bgs[1]]
        for run, n, s, txt in scat:
            xy, south = agents_at(d, run, t)
            n.set_offsets(xy[~south] if len(xy) else np.empty((0, 2)))
            s.set_offsets(xy[south] if len(xy) else np.empty((0, 2)))
            txt.set_text(f"t = {t:4.1f} s   in room: {len(xy):2d}")
            artists += [n, s, txt]
        return artists

    anim = FuncAnimation(fig, frame, frames=len(times), blit=False)
    anim.save(out / "congestion.gif", writer=PillowWriter(fps=10), dpi=100)
    plt.close(fig)


def main():
    out = Path(__file__).parent
    sns.set_theme(font_scale=1.0, style="whitegrid", font="DejaVu Sans")
    d = run_both()
    travel_time_figure(out)
    congestion_figure(d, out)
    congestion_gif(d, out)


if __name__ == "__main__":
    main()
