/**
 * \file        LastDestinationsSensor.cpp
 * \date        Jan 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 * \section Description
 *
 *
 *
 **/

#include "LastDestinationsSensor.h"


#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"
#include "router/smoke_router/cognitiveMap/cognitivemap.h"
#include "router/smoke_router/NavigationGraph.h"

LastDestinationsSensor::~LastDestinationsSensor()
{
}

std::string LastDestinationsSensor::GetName() const
{
    return "LastDestinationsSensor";
}

void LastDestinationsSensor::execute(const Pedestrian * /*pedestrian*/, CognitiveMap &cognitive_map) const
{
    NavigationGraph * ng = cognitive_map.GetGraphNetwork()->GetNavigationGraph();
    std::vector<const GraphEdge *> & destinations = cognitive_map.GetGraphNetwork()->GetDestinations();
    int i = 1;

    std::set<const GraphEdge *> rated;

    for(std::vector<const GraphEdge *>::reverse_iterator it = destinations.rbegin(); it != destinations.rend(); ++it) {
        GraphEdge * to_edge = ng->operator []((*it)->GetSrc()->GetSubRoom())->operator []((*it)->GetCrossing());

        if(to_edge != nullptr && rated.find(to_edge) == rated.end()) {
            to_edge->SetFactor(300.0/(i*3.0), GetName());
            rated.insert(to_edge);
        }

        if((*it)->GetSrc() != nullptr && (*it)->GetDest() != nullptr && (*it)->GetCrossing() != nullptr) {
            GraphEdge * back_edge = ng->operator []((*it)->GetDest()->GetSubRoom())->operator []((*it)->GetCrossing());
            if(back_edge != nullptr && rated.find(back_edge) == rated.end()) {
                back_edge->SetFactor(300.0/(i*3.0), GetName());
                rated.insert(back_edge);
            }
        }
        i++;
    }
}
