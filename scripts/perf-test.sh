set -e

# Build jupedsim
export CXX=/usr/bin/g++
export CC=/usr/bin/gcc
numcpus=$(nproc)
cd /build
cmake  /src -DCMAKE_PREFIX_PATH=/opt/deps -DBUILD_TESTS=ON -DWERROR=ON
cmake --build . -- -j ${numcpus} -- VERBOSE=1

# load correct environment
cd /build
source environment

# Create flame graph for large_street_network
perf record --call-graph fp -e cycles:u /src/performancetest/large_street_network.py --limit 4000
perf script > large_street_network.perf
/opt/FlameGraph/stackcollapse-perf.pl large_street_network.perf > large_street_network.folded
/opt/FlameGraph/flamegraph.pl --width 2000 large_street_network.folded > large_street_network_perf.svg

# Create flame graph for grosser_stern
perf record --call-graph fp -e cycles:u /src/performancetest/grosser_stern.py --limit 200
perf script > grosser_stern.perf
/opt/FlameGraph/stackcollapse-perf.pl grosser_stern.perf > grosser_stern.folded
/opt/FlameGraph/flamegraph.pl --width 2000 grosser_stern.folded > grosser_stern.svg
