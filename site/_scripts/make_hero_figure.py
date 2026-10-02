# SPDX-License-Identifier: LGPL-3.0-or-later
"""Generate the landing page's hero figure from a JuPedSim run.

A 10 x 5.8 m room (the view box's aspect ratio) with a wall at x = 7 that
has a 1 m wide gap; about 40 agents (collision-free speed model) walk to an
exit stage behind the wall.
Every agent's position is recorded every 10 iterations until 12 agents have
passed the wall (or 3000 iterations). 12 trajectories whose start positions
are spread over the room are simplified (a point is kept when it is more
than 0.15 m from the last kept one; the last point is always kept) and
written as an SVG in view coordinates rounded to 1 decimal.

The SVG carries no colours, only classes styled by the theme's site.css:
``hf-frame`` (room), ``hf-wall``, ``hf-traj`` (trajectories), ``hf-agent``
(each trajectory's last position) and one ``hf-alert`` (the agent nearest
the gap centre at the final frame). Same seed and code give a
byte-identical file. The build never runs this script; run it by hand
(with the build's environment sourced) and commit the output:

    python site/_scripts/make_hero_figure.py
"""

import argparse
import math
import pathlib
import sys

import jupedsim as jps
import shapely

REPO = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_OUT = REPO / "docs/_theme/jupedsim_book/hero-figure.svg"
MAX_BYTES = 15 * 1024

ROOM_W, ROOM_H = 10.0, 5.8
WALL_X0, WALL_X1 = 6.9, 7.1
GAP_Y0, GAP_Y1 = ROOM_H / 2 - 0.5, ROOM_H / 2 + 0.5
GAP_CENTRE = ((WALL_X0 + WALL_X1) / 2, (GAP_Y0 + GAP_Y1) / 2)
EXIT = shapely.box(9.3, ROOM_H / 2 - 1, 9.8, ROOM_H / 2 + 1)
START = shapely.box(0.4, 0.4, 6.4, ROOM_H - 0.4)
N_AGENTS = 40
N_PASSED = 12
N_TRAJ = 12
MAX_ITER = 3000
RECORD_EVERY = 10
MIN_STEP = 0.15

# The room fills the view box; the margin keeps the frame's stroke inside.
VIEW_W, VIEW_H, MARGIN = 480, 280, 0.5
SCALE = min((VIEW_W - 2 * MARGIN) / ROOM_W, (VIEW_H - 2 * MARGIN) / ROOM_H)
OFF_X = (VIEW_W - ROOM_W * SCALE) / 2
OFF_Y = (VIEW_H - ROOM_H * SCALE) / 2
LABEL = (
    "Simulated trajectories of pedestrians leaving a room through a bottleneck"
)


def walls():
    """The wall at x = 7 as two rectangles above and below the gap."""
    return [
        shapely.box(WALL_X0, 0.0, WALL_X1, GAP_Y0),
        shapely.box(WALL_X0, GAP_Y1, WALL_X1, ROOM_H),
    ]


def simulate(seed):
    """Run the scenario; return ``({agent: [(x, y), ...]}, final_positions)``.

    Positions are recorded at iteration 0 and every ``RECORD_EVERY``
    iterations; ``final_positions`` are those of the agents still in the
    simulation at the last recorded frame.
    """
    geometry = shapely.box(0.0, 0.0, ROOM_W, ROOM_H)
    for wall in walls():
        geometry = geometry.difference(wall)
    sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=geometry)
    exit_id = sim.add_exit_stage(EXIT)
    journey_id = sim.add_journey(jps.JourneyDescription([exit_id]))
    starts = jps.distribute_by_number(
        polygon=START,
        number_of_agents=N_AGENTS,
        distance_to_agents=0.5,
        distance_to_polygon=0.3,
        seed=seed,
    )
    for pos in starts:
        sim.add_agent(
            journey_id=journey_id,
            stage_id=exit_id,
            position=pos,
            state=jps.CollisionFreeSpeedModelState(),
        )

    tracks = {}

    def record():
        frame = {a.id: tuple(a.position) for a in sim.agents()}
        for agent_id, pos in frame.items():
            tracks.setdefault(agent_id, []).append(pos)
        return frame

    frame = record()
    while sim.iteration_count() < MAX_ITER:
        passed = sum(1 for t in tracks.values() if t[-1][0] > WALL_X1)
        if passed >= N_PASSED:
            break
        sim.iterate(RECORD_EVERY)
        frame = record()
    return tracks, frame


