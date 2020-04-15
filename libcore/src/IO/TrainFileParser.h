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
    const Trains & trainTypes,
    const fs::path & trainTimeTableFile);

void ParseTrainTypes(Building & building, const fs::path & trainTypeFile);

//std::map<int, TrainTimeTable> ParseTrainTimetable(const fs::path & trainTimeTableFile);

//std::optional<TrainTimeTable> ParseTrainTimeTableNode(TiXmlElement * node);

//void ParseTrainTimeTableNode(EventManager & eventManager, TiXmlElement * node);

//std::map<std::string, TrainType> ParseTrainType(const fs::path & trainTypeFile);

std::optional<TrainType> ParseTrainTypeNode(TiXmlElement * node);
}; // namespace TrainFileParser
