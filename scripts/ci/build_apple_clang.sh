set -e
numcpus=$(sysctl -n hw.logicalcpu)
mkdir build && cd build
python3 -m venv python-env
source python-env/bin/activate
pip install -r ../requirements.txt
cmake .. -DBUILD_TESTS=ON -DWERROR=ON
cmake --build . -- -j ${numcpus}
cmake --build . -t tests -- -j ${numcpus}
