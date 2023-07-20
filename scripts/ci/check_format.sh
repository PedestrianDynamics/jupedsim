#!/bin/bash
set -ex
mkdir build && cd build
cmake  .. -DCMAKE_PREFIX_PATH=/opt/deps -DWITH_FORMAT=ON

cmake --build . --target check-format -- VERBOSE=1