# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests that Python can reach the floorfield Rust crate via C++ (cxx bridge)."""

import py_jupedsim as jps_native


def test_add():
    assert jps_native.floorfield.add(2, 3) == 5


def test_add_negative():
    assert jps_native.floorfield.add(-10, 10) == 0


def test_greeting():
    msg = jps_native.floorfield.greeting()
    assert isinstance(msg, str)
    assert "Rust" in msg
