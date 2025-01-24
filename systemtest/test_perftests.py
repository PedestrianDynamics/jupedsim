// SPDX-License-Identifier: LGPL-3.0-or-later
import importlib
import sys
from contextlib import contextmanager

import pytest


@contextmanager
def ARGV(args: list[str]):
    old_args = sys.argv
    try:
        sys.argv = args
        yield
    finally:
        sys.argv = old_args


@pytest.mark.parametrize(
    "module_name", ["grosser_stern", "large_street_network"]
)
def test_grosser_strern(module_name):
    module = importlib.import_module(module_name, package=None)
    with ARGV([module_name, "-l", "1"]):
        module.main()
