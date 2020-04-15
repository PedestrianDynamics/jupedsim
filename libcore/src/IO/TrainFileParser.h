#pragma once

#include "geometry/Building.h"

#include <optional>
#include <string>
#include <tinyxml.h>

namespace TrainFileParser
{
std::map<int, TrainTimeTable> ParseTrainTimetable(const fs::path & trainTimeTableFile);

std::optional<TrainTimeTable> ParseTrainTimeTableNode(TiXmlElement * node);

std::map<std::string, TrainType> ParseTrainType(const fs::path & trainTypeFile);

std::optional<TrainType> ParseTrainTypeNode(TiXmlElement * node);
}; // namespace TrainFileParser
