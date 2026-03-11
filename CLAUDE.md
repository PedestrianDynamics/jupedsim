# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Development

### Developer Build (C++ and Python)

```bash
git submodule update --init
pip install -r requirements.txt
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON -DWITH_FORMAT=ON
make -j$(nproc)
source ./environment   # sets PYTHONPATH for development
```

Requires a C++20 compiler and CMake >= 3.22.

### Quick Python Install

```bash
pip install -r requirements.txt
pip install .
```

## Testing

```bash
# All tests
make tests

# C++ unit tests only (Google Test)
make unittests

# Single C++ test
./bin/libsimulator-tests --gtest_filter='TestClassName.TestName'

# Python system tests only (pytest)
make systemtests

# Single Python test
pytest systemtest/test_file.py::test_name
```

Test output XML is written to the build directory for CI.

## Formatting & Linting

```bash
# C++ (clang-format, requires -DWITH_FORMAT=ON)
cmake --build . --target check-format   # check only
cmake --build . --target reformat       # apply fixes

# Python (ruff)
ruff check .          # lint
ruff format --diff .  # check format
ruff format .         # apply format
```

CI enforces both C++ and Python formatting — code that fails format checks will not merge.

## Architecture

JuPedSim is a three-layer pedestrian dynamics simulation library:

```
Python API (python_modules/jupedsim/)
    ↓ wraps
pybind11 bindings (python_bindings_jupedsim/)
    ↓ exposes
C++ core (libsimulator/)
```

**libcommon/** — Header-only utilities (Visitor pattern, Unreachable macro).

**libsimulator/** — The simulation engine. Key subsystems:
- `Simulation` — Main facade: manages agents, journeys, geometry, clock, and coordinates three decision systems (strategical → tactical → operational)
- **Operational models** — Movement algorithms using the strategy pattern with `std::variant` for polymorphic agent state: `SocialForceModel`, `GeneralizedCentrifugalForceModel`, `CollisionFreeSpeedModel` (V1 & V2), `AnticipationVelocityModel`. Each has a corresponding Builder, Data, and Update type.
- **Journey/Stage system** — Journeys define agent paths through stages (waypoints, queues, waiting sets, exits, direct steering) with configurable transitions (fixed, round-robin, least-targeted)
- **Geometry** — Collision geometry built on CGAL, with routing engine for pathfinding and grid-based neighborhood search
- Tests in `libsimulator/test/`, benchmarks in `libsimulator/benchmark/`

**python_bindings_jupedsim/** — One `.cpp` file per bound component.

**python_modules/jupedsim/** — Pythonic wrapper layer. Adds shapely integration, serialization (SQLite), recording, and visualization support. Sub-packages: `models/` (one file per operational model), `internal/` (tracing, grid, AABB, notebook utils).

**third-party/** — Vendored dependencies: CGAL, fmt, glm, pybind11, googletest, benchmark, boost.

## Code Conventions

- **C++20** — `.clang-format` enforces LLVM-derived style: 100-column limit, 4-space indent, Linux brace style, left-aligned pointers
- **Python** — ruff with 80-char lines, double quotes, isort. Config in `pyproject.toml`
- **Naming** — C++ uses PascalCase for classes and methods. Python follows PEP 8.
- **License** — All source files must have `SPDX-License-Identifier: LGPL-3.0-or-later` header
- **Tests** — C++ test files: `Test*.cpp`. Python test files: `test_*.py`
- **Headers** — Use `#pragma once`
