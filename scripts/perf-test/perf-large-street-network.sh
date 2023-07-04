set -e

# load correct environment
cd /build
source environment

# Create flame graph for large_street_network
perf record --call-graph fp -e cycles:u /src/performancetest/large_street_network.py --limit 10
perf script > large_street_network.perf
/opt/FlameGraph/stackcollapse-perf.pl large_street_network.perf > large_street_network.folded
/opt/FlameGraph/flamegraph.pl --width 2000 large_street_network.folded > large_street_network_perf.svg
