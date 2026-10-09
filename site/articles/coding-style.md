---
date: "2026-10-09"
author: "JuPedSim team"
description: "The C++ naming convention of JuPedSim, why it mirrors the Python API, and how clang-format and clang-tidy enforce it."
---

<!-- SPDX-License-Identifier: LGPL-3.0-or-later -->

# Coding style

JuPedSim is a C++ core driven from a Python API. People who work on it switch between the
two languages all day, so the C++ naming convention follows the Python one wherever C++
allows it: functions and variables are `snake_case`, a leading underscore marks a member
that is not part of a class's public surface, and types are `PascalCase`. One mental model
for both sides.

This page is the reference. The rules are enforced by clang-format (layout) and clang-tidy
(names) in CI, so a pull request that breaks them fails before anyone reads it.

## The rules in one line

Types and template parameters are `PascalCase`; everything you call or read is
`snake_case`; a leading `_` means private; macros `SHOUT`.

## The full table

| Identifier | Style | Example |
|---|---|---|
| class, struct, union, enum, concept, type alias, typedef | `PascalCase` | `SimulationClock`, `AABB` |
| template parameter (type, value, template, pack) | `PascalCase` | `template <typename Agent>` |
| function, method, static method, lambda bound to a variable | `snake_case` | `agents_in_range()` |
| parameter, parameter pack, local, global, static local | `snake_case` | `journey_id` |
| constant at any scope (`const`, `constexpr`, `static const`) | `snake_case` | `min_part_length` |
| non-static data member, private or protected | `_snake_case` | `_clock` |
| non-static data member, public (struct field) | `snake_case` | `agent.position` |
| static data member, any access | `snake_case` | `UniqueID::invalid` |
| enumerator | `PascalCase` | `Orientation::Clockwise` |
| macro | `UPPER_CASE` | `JPS_TRACE_EVENT` |
| namespace | `snake_case` | `jps` |
| names a standard protocol requires | as the standard spells them | `value_type`, `begin()` |
| file | after its main type: `PascalCase.hpp/.cpp`; bindings `snake_case.cpp` after the Python module; tests `TestX.cpp` | `Simulation.hpp`, `agent_view.cpp` |

Constants are variables: there is no `k` prefix and no `UPPER_CASE` for them, those are
reserved for macros. Operators, constructors, destructors and `main` have no name to style.

## Acronyms and the exceptions

- In `snake_case` an acronym is one lowercase word: `get_aabb()`, `dt()`, `agent_id`.
- In `PascalCase` an established acronym keeps its casing: `AABB`, `ID`, `CDT`. Both `AABBTree`
  and `AabbTree` pass the check; do not rename existing types for this.
- Static data members never carry the underscore, whatever their access. clang-tidy cannot
  apply access-dependent styles to them, and one rule for all is better than a rule the tool
  cannot check.
- Names required by a standard or library protocol are spelled as that protocol spells them
  (`value_type`, `iterator_category`, CGAL's `Rebind_TDS`, pybind11's `cast_op_type`,
  googletest's `PrintTo`). They are listed explicitly in `.clang-tidy`; add a new one there
  with a comment saying which protocol needs it.
- File names are not tool-checked.

## In code

```cpp
template <typename Agent>
class NeighborhoodSearch {
    double _cell_size;
    std::vector<Agent*> _cells;

public:
    explicit NeighborhoodSearch(double cell_size);
    std::vector<Agent*> agents_in_range(Point p, double distance) const;
};

struct GenericAgent {
    using ID = UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    static constexpr double default_radius = 0.2;
};

enum class Orientation { Colinear, Clockwise, CounterClockwise };

constexpr double snap_eps = 1e-9;

#define JPS_TRACE_EVENT(name) ...
```

## Layout

clang-format 23 with the repository's `.clang-format`: LLVM base, four-space indentation,
100 columns, Linux braces, left-aligned pointers and references, includes sorted in three
groups. Every file starts with `// SPDX-License-Identifier: LGPL-3.0-or-later`; headers use
`#pragma once`.

## How it is enforced

- `.clang-tidy` at the repository root holds the naming rules
  (`readability-identifier-naming`) and the other checks that run. It is the only place that
  decides what clang-tidy checks; the build targets and the CI script pass no check list.
- `third-party/` and generated headers (`BuildInfo.hpp`, the floor field's cxxbridge header)
  are never checked.
- Locally: configure with `-DWITH_FORMAT=ON -DWITH_TIDY=ON` (clang-format 23 and clang-tidy
  23 on `PATH`, or `CLANG_FORMAT` / `CLANG_TIDY` pointing at them), then `ninja check-format`
  and `ninja check-tidy`. `ninja reformat` and `ninja tidy-fix` apply the fixes. On macOS
  a Homebrew clang-tidy older than the installed SDK may fail to parse it (LLVM 21 with the
  macOS 27 SDK did); point `-DCLANG_TIDY_SYSROOT=` at an older SDK then, e.g. `MacOSX26.sdk`.
- CI: the jobs "Check Format" (`scripts/ci/check_format.sh`) and "Check clang-tidy"
  (`scripts/ci/check_tidy.sh`) run on every push and pull request to `master` and the
  release branches.
- Python is covered by ruff (`ruff check`, `ruff format --check`), configured in
  `pyproject.toml`.

A failing "Check clang-tidy" job prints `invalid case style for <kind> '<name>'` with the
file and line. Rename, or run `ninja tidy-fix` followed by `ninja reformat`. If the name is
one a protocol forces on you, add it to the ignore list in `.clang-tidy` with a comment.

## History

Until October 2026 JuPedSim mostly used `PascalCase` methods, `camelCase` locals and
`_camelCase` members. The switch was one mechanical commit, clang-tidy fixes plus a reformat,
listed in `.git-blame-ignore-revs` so that
`git config blame.ignoreRevsFile .git-blame-ignore-revs` keeps `git blame` useful.
