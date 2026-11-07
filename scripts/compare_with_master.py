#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Record and check the observable behaviour of the example scripts.

``record`` runs every script listed in ``route_api_baselines/SCENARIOS.txt``
and stores per-simulation metrics as JSON. ``check`` runs them again and
fails on any difference not listed in ``route_api_baselines/EXEMPT.txt``.

Each script runs in a fresh temporary copy of its own directory, so relative
data paths resolve and output files do not touch the repository. Notebooks
are converted by extracting their code cells; lines starting with ``%`` or
``!`` are dropped. Unset random seeds are pinned so runs are reproducible. ``--selftest`` runs a small built-in scenario twice.

Usage:
    scripts/compare_with_master.py record [script ...]
    scripts/compare_with_master.py check [script ...]
    scripts/compare_with_master.py --selftest
"""

import argparse
import importlib
import json
import os
import platform
import random
import re
import runpy
import shutil
import subprocess
import sys
import tempfile
import time
import types
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BASE_DIR = Path(__file__).resolve().parent / "route_api_baselines"
# Where the build tree puts jupedsim and the example helpers.
BUILD_PATHS = (
    ROOT / "build" / "stage",
    ROOT / "python_modules" / "jupedsim_examples",
)
SAMPLE_EVERY = 100
DEFAULT_SEED = 1
METRICS = (
    "agent_count",
    "exit_counts",
    "assignments",
    "positions",
    "end_time",
)


class _Api1x:
    """Stage creation hooks and agent target lookup for the 1.x API."""

    name = "1.x"

    def __init__(self, jps):
        self.jps = jps
        self.exit_order = {}  # simulation -> [stage ids]
        self.stage_order = {}  # simulation -> [stage ids]
        self.direct = {}  # simulation -> {stage ids}

    @staticmethod
    def detect(jps):
        return hasattr(jps, "JourneyDescription")

    def install(self, on_stage):
        sim_cls = self.jps.Simulation
        for method, kind in (
            ("add_exit_stage", "exit"),
            ("add_waypoint_stage", "waypoint"),
            ("add_direct_steering_stage", "direct"),
        ):
            setattr(
                sim_cls,
                method,
                self._hook(getattr(sim_cls, method), kind, on_stage),
            )

    @staticmethod
    def _hook(original, kind, on_stage):
        def wrapper(sim, *args, **kwargs):
            stage = original(sim, *args, **kwargs)
            on_stage(sim, kind, stage)
            return stage

        return wrapper

    def target(self, agent):
        """Id of the stage the agent currently targets."""
        return agent.stage_id

    @staticmethod
    def removed_exit(rec, removed):
        """Exit stage id the removed agent was targeting, or None."""
        return rec.targets.get(removed)

    @staticmethod
    def xy(agent):
        return agent.position[0], agent.position[1]


class _Api2x:
    """Stage creation hooks and agent target lookup for the 2.0 API.

    Stages are handles. Direct steering has no stage: an agent without a
    stage counts as steering directly.
    """

    name = "2.0"
    DIRECT = "direct"

    def __init__(self, jps):
        self.jps = jps

    @staticmethod
    def detect(jps):
        return hasattr(jps, "Route")

    def install(self, on_stage):
        sim_cls = self.jps.Simulation
        for method, kind in (
            ("add_exit", "exit"),
            ("add_waypoint", "waypoint"),
        ):
            setattr(
                sim_cls,
                method,
                _Api1x._hook(getattr(sim_cls, method), kind, on_stage),
            )

    def target(self, agent):
        """Handle of the stage the agent targets, DIRECT without one."""
        stage = agent.stage
        return self.DIRECT if stage is None else stage

    @staticmethod
    def removed_exit(rec, removed):
        """Exit handle of the removal record, or None."""
        return removed.exit

    @staticmethod
    def xy(agent):
        return agent.position.xy


class _SimRecord:
    def __init__(self):
        self.stages = []  # stage handles in creation order
        self.exits = []  # exit handles in creation order
        self.direct = set()
        self.agent_count = []  # [iteration, count] at changes
        self.exit_counts = {}
        self.assignments = {}
        self.positions = {}
        self.end_time = 0.0
        self.targets = {}  # agent id -> target before the iteration


class _Recorder:
    def __init__(self, jps, api):
        self.jps = jps
        self.api = api
        self.records = {}  # id(sim) -> _SimRecord (insertion ordered)
        self.sims = []

    def record_of(self, sim):
        rec = self.records.get(id(sim))
        if rec is None:
            rec = self.records[id(sim)] = _SimRecord()
            self.sims.append(sim)
        return rec

    def on_stage(self, sim, kind, stage):
        rec = self.record_of(sim)
        rec.stages.append(stage)
        if kind == "exit":
            rec.exits.append(stage)
        if kind == "direct":
            rec.direct.add(stage)

    def install(self):
        self.api.install(self.on_stage)
        original = self.jps.Simulation.iterate
        recorder = self

        def iterate(sim, count=1):
            for _ in range(count):
                recorder.step(sim, original)

        self.jps.Simulation.iterate = iterate

    def step(self, sim, original):
        rec = self.record_of(sim)
        it = sim.iteration_count()
        agents = list(sim.agents())
        rec.targets = {a.id: self.api.target(a) for a in agents}
        if not rec.agent_count:
            self.note_count(rec, it, len(agents))
        if it % SAMPLE_EVERY == 0:
            self.sample(rec, it, agents)
        original(sim, 1)
        self.note_count(rec, it + 1, sim.agent_count())
        self.note_removed(sim, rec, it)
        rec.end_time = sim.elapsed_time()

    @staticmethod
    def note_count(rec, it, count):
        if not rec.agent_count or rec.agent_count[-1][1] != count:
            rec.agent_count.append([it, count])

    def sample(self, rec, it, agents):
        rec.assignments[str(it)] = {
            str(a.id): self.assignment(rec, rec.targets[a.id]) for a in agents
        }
        rec.positions[str(it)] = {
            str(a.id): list(self.api.xy(a)) for a in agents
        }

    @staticmethod
    def assignment(rec, stage):
        if stage == _Api2x.DIRECT or stage in rec.direct:
            return "direct"
        if stage in rec.stages:
            return rec.stages.index(stage)
        return "unknown"

    def note_removed(self, sim, rec, it):
        for removed in sim.removed_agents():
            stage = self.api.removed_exit(rec, removed)
            key = str(rec.exits.index(stage)) if stage in rec.exits else "other"
            rec.exit_counts.setdefault(key, []).append(it)

    def result(self):
        out = []
        for sim in self.sims:
            rec = self.records[id(sim)]
            out.append({m: getattr(rec, m) for m in METRICS})
        return out


class _HeadlessViewer:
    """Stands in for examples/simulation_viewer.py, which needs a display.

    Mirrors the viewer's loop: iterate and call ``on_step`` until no agents
    remain.
    """

    max_iterations = 200000

    def __init__(self, sim, on_step=None, **_):
        self._sim = sim
        self._on_step = on_step

    def run(self):
        for _ in range(self.max_iterations):
            self._sim.iterate()
            if self._on_step is not None:
                self._on_step(self._sim)
            if self._sim.agent_count() == 0:
                return


def _pin_randomness(jps):
    """Make scripts that leave random seeds unset reproducible."""
    import inspect

    import numpy as np

    random.seed(0)
    np.random.seed(0)
    distributions = importlib.import_module("jupedsim.distributions")
    for name in dir(distributions):
        func = getattr(distributions, name)
        if not name.startswith("distribute_") or not callable(func):
            continue
        wrapped = _with_default_seed(func, inspect.signature(func))
        setattr(distributions, name, wrapped)
        if getattr(jps, name, None) is func:
            setattr(jps, name, wrapped)


def _with_default_seed(func, signature):
    def wrapper(*args, **kwargs):
        bound = signature.bind(*args, **kwargs)
        if bound.arguments.get("seed") is None:
            bound.arguments["seed"] = DEFAULT_SEED
        return func(*bound.args, **bound.kwargs)

    return wrapper


def _notebook_source(path):
    cells = json.loads(path.read_text())["cells"]
    lines = []
    for cell in cells:
        if cell["cell_type"] != "code":
            continue
        for line in "".join(cell["source"]).splitlines():
            if not line.lstrip().startswith(("%", "!")):
                lines.append(line)
        lines.append("")
    return "\n".join(lines)


def _run_in_child(script, workdir):
    """Execute the scenario in the current process and print metrics."""
    import jupedsim as jps

    if _Api1x.detect(jps):
        api = _Api1x(jps)
    elif _Api2x.detect(jps):
        api = _Api2x(jps)
    else:
        raise SystemExit("No adapter for this jupedsim API")
    recorder = _Recorder(jps, api)
    recorder.install()
    _pin_randomness(jps)
    viewer_module = types.ModuleType("simulation_viewer")
    viewer_module.SimulationViewer = _HeadlessViewer
    sys.modules["simulation_viewer"] = viewer_module
    os.environ["MPLBACKEND"] = "Agg"
    path = Path(script).resolve()
    if path.suffix == ".ipynb":
        source = _notebook_source(path)
        code = compile(source, str(path), "exec")
        namespace = {"__name__": "__main__", "__file__": str(path)}
        sys.path.insert(0, str(path.parent))
        exec(code, namespace)
    else:
        sys.path.insert(0, str(path.parent))
        sys.argv = [str(path)]
        try:
            runpy.run_path(str(path), run_name="__main__")
        except SystemExit as e:
            if e.code not in (None, 0):
                raise
    return recorder.result()


def _environment():
    cmake = {}
    cache = ROOT / "build" / "CMakeCache.txt"
    if cache.exists():
        for line in cache.read_text().splitlines():
            m = re.match(
                r"(CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER):\w+=(.*)", line
            )
            if m:
                cmake[m.group(1)] = m.group(2)
    compiler = cmake.get("CMAKE_CXX_COMPILER", "c++")
    try:
        version = subprocess.run(
            [compiler, "--version"], capture_output=True, text=True
        ).stdout.splitlines()[0]
    except (OSError, IndexError):
        version = "unknown"
    commit = subprocess.run(
        ["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True
    ).stdout.strip()
    return {
        "commit": commit,
        "compiler": compiler,
        "compiler_version": version,
        "cmake_build_type": cmake.get("CMAKE_BUILD_TYPE", "unknown"),
        "platform": platform.platform(),
        "python": platform.python_version(),
    }


def _child_env():
    """The environment of a child run: the build tree's jupedsim comes first."""
    inherited = os.environ.get("PYTHONPATH")
    pythonpath = os.pathsep.join(
        [str(p) for p in BUILD_PATHS] + ([inherited] if inherited else [])
    )
    return dict(os.environ, MPLBACKEND="Agg", PYTHONPATH=pythonpath)


