set -e
echo Using ${CC} and ${CXX}
numcpus=$(nproc)
mkdir build && cd build
cmake  .. -DBUILD_TESTS=ON -DWERROR=ON
cmake --build . -- -j ${numcpus} -- VERBOSE=1
cmake --build . -t tests -- -j ${numcpus} -- VERBOSE=1
