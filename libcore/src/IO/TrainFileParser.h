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
/**
 * Parses the time table for trains from \p trainTimeTableFile and adds the arriving and departing
 * trains as event in \p eventManager. Additionally the trains will also be added to \p building.
 * @param eventManager Manager for handling the events
 * @param building Geometry of the simulation
 * @param trainTypes User defined train types
 * @param trainTimeTableFile File containing information of arriving and departing trains
 */
void ParseTrainTimeTable(
    EventManager & eventManager,
    Building & building,
    const std::map<std::string, TrainType> & trainTypes,
    const fs::path & trainTimeTableFile);

/**
 * Parses the train types from a specific file: \p trainTypeFile.
 * @param trainTypeFile File containing information of the train
 * @return map of parsed TrainTypes with the _type as key
 */
std::map<std::string, TrainType> ParseTrainTypes(const fs::path & trainTypeFile);

/**
 * Parses a specific train type node \p node.
 * @param node XML node containing the information of the train type
 * @return TrainType if node could parsed correctly, nullopt otherwise
 */
std::optional<TrainType> ParseTrainTypeNode(TiXmlElement * node);
}; // namespace TrainFileParser
