#include "Parsing.h"

#include "TrajectoryData.h"

#include <benchmark/benchmark.h>

static void BM_ParseTxtFormat(benchmark::State & state)
{
    // Perform setup here
    for(auto _ : state) {
        TrajectoryData data;
        Parsing::ParseTxtFormat("/Users/kkratz/Downloads/results/bottleneck_traj.txt", &data);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_ParseTxtFormat);
