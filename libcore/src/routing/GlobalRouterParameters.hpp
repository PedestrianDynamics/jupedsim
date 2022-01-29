#pragma once

#include "NavLineParameters.hpp"

#include <cfloat>
#include <filesystem>
#include <vector>

struct GlobalRouterParameters {
    std::vector<NavLineParameters> optionalNavLines{};
    double minDistanceBetweenTriangleEdges{-DBL_MAX};
    double minAngleInTriangles{-DBL_MAX};
    bool useMeshForLocalNavigation{true};
    bool generateNavigationMesh{false};
};
