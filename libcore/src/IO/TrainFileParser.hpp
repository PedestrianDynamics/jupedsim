#pragma once

#include "events/Event.hpp"
#include "geometry/Building.hpp"
#include "geometry/TrainGeometryInterface.hpp"

#include <optional>
#include <string>
#include <tinyxml.h>

namespace TrainFileParser
{
struct TimeTableContents {
    std::map<int, TrainType> trains;
    std::vector<TrainEvent> events;
};
/**
 * Parses the time table for trains from \p trainTimeTableFile and adds the arriving and departing
 * trains as event in \p eventManager. Additionally the trains will also be added to \p building.
 * @param trainTypes User defined train types
 * @param trainTimeTableFile File containing information of arriving and departing trains
 */
TimeTableContents ParseTrainTimeTable(
    const std::map<std::string, TrainType> & trainTypes,
    const fs::path & trainTimeTableFile);

/**
 * Parses the train types from a specific file: \p trainTypeFile.
 * @param trainTypeFile File containing information of the train
 * @return map of parsed TrainTypes with the _type as key
 */
std::map<std::string, TrainType> ParseTrainTypes(const fs::path & trainTypeFile);

}; // namespace TrainFileParser
