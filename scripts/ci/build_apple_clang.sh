set -e
numcpus=$(sysctl -n hw.logicalcpu) 
mkdir build && cd build
./../scripts/setup-deps.sh
cmake .. -DCMAKE_PREFIX_PATH=$(pwd)/deps -DBUILD_TESTS=ON
cmake --build . -- -j ${numcpus}
cmake --build . -t tests -- -j ${numcpus}
cmake --build . -t package -- -j ${numcpus}
