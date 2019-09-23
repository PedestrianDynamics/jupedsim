/**
 * \file        GraphEdge.cpp
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
 **/
#include "GraphEdge.h"

#include "GraphVertex.h"

#include "geometry/SubRoom.h"
#include "geometry/Crossing.h"
#include "geometry/Transition.h"

/**
 * Constructors & Destructors
 */

GraphEdge::~GraphEdge()
{
     return;
}

GraphEdge::GraphEdge(const GraphVertex * const s, const GraphVertex  * const d, const Crossing * const crossing)
     : _src(s), _dest(d), _crossing(crossing)
{
     CalcApproximateDistance();

}

GraphEdge::GraphEdge(GraphEdge const &ge)
     : _src(ge._src), _dest(ge._dest), _crossing(ge._crossing), _approximate_distance(ge._approximate_distance)
{
}

void GraphEdge::CalcApproximateDistance()
{
     double distance = 0.0;
     int count = 0;
     for(std::vector<Crossing*>::const_iterator it = _src->GetSubRoom()->GetAllCrossings().begin(); it != _src->GetSubRoom()->GetAllCrossings().end(); ++it) {
          if(_crossing->GetUniqueID() == (*it)->GetUniqueID()) continue;
          if(GetDest() != nullptr && ((*it)->GetSubRoom1() == GetDest()->GetSubRoom() || (*it)->GetSubRoom2() == GetDest()->GetSubRoom())) continue;
          count++;
          distance = distance + (((*it)->GetCentre() - _crossing->GetCentre()).Norm());
     }

     for(std::vector<Transition*>::const_iterator it = _src->GetSubRoom()->GetAllTransitions().begin(); it != _src->GetSubRoom()->GetAllTransitions().end(); ++it) {
          if(_crossing->GetUniqueID() == (*it)->GetUniqueID()) continue;
          if(GetDest() != nullptr && ((*it)->GetSubRoom1() == GetDest()->GetSubRoom() || (*it)->GetSubRoom2() == GetDest()->GetSubRoom())) continue;
          count++;
          distance = distance + (((*it)->GetCentre() - _crossing->GetCentre()).Norm());
     }
     if(count == 0) _approximate_distance = 0;
     else _approximate_distance = distance/count;
}


double GraphEdge::GetWeight(const Point & position) const
{
//    if(factors.empty()) {
//        return GetApproximateDistance(position);
//    }
    //double weight = GetFactorWithDistance(GetApproximateDistance(position));
    //double weight = GetApproximateDistance(position) * GetSpecificFactor("SmokeSensor");
    double weight = GetApproximateDistance(position) * GetFactor();
    //std::cout << " End Weight from Graph Edge: " << weight << " approx distance: " << GetApproximateDistance(position) << std::endl;
    //getc(stdin);
    return weight;
}

double GraphEdge::GetFactor() const
{
    double factor = 1.0;

    for(FactorContainer::const_iterator it = factors.begin(); it != factors.end(); ++it) {
        factor = factor * it->second.first;
    }
    //std::cout << "Total Factor from Graph Edge: " << factor << std::endl;
    return factor;
}

double GraphEdge::GetSpecificFactor(std::string name) const
{
    for(FactorContainer::const_iterator it = factors.begin(); it != factors.end(); ++it)
    {
        if (it->first==name)
        {
            return it->second.first;
        }
    }
    //if no factor with the name was found return 1.0
    return 1.0;
}

double GraphEdge::GetFactorWithDistance(double distance) const
{
    double factor = distance;
    for(FactorContainer::const_iterator it = factors.begin(); it != factors.end(); ++it) {
        factor = factor + distance * it->second.first;
    }

    return factor;
}

void GraphEdge::SetFactor(double factor, std::string name)
{
     //TODO: set global time as second double
     factors[name] = std::make_pair(factor, 0.0);
}



/**
 * GETTER AND SETTER
 */

double GraphEdge::GetRoomToFloorFactor() const
{
     if(GetDest() == nullptr || GetDest()->GetSubRoom()->GetType() == GetSrc()->GetSubRoom()->GetType()) return 1.0;
     if(GetDest()->GetSubRoom()->GetType() == "floor") return 1.0;
     else return 5.0;
}

double GraphEdge::GetApproximateDistance(const Point & position) const
{
     return (_crossing->GetCentre()-position).Norm();
}

double GraphEdge::GetApproximateDistance() const
{
     return _approximate_distance;
}

const GraphVertex * GraphEdge::GetDest() const
{
     return _dest;
}
const GraphVertex * GraphEdge::GetSrc() const
{
     return _src;
}

const Crossing * GraphEdge::GetCrossing() const
{
    return _crossing;
}

bool GraphEdge::IsExit() const
{
     return _crossing->IsExit();
}