def _jupedsim_file():
    """The file of the jupedsim module that child runs import."""
    proc = subprocess.run(
        [sys.executable, "-c", "import jupedsim; print(jupedsim.__file__)"],
        env=_child_env(),
        capture_output=True,
        text=True,
    )
    if proc.returncode != 0:
        return "not importable"
    return proc.stdout.strip()


class ScenarioCrashError(Exception):
    """A scenario exited with an error; the message is its stderr tail."""


def run_scenario(script):
    """Run one scenario in a child process; return its per-simulation metrics."""
    script = Path(script)
    with tempfile.TemporaryDirectory() as tmp:
        src_dir = script.resolve().parent
        work = Path(tmp) / src_dir.name
        shutil.copytree(src_dir, work)
        out = Path(tmp) / "metrics.json"
        env = _child_env()
        proc = subprocess.run(
            [
                sys.executable,
                str(Path(__file__).resolve()),
                "--child",
                str(work / script.name),
                str(out),
            ],
            cwd=work,
            env=env,
            capture_output=True,
            text=True,
        )
        if proc.returncode != 0:
            raise ScenarioCrashError(proc.stderr[-4000:])
        return json.loads(out.read_text())


def _baseline_path(script):
    return BASE_DIR / (script.replace("/", "__") + ".json")


