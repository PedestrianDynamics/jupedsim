#include <SimulexAPI.h>

#include "test_data.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

void DumpDistanceMap(size_t testNumber)
{
    auto DistMapSetupPointer = (DISTMAPSETUP*) GetDistanceMapsPtr();
    std::vector<int> AllMapValuesInCms;
    int nXvals, nYvals;
    FPOINT3D mapOrigin = {0};

    GetFloorDistanceMapDims(0, nXvals, nYvals, mapOrigin.x, mapOrigin.y, mapOrigin.z);

    int nMapVals = nXvals * nYvals;
    AllMapValuesInCms.resize(nMapVals);

    int nGotVals = GetFloorMapValues(0, 0, nMapVals, AllMapValuesInCms[0]);

    std::stringstream ss;
    ss << "test_output_simulex_" << std::setfill('0') << std::setw(2) << testNumber << ".csv";
    std::ofstream file(ss.str());
    for(size_t y = 0; y < nYvals; ++y) {
        for(size_t x = 0; x < nXvals; ++x) {
            file << AllMapValuesInCms[x + y * nXvals]; //(x) + (y) * (int) xsize)
            if(x < nXvals - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

void RunTestCase(
    size_t testNumber,
    const std::tuple<std::vector<help::Line>, help::Point>& testCase)
{
//    const auto& testCaseLines = std::get<0>(testCase);
    const auto& testCaseExit = std::get<1>(testCase);
//
//    std::vector<LINE> obstacleLines;
//    std::transform(
//        std::begin(testCaseLines),
//        std::end(testCaseLines),
//        std::back_inserter(obstacleLines),
//        [](const help::Line line) {
//            const auto& p1 = std::get<0>(line);
//            const auto& p2 = std::get<1>(line);
//            return LINE{std::get<0>(p1), std::get<1>(p1), std::get<0>(p2), std::get<1>(p2)};
//        });
//
    PrepareMovePlaneBufForFloor(
        true, false); // Prepare floor data in the floor/movement plane 'input buffer'...

    SetMovePlaneBufTitle("Ground Floor"); // Define the "name"
//    AddLinesToMovePlaneBuf(obstacleLines.data(), obstacleLines.size(), 0.0f, 0.0f, 0);
//    SetMovePlaneBufSpaceDims(0.0f, 0.0f, 0.0f, 0.0f);
        SetMovePlaneBufSpaceDims(
        2.0f,
        2.0f,
        2.0f,
        2.0f); // Add bounding space (left, right, top, bottom)

    AddFloorFromMovePlaneBuf(nullptr);


    EXIT exit = {
        "Exit",
        (WORD) (FLOORWINDOW | GetNumFloors() - 1),
        0,//std::get<0>(testCaseExit),
        0,//std::get<1>(testCaseExit),
        NinetyDegrees,
        0.00000001f,
        0};
    AddExit(&exit, 0);

    CalculateAllDistanceMaps();
    DumpDistanceMap(testNumber);
}

int main(int argc, char* argv[])
{
    for(size_t i = 0; i < help::cases.size(); ++i) {
        RunTestCase(i, help::cases[i]);
    }
}