def spread(tracks, n):
    """Pick ``n`` agents whose start positions are spread over the room.

    Farthest-point sampling on the start positions, beginning with the
    agent that starts nearest the start region's centre; ties go to the
    lower id, so the choice is deterministic.
    """
    c = START.centroid
    ids = sorted(tracks)
    first = min(ids, key=lambda i: (math.dist(tracks[i][0], (c.x, c.y)), i))
    chosen = [first]
    while len(chosen) < min(n, len(ids)):
        best = max(
            (i for i in ids if i not in chosen),
            key=lambda i: (
                min(math.dist(tracks[i][0], tracks[j][0]) for j in chosen),
                -i,
            ),
        )
        chosen.append(best)
    return sorted(chosen)


def simplify(points):
    """Keep points more than ``MIN_STEP`` from the last kept one, plus the last."""
    kept = [points[0]]
    for p in points[1:]:
        if math.dist(p, kept[-1]) > MIN_STEP:
            kept.append(p)
    if kept[-1] != points[-1]:
        kept.append(points[-1])
    return kept


def view(x, y):
    """World metres -> SVG view coordinates (y up -> y down)."""
    return OFF_X + x * SCALE, OFF_Y + (ROOM_H - y) * SCALE


def num(v):
    """Round to 1 decimal; drop a trailing ``.0``."""
    s = f"{round(v, 1):.1f}"
    s = s.removesuffix(".0")
    return "0" if s == "-0" else s


def rect(cls, geom):
    x0, y0, x1, y1 = geom.bounds
    vx, vy = view(x0, y1)
    return (
        f'<rect class="{cls}" x="{num(vx)}" y="{num(vy)}"'
        f' width="{num((x1 - x0) * SCALE)}" height="{num((y1 - y0) * SCALE)}"/>'
    )


def path(points):
    coords = [view(x, y) for x, y in simplify(points)]
    d = "M" + "L".join(f"{num(x)} {num(y)}" for x, y in coords)
    return f'<path class="hf-traj" d="{d}"/>'


def circle(cls, pos, r):
    x, y = view(*pos)
    return f'<circle class="{cls}" cx="{num(x)}" cy="{num(y)}" r="{r}"/>'


def render(seed):
    tracks, final = simulate(seed)
    chosen = spread(tracks, N_TRAJ)
    alert = min(
        final, key=lambda i: (math.dist(final[i], GAP_CENTRE), i), default=None
    )
    if alert is None:
        sys.exit("make_hero_figure: no agent left at the final frame")
    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {VIEW_W} {VIEW_H}"'
        f' role="img" aria-label="{LABEL}">',
        f"<!-- Generated by site/_scripts/make_hero_figure.py --seed {seed};"
        " do not edit. -->",
        rect("hf-frame", shapely.box(0.0, 0.0, ROOM_W, ROOM_H)),
    ]
    lines += [path(tracks[i]) for i in chosen]
    lines += [rect("hf-wall", w) for w in walls()]
    lines += [
        circle("hf-agent", tracks[i][-1], 4) for i in chosen if i != alert
    ]
    lines.append(circle("hf-alert", final[alert], 4.5))
    lines.append("</svg>")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument(
        "--out",
        type=pathlib.Path,
        default=DEFAULT_OUT,
        help="SVG to write (default: the theme's hero-figure.svg)",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=1,
        help="seed for the start positions (default: %(default)s)",
    )
    args = parser.parse_args()
    svg = render(args.seed).encode("utf-8")
    print(f"{args.out}: {len(svg)} bytes (limit {MAX_BYTES})")
    if len(svg) > MAX_BYTES:
        sys.exit(f"make_hero_figure: {len(svg)} bytes exceed {MAX_BYTES}")
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(svg)


if __name__ == "__main__":
    main()
