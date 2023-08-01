set -e
export CXX=/usr/bin/clang++
export CC=/usr/bin/clang
export PYTHON_DIR=/opt/python/cp311-cp311

numcpus=$(nproc)
mkdir build && cd build
cmake  .. -DBUILD_TESTS=ON -DWERROR=ON -DPython_ROOT_DIR=$PYTHON_DIR
cmake --build . -- -j ${numcpus} -- VERBOSE=1
cmake --build . -t tests -- -j ${numcpus} -- VERBOSE=1
