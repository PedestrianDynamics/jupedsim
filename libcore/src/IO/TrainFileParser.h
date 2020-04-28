#pragma once

#include "events/Event.h"
#include "events/EventManager.h"
#include "events/TrainEvent.h"
#include "geometry/Building.h"

#include <optional>
#include <string>
#include <tinyxml.h>

namespace TrainFileParser
{
void ParseTrainTimeTable(
    EventManager & eventManager,
    Building & building,
    const std::map<std::string, TrainType> & trainTypes,
    const fs::path & trainTimeTableFile);

std::map<std::string, TrainType> ParseTrainTypes(const fs::path & trainTypeFile);

//std::map<int, TrainTimeTable> ParseTrainTimetable(const fs::path & trainTimeTableFile);

//std::optional<TrainTimeTable> ParseTrainTimeTableNode(TiXmlElement * node);

//void ParseTrainTimeTableNode(EventManager & eventManager, TiXmlElement * node);

//std::map<std::string, TrainType> ParseTrainType(const fs::path & trainTypeFile);

std::optional<TrainType> ParseTrainTypeNode(TiXmlElement * node);
}; // namespace TrainFileParser