def _scenarios(selected):
    listed = [
        line.strip()
        for line in (BASE_DIR / "SCENARIOS.txt").read_text().splitlines()
        if line.strip()
    ]
    return [s for s in listed if not selected or s in selected]


def _exemptions():
    exempt = set()
    for line in (BASE_DIR / "EXEMPT.txt").read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split(None, 2)
        if len(parts) < 3 or parts[1] not in METRICS:
            raise SystemExit(
                f"Bad EXEMPT.txt line (need script metric reason): {line}"
            )
        exempt.add((parts[0], parts[1]))
    return exempt


def _side(key, old, new):
    return "baseline" if key in old else "current"


def _diff_scalar(old, new):
    return f"{old!r} vs {new!r}"


def _diff_changes(old, new):
    """Compare [iteration, value] change points."""
    for a, b in zip(old, new):
        if a != b:
            return f"iteration {min(a[0], b[0])}: baseline {a[1]} vs current {b[1]}"
    longer, side = (
        (old, "baseline") if len(old) > len(new) else (new, "current")
    )
    return f"iteration {longer[min(len(old), len(new))][0]}: only in {side}"


def _diff_exit_counts(old, new):
    """Compare per-exit lists of removal iterations."""
    for key in sorted(set(old) | set(new), key=_sort_key):
        if key not in old or key not in new:
            first = (old.get(key) or new.get(key))[0]
            return (
                f"exit {key}: only in {_side(key, old, new)}, "
                f"first removal at iteration {first}"
            )
        a, b = old[key], new[key]
        for i, (x, y) in enumerate(zip(a, b)):
            if x != y:
                return (
                    f"exit {key}: removal #{i} at iteration "
                    f"baseline {x} vs current {y}"
                )
        if len(a) != len(b):
            longer, side = (
                (a, "baseline") if len(a) > len(b) else (b, "current")
            )
            return (
                f"exit {key}: extra removal at iteration "
                f"{longer[min(len(a), len(b))]} only in {side}"
            )
    return None


