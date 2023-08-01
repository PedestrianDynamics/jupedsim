set -e
numcpus=$(sysctl -n hw.logicalcpu)
mkdir build && cd build
python3.10 -m venv python-env
source python-env/bin/activate
pip install wheel build setuptools pytest
cmake .. -DBUILD_TESTS=ON -DWERROR=ON
cmake --build . -- -j ${numcpus}
cmake --build . -t tests -- -j ${numcpus}
cd ..
python -m build -w
