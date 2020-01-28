/**
 * Copyright (c) 2020 Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 **/
#pragma once

#include "events/Event.h"
#include "general/Filesystem.h"

#include <map>
#include <vector>

namespace EventFileParser
{
/**
 * Reads all events from a specfic files and returns them in a vector.
 * @param eventFile File containing the events
 * @return All events which could be parsed from \p eventFile
 */
std::vector<Event> ParseEvents(const fs::path & eventFile);

/**
 * Reads the events from a specific schedule file and returns them in a vector.
 * @param scheduleFile File containing the schedule
 * @return All events which could be parsed from \p scheduleFile
 */
std::vector<Event> ParseSchedule(const fs::path & scheduleFile);

/**
 * Reads the max agents defintions from the schedule file and returns them as a map of transition ID to maxAgents.
 * @param scheduleFile File containing the schedule
 * @return Map of max agents defintion which could be parsed from \p scheduleFile (ID to maxAgents)
 */
std::map<int, int> ParseMaxAgents(const fs::path & scheduleFile);
}; // namespace EventFileParser
