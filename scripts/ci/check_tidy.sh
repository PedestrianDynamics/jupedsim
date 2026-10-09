#!/bin/bash
# SPDX-License-Identifier: LGPL-3.0-or-later
# Runs the clang-tidy checks from .clang-tidy (naming convention and friends) over the
# project directories. Needs clang-tidy 23 (or CLANG_TIDY pointing at it), cmake, cargo
# (the floor field's generated cxxbridge header is built first) and pytest (configure-time
# requirement of BUILD_TESTS, which puts the tests into the compile database).
set -ex
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON -DWITH_TIDY=ON
cmake --build . --target check-tidy
