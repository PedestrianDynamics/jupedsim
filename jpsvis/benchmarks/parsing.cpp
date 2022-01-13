#include "Parsing.h"

#include "TrajectoryData.h"

#include <benchmark/benchmark.h>
#include <filesystem>
/// A note about the benchmarks here:
/// Right now we do not intend to execute thses in any automated enivronment
/// Consider them as a local developemnt tool if you want to fine tune the performance of specifc
/// functions.
static void BM_ParseTxtFormat(benchmark::State & state)
{
    // Note: The path below will only work if you call the benchmarks from the source folder,
    // i.e. from `jpsvis`  call: `./<path-to-build>/bin/benchmarks`
    // Most likely you will want to replace this path with a path to a much larger file anyways that
    // is not part of the repository, i.e. any trajectory upwards of 500 Mb.
    const auto path = QString::fromStdString(
        std::filesystem::current_path() / "samples/02_stairs/stairs_trajectories.txt");
    for(auto _ : state) {
        TrajectoryData data;
        Parsing::ParseTxtFormat(path, &data);
    }
}
BENCHMARK(BM_ParseTxtFormat);
