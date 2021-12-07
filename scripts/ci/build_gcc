set -e
export CXX=/usr/bin/g++
export CC=/usr/bin/gcc
mkdir build && cd build
cmake  .. -DCMAKE_PREFIX_PATH=/opt/deps -DBUILD_TESTS=ON
cmake --build . -- -j $(nproc) -- VERBOSE=1
