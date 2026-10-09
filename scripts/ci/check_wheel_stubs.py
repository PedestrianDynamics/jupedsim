#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Check that the installed jupedsim ships the type stubs of the native
extension jupedsim.py_jupedsim.

Without submodules pybind11-stubgen writes a single py_jupedsim.pyi; with
submodules it writes a py_jupedsim/ package with one stub per module.

Used as the cibuildwheel test command; exits non-zero and names the missing
stubs if any is absent.
"""

import importlib.resources
import sys
import types

import jupedsim.py_jupedsim as native


def main():
    package = importlib.resources.files("jupedsim")
    submodules = sorted(
        name
        for name, value in vars(native).items()
        if isinstance(value, types.ModuleType)
        and value.__name__.startswith(f"{native.__name__}.")
    )
    if submodules:
        expected = ["py_jupedsim/__init__.pyi"] + [
            f"py_jupedsim/{name}.pyi" for name in submodules
        ]
    else:
        expected = ["py_jupedsim.pyi"]
    missing = [stub for stub in expected if not (package / stub).is_file()]
    if missing:
        print(f"missing stubs in {package}: {', '.join(missing)}")
        return 1
    print(f"stubs present: {', '.join(expected)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
