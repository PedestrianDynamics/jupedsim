#! /bin/zsh 
set -e
eval "$(/opt/homebrew/bin/brew shellenv)"
numcpus=$(sysctl -n hw.logicalcpu)
mkdir build && cd build
python3.10 -m venv python-env
source python-env/bin/activate
pip install -r ../requirements.txt
cmake .. -DBUILD_TESTS=ON -DWERROR=ON -DCMAKE_UNITY_BUILD=ON
cmake --build . -- -j ${numcpus}
cmake --build . -t tests -- -j ${numcpus}
cd ..
python -m build -w