def _diff_sampled(old, new, fields):
    """Compare {iteration: {agent id: value}} samples."""
    for it in sorted(set(old) | set(new), key=_sort_key):
        if it not in old or it not in new:
            return f"iteration {it}: sampled only in {_side(it, old, new)}"
        if old[it] == new[it]:
            continue
        for agent in sorted(set(old[it]) | set(new[it]), key=_sort_key):
            a, b = old[it].get(agent), new[it].get(agent)
            if a == b:
                continue
            where = f"iteration {it}, agent {agent}"
            if a is None or b is None:
                return f"{where}: only in {_side(agent, old[it], new[it])}"
            return f"{where}, {_field_diff(a, b, fields)}"
    return None


def _field_diff(a, b, fields):
    if not fields:
        return f"target baseline {a!r} vs current {b!r}"
    for name, x, y in zip(fields, a, b):
        if x != y:
            return f"{name} baseline {x!r} vs current {y!r}"
    return f"baseline {a!r} vs current {b!r}"


def first_difference(metric, old, new):
    """Return a description of the first differing point, or None."""
    if old == new:
        return None
    if metric == "agent_count":
        return _diff_changes(old, new)
    if metric == "exit_counts":
        return _diff_exit_counts(old, new)
    if metric == "positions":
        return _diff_sampled(old, new, ("x", "y"))
    if metric == "assignments":
        return _diff_sampled(old, new, None)
    return _diff_scalar(old, new)


def _sort_key(k):
    return (0, int(k), "") if str(k).isdigit() else (1, 0, str(k))


def compare(script, baseline, current, exempt):
    """Return a list of difference messages for a scenario."""
    if len(baseline) != len(current):
        return [f"{script}: simulation count {len(baseline)} vs {len(current)}"]
    problems = []
    for index, (old, new) in enumerate(zip(baseline, current)):
        for metric in METRICS:
            if (script, metric) in exempt:
                continue
            diff = first_difference(metric, old[metric], new[metric])
            if diff:
                problems.append(f"{script} sim#{index} {metric}: {diff}")
    return problems


def cmd_record(selected):
    env = _environment()
    print(f"jupedsim: {_jupedsim_file()}")
    BASE_DIR.mkdir(exist_ok=True)
    failed = False
    for script in _scenarios(selected):
        start = time.time()
        try:
            metrics = run_scenario(ROOT / script)
        except ScenarioCrashError as crash:
            print(f"FAIL {script}: crashed\n{crash}")
            failed = True
            continue
        data = {"environment": env, "simulations": metrics}
        _baseline_path(script).write_text(
            json.dumps(data, sort_keys=True, separators=(",", ":")) + "\n"
        )
        print(f"recorded {script} ({time.time() - start:.1f}s)")
    return 1 if failed else 0


def cmd_check(selected):
    exempt = _exemptions()
    current_env = _environment()
    print(f"jupedsim: {_jupedsim_file()}")
    failed = False
    for script in _scenarios(selected):
        start = time.time()
        path = _baseline_path(script)
        if not path.exists():
            print(f"FAIL {script}: no baseline")
            failed = True
            continue
        baseline = json.loads(path.read_text())
        try:
            current = run_scenario(ROOT / script)
        except ScenarioCrashError as crash:
            print(f"FAIL {script}: crashed\n{crash}")
            failed = True
            continue
        problems = compare(script, baseline["simulations"], current, exempt)
        status = "FAIL" if problems else "ok"
        print(f"{status} {script} ({time.time() - start:.1f}s)")
        for p in problems:
            print("  " + p)
        if problems:
            failed = True
            print(f"  baseline environment: {baseline['environment']}")
            print(f"  current environment:  {current_env}")
    return 1 if failed else 0


