#include "DistanceMap.hpp"
#include "test_data.hpp"

using DMapBuilder = distance::DistanceMapBuilder<int32_t, double>;
using DMap = distance::DistanceMap<int32_t, double>;

int main()
{
    int counter{};
    DMapBuilder b{};
    b.AddExitPoint({0, 0});
    distance::DumpDistanceMapMatplotlibCSV(
        b.Build()->LocalDistance(), "jupedsim_local_distance_map.csv");
    for(const auto& [segments, exit] : help::cases) {
        DMapBuilder builder{};
        for(const auto& [p1, p2] : segments) {
            builder.AddLine(
                {{std::get<0>(p1), std::get<1>(p1)}, {std::get<0>(p2), std::get<1>(p2)}});
        }
        builder.AddExitPoint({std::get<0>(exit), std::get<1>(exit)});
        const auto map = builder.Build();
        distance::DumpDistanceMapMatplotlibCSV(
            map->Distance(), fmt::format("dump_{:02}.csv", counter++));
    }
}
