set -e

# load correct environment
cd /build
source environment

# Create flame graph for grosser_stern
perf record --call-graph fp -e cycles:u /src/performancetest/grosser_stern.py --limit 10
perf script > grosser_stern.perf
/opt/FlameGraph/stackcollapse-perf.pl grosser_stern.perf > grosser_stern.folded
/opt/FlameGraph/flamegraph.pl --width 2000 grosser_stern.folded > grosser_stern.svg
