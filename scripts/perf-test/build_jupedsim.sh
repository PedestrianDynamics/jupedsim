set -e

# Build jupedsim
export CXX=/usr/bin/g++
export CC=/usr/bin/gcc
numcpus=$(nproc)
mkdir /build && cd /build
cmake  /src -DCMAKE_PREFIX_PATH=/opt/deps -DBUILD_TESTS=ON -DWERROR=ON
cmake --build . -- -j ${numcpus} -- VERBOSE=1
