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

#ifndef JPSCORE_SIMOBSERVER_H
#define JPSCORE_SIMOBSERVER_H

#include "../pedestrian/AgentsSource.h"

#include <map>

class SimObserver
{
public:
    struct ODRelation {
        int nrAgents;
        std::shared_ptr<AgentsSource> src;
        int goalId;
    };

    void AddPedestrian(std::shared_ptr<AgentsSource> src, int goalId);

    std::map<std::string, ODRelation> GetAllRelations();

private:
    std::map<std::string, ODRelation> _odRelations;
};

#endif //JPSCORE_SIMOBSERVER_H
