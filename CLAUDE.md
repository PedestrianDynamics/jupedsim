# JuPedSim

Pedestrian dynamics simulation: C++20 core, pybind11 bindings, Python API. Paths are relative
to the repository root.

## Layout

| Path | Contents |
|------|----------|
| `libsimulator/` | C++ core: `src/`, GTest tests in `test/`, benchmarks in `benchmark/` |
| `libfloorfield/` | Rust crate (floor fields, fast sweeping), bridged via cxx + corrosion; see `DESIGN.md` |
| `libcommon/` | header-only utils (`Visitor`/`Overloaded`, `Unreachable`) |
| `python_bindings_jupedsim/` | pybind11 module, one `.cpp` per bound component; generates type stubs |
| `python_modules/jupedsim/` | Python API (package `jupedsim`), tests in `tests/` |
| `python_modules/jupedsim/jupedsim/models/` | community Python models (see its `README.md`) |
| `python_modules/jupedsim_examples/` | developer example Python models (`pysocial_force`), not distributed |
| `python_modules/trame_viewer/`, `jupedsim_vis/` | separate viewer packages |
| `jupedsim.org/` | website: `www/`, `docs/`, shared `theme/`, tooling `tools/` |
| `tools/` | C++ CLI tools (`remesh`), `BUILD_TOOLS=ON` |
| `scripts/` | standalone Python scripts; CI helpers in `scripts/ci/` |
| `third-party/` | submodules benchmark, cgal, corrosion, fmt, googletest, pybind11; vendored boost, perfetto |

## Architecture

- **`Simulation::iterate()`**: agent removal → neighborhood search → strategical (next
  target) → tactical (routing) → operational (model) → neighborhood rebuild → clock advance
  → record reached targets.
- **Geometric primitives**: `kernel.hpp` — `Point` and `Vector` wrap CGAL Epick (no glm).
  Geometry is built with `GeometryBuilder` (`src/geometry/`), mirrored in Python.
- **Operational models**: one folder each in `libsimulator/src/operational_models/` (social
  force, GCFM, collision-free speed V1–V3, anticipation velocity, warp driver, `custom_model`).
  Per-agent state is `GenericAgent::state`, the `OperationalModelState` variant of one
  `<Model>State` per model. Models implement
  `OperationalModel::compute_next_state(current, next, step)`; `next` starts as a copy of
  `current`. Each agent has its own seeded RNG stream (`agent_rng.hpp`).
- **Custom/Python models**: `CustomModel` (C++), driven from Python via
  `python_bindings_jupedsim/python_model.cpp` and `jupedsim/custom_model.py`
  (`CustomOperationalModel`).
- **Routes** (`route.hpp`): targets (stages `Waypoint`, `Exit`) connected by transitions:
  fixed, round-robin, least-targeted; the transition from `AGENT_LOCATION` picks the first
  target. An agent without a route is steered to a location (direct steering).
- **Routing**: `FloorfieldRoutingEngine` (floor fields from `libfloorfield`, Python-settable
  speed fields) and a CGAL surface-mesh shortest-path engine.
- **Python output**: HDF5 (`hdf5_serialization.py`, writer + loader); no SQLite.

## Build

```sh
git submodule update --init
pip install -r requirements.txt          # into the env you configure with
cmake -S . -B <build-dir> -G Ninja -DBUILD_TESTS=ON
ninja -C <build-dir>
```

- Needs CMake >= 3.22, a C++20 compiler and a Rust toolchain (`cargo`). Default build type
  `Release`; `compile_commands.json` is exported.
- **Python targets (`ninja tests`, stubgen, pytest wrapper, website) use the interpreter found
  at configure time** (`_Python_EXECUTABLE` in `CMakeCache.txt`), not the active one. Check it
  before relying on installed packages; reconfigure with the right env active if wrong.
- `pip install .` builds the package without a build dir (scikit-build-core). Reproduce
  pip/sdist failures in a **fresh** venv: a dev venv already has build deps
  (`pybind11-stubgen`, shapely) and hides missing ones.