_SELFTEST = """
import jupedsim as jps
def main(shift):
    surface = jps.WalkableSurface()
    region = surface.add_region(boundary=[(0, 0), (20, 0), (20, 10), (0, 10)])
    sim = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=surface)
    wp = sim.add_waypoint_stage([([(8, 4), (8, 6), (9, 6), (9, 4)], region)])
    exits = [
        sim.add_exit_stage([([(19, 4), (19, 6), (20, 6), (20, 4)], region)]),
        sim.add_exit_stage([([(19, 6), (19, 8), (20, 8), (20, 6)], region)]),
    ]
    ex = exits[EXIT_INDEX]
    journey = jps.JourneyDescription([wp, ex])
    journey.set_transition_for_stage(wp, jps.Transition.create_fixed_transition(ex))
    jid = sim.add_journey(journey)
    for i in range(3):
        sim.add_agent(
            journey_id=jid, stage_id=wp, position=(2 + shift, 3 + 2 * i),
            state=jps.CollisionFreeSpeedModelState(radius=0.3), region_id=region)
    sim.iterate(1800)
main(SHIFT)
"""


_SELFTEST_2X = """
import jupedsim as jps
import shapely
def main(shift):
    surface = jps.WalkableSurface()
    region = surface.add_region(
        polygon=shapely.Polygon([(0, 0), (20, 0), (20, 10), (0, 10)]))
    sim = jps.Simulation(jps.CollisionFreeSpeedModel(), surface)
    wp = sim.add_waypoint([(8, 4), (8, 6), (9, 6), (9, 4)], region=region)
    exits = [
        sim.add_exit([(19, 4), (19, 6), (20, 6), (20, 4)], region=region),
        sim.add_exit([(19, 6), (19, 8), (20, 8), (20, 6)], region=region),
    ]
    ex = exits[EXIT_INDEX]
    route = jps.Route([wp, ex])
    for i in range(3):
        sim.add_agent(
            region.at(2 + shift, 3 + 2 * i), route=route,
            state=jps.CollisionFreeSpeedModelState(radius=0.3))
    sim.iterate(1800)
main(SHIFT)
"""


def _selftest_source():
    sys.path[:0] = [str(p) for p in BUILD_PATHS]
    import jupedsim as jps

    return _SELFTEST if _Api1x.detect(jps) else _SELFTEST_2X


def cmd_selftest():
    source = _selftest_source()
    with tempfile.TemporaryDirectory() as tmp:
        results = []
        for shift, exit_index in ((0.0, 0), (0.0, 0), (0.5, 0), (0.0, 1)):
            script = Path(tmp) / "selftest.py"
            script.write_text(
                source.replace("SHIFT", repr(shift)).replace(
                    "EXIT_INDEX", str(exit_index)
                )
            )
            results.append(_run_selftest(script))
    same = compare("selftest", results[0], results[1], set())
    moved = compare("selftest", results[0], results[2], set())
    rerouted = compare("selftest", results[0], results[3], set())
    detected = [
        metric
        for metric in ("assignments", "exit_counts")
        if any(f" {metric}:" in line for line in rerouted)
    ]
    if same or not moved or detected != ["assignments", "exit_counts"]:
        print("selftest FAILED", same, moved, rerouted)
        return 1
    print("selftest ok:", moved[0])
    print("selftest ok:", rerouted[0])
    return 0


def _run_selftest(script):
    out = script.parent / "m.json"
    subprocess.run(
        [
            sys.executable,
            str(Path(__file__).resolve()),
            "--child",
            str(script),
            str(out),
        ],
        cwd=script.parent,
        env=_child_env(),
        check=True,
    )
    return json.loads(out.read_text())


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("command", nargs="?", choices=("record", "check"))
    parser.add_argument("scripts", nargs="*")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--child", nargs=2, metavar=("SCRIPT", "OUT"))
    args = parser.parse_args()
    if args.child:
        metrics = _run_in_child(args.child[0], Path.cwd())
        Path(args.child[1]).write_text(json.dumps(metrics, sort_keys=True))
        return 0
    if args.selftest:
        return cmd_selftest()
    if args.command == "record":
        return cmd_record(args.scripts)
    if args.command == "check":
        return cmd_check(args.scripts)
    parser.error("a command or --selftest is required")


if __name__ == "__main__":
    sys.exit(main())
