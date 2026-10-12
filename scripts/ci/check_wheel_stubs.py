#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Check that the installed jupedsim ships the type stubs of the native
extension jupedsim.pyjupedsim.

Without submodules pybind11-stubgen writes a single pyjupedsim.pyi; with
submodules it writes a pyjupedsim/ package with one stub per module.

Used as the cibuildwheel test command; exits non-zero and names the missing
stubs if any is absent.
"""

import importlib.resources
import sys
import types

import jupedsim.pyjupedsim as jps


def main():
    package = importlib.resources.files("jupedsim")
    submodules = sorted(
        name
        for name, value in vars(jps).items()
        if isinstance(value, types.ModuleType)
        and value.__name__.startswith(f"{jps.__name__}.")
    )
    if submodules:
        expected = ["pyjupedsim/__init__.pyi"] + [
            f"pyjupedsim/{name}.pyi" for name in submodules
        ]
    else:
        expected = ["pyjupedsim.pyi"]
    missing = [stub for stub in expected if not (package / stub).is_file()]
    if missing:
        print(f"missing stubs in {package}: {', '.join(missing)}")
        return 1
    print(f"stubs present: {', '.join(expected)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