| CMake option | Default | Effect |
|--------------|---------|--------|
| `BUILD_TESTS` | OFF | unit + Python tests |
| `BUILD_TOOLS` | OFF | `tools/` |
| `BUILD_BENCHMARKS` | OFF | micro benchmarks |
| `BUILD_WITH_SANITIZERS` | OFF | ASan + UBSan (Linux/macOS) |
| `USE_IPO` | ON | LTO where supported |
| `WERROR` | OFF | warnings as errors (not Windows) |
| `WITH_FORMAT` | OFF | `check-format`, `reformat` (clang-format 23) |
| `WITH_TIDY` | OFF | `check-tidy`, `tidy-fix`, `check-includes`, `includes-fix` (clang-tidy 23); forces `BUILD_TESTS`, `BUILD_TOOLS`, `BUILD_BENCHMARKS` on |
| `BUILD_WEBSITE` | OFF | `website`, `website-fast` (UNIX; formerly `BUILD_SITE`) |

## Tests

CMake custom targets (not ctest), run from the build dir; JUnit XML lands there.

| Target | Runs |
|--------|------|
| `ninja tests` | all below (`websitetoolingtests` UNIX only) |
| `ninja unittests` | `libsimulator-tests` (GTest) + `cargo test` for `libfloorfield` |
| `ninja pythontests` | pytest over `python_modules/*/tests` |
| `ninja scripttests` | `scripts/dxf2wkt.py --selftest` |
| `ninja websitetoolingtests` | pytest in `jupedsim.org/tools` |

```sh
<build-dir>/bin/libsimulator-tests --gtest_filter='Suite.Name'
source <build-dir>/environment   # PYTHONPATH = <build-dir>/stage + jupedsim_examples
pytest python_modules/jupedsim/tests/test_targets.py::test_name   # from repo root
```

Where new tests go:

- Python: `python_modules/<pkg>/tests/test_<module>.py` (pytest, `--import-mode=importlib`).
- C++: `libsimulator/test/test_*.cpp`.
- Scripts in `scripts/`: inside the script behind `--selftest`, wired into `scripttests`.
  Pin its deps in `requirements.txt`; a missing dep fails, never skips.

## Website

`jupedsim.org/`: **www** Sphinx project (landing page, articles, notes), **docs** Sphinx
project, shared `theme/`, tooling package `tools/` (`jps_website_tooling`, CLI
`jps-website-tooling`, Python >= 3.14; subcommands `build-website`, `versions`,
`build-search-index`, `preview`, `publish`, `tag`, `resolve`). "The website" = all of it;
"www" = the root project.

- `ninja website` / `website-fast` (no notebook execution) → `<build-dir>/website`. Strict by
  default; `<build-dir>/build-website --lenient` keeps warnings.
- CMake runs the tooling uninstalled via `PYTHONPATH`; workflows `pip install ./jupedsim.org/tools`.
- Theme / assembled-site checks: `bash jupedsim.org/theme/check.sh --build-dir <build-dir>`.

## Code style

- **Check before reporting done:** `pre-commit run --files <changed files>` (pinned
  clang-format and ruff; same as CI). It does not cover clang-tidy (`ninja check-tidy`).
- **C++**: clang-format (LLVM base, 4-space, 100 cols, Linux braces, left-aligned pointers);
  `.clang-tidy` naming: types and template params PascalCase; functions, variables,
  constants, namespaces snake_case; private/protected members `_snake_case`, public/static
  snake_case; enumerators PascalCase; macros UPPER_CASE; acronyms lowercase (`get_aabb`).
  `#pragma once`. Files and dirs snake_case. Reference:
  `jupedsim.org/www/articles/coding-style.md`.
- **Python**: ruff (config in `pyproject.toml`), 80 cols, double quotes, isort, E501 ignored.
  Supports 3.12–3.14; `py312` target flags newer-only APIs.
- **Docstrings**: Google style, public API only (modules, public classes/functions/methods):
  one-line summary, blank line, `Args:` / `Returns:` / `Raises:` as applicable
  (`name: description`, no types when annotated). None for `_private` helpers, nested
  functions, tests. Don't rewrite or strip existing docstrings.
- **License**: C++, Rust and Python sources start with
  `SPDX-License-Identifier: LGPL-3.0-or-later`; other files need none.
- **CI** (cibuildwheel workflow): `pre-commit run --all-files` and `scripts/ci/check_tidy.sh`
  (clang-tidy + include-cleaner 23).
