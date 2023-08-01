set -e
export CXX=$(which g++)
export CC=$(which gcc)
export PYTHON_DIR=/opt/python/cp311-cp311

# Compile library and Python extension + testing
numcpus=$(nproc)
mkdir build && cd build
cmake  .. -DBUILD_TESTS=ON -DWERROR=ON -DPython_ROOT_DIR=$PYTHON_DIR
cmake --build . -- -j ${numcpus} -- VERBOSE=1
cmake --build . -t tests -- -j${numcpus} -- VERBOSE=1

# Create Python wheel
cd .. && mkdir output
$PYTHON_DIR/bin/pip wheel . -w output
auditwheel repair output/jupedsim*whl
