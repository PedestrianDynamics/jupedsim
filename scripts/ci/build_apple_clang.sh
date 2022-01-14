set -e
numcpus=$(sysctl -n hw.logicalcpu)
mkdir build && cd build
./../scripts/setup-deps.sh --install-path deps
python3 -m venv python-env
source python-env/bin/activate
pip install -r ../requirements.txt
cmake .. -DCMAKE_PREFIX_PATH="$(pwd)/deps;/usr/local/opt/qt@5" -DBUILD_TESTS=ON
cmake --build . -- -j ${numcpus}
cmake --build . -t tests -- -j ${numcpus}
cmake --build . -t package -- -j ${numcpus}
