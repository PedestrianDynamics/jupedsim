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

class EventFileParser
{
public:
    static std::vector<Event> ParseEvents(const fs::path & eventFile);
    static std::vector<Event> ParseSchedule(const fs::path & scheduleFile);
    static std::map<int, int> ParseMaxAgents(const fs::path & scheduleFile);
};
