set -e
numcpus=$(nproc)
mkdir build && cd build
cmake  .. \
    -DCMAKE_PREFIX_PATH=/opt/deps \
    -DBUILD_TESTS=ON \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix \
    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-c++-posix
cmake --build . -- -j ${numcpus} -- VERBOSE=1
