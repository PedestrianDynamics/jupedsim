/**
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
//
// Created by laemmel on 13.04.16.
//

#include "SimObserver.h"

#include "../pedestrian/StartDistribution.h"

#include <sstream>

void SimObserver::AddPedestrian(std::shared_ptr<AgentsSource> src, int goalId)
{
    StartDistribution * dstr = src->GetStartDistribution().get();
    std::stringstream str;
    str << dstr->GetRoomId() << "_" << dstr->GetSubroomID() << "_" << goalId; //TODO bad practice
    std::string key         = str.str();
    ODRelation * odRelation = &_odRelations[key];
    if(odRelation->src == 0) {
        odRelation->src      = src;
        odRelation->goalId   = goalId;
        odRelation->nrAgents = 0;
    }
    odRelation->nrAgents++;
}

std::map<std::string, SimObserver::ODRelation> SimObserver::GetAllRelations()
{
    return _odRelations;
}
