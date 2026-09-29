#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Check that the installed jupedsim ships a type stub for every module of the
native extension jupedsim.py_jupedsim, including its submodules.

Used as the cibuildwheel test command; exits non-zero and names the missing
stubs if any is absent.
"""

import importlib.resources
import sys
import types

import jupedsim.py_jupedsim as native


def main():
    stub_dir = importlib.resources.files("jupedsim") / "py_jupedsim"
    submodules = sorted(
        name
        for name, value in vars(native).items()
        if isinstance(value, types.ModuleType)
        and value.__name__.startswith(f"{native.__name__}.")
    )
    expected = ["__init__.pyi"] + [f"{name}.pyi" for name in submodules]
    missing = [stub for stub in expected if not (stub_dir / stub).is_file()]
    if missing:
        print(f"missing stubs in {stub_dir}: {', '.join(missing)}")
        return 1
    print(f"stubs present: {', '.join(expected)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
