/**
 * \file        GraphEdge.h
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
 * Edge of a Graph.
 *
 *
 **/
#pragma once

#include "geometry/Line.h"
#include "router/smoke_router/fire_mesh/FireMesh.h"

#include <map>

class SubRoom;
class GraphVertex;
class Crossing;

//log output
extern OutputHandler* Log;

/**
 * @brief Graph Edge.
 *
 */

class GraphEdge
{
public:
     typedef std::map<std::string, std::pair<double, double> > FactorContainer;

     GraphEdge(const GraphVertex * const s, const GraphVertex * const d, const Crossing * const crossing);
     GraphEdge(GraphEdge const & ge);
     virtual ~GraphEdge();

     void CalcApproximateDistance();

     // Getter collection
     const GraphVertex * GetDest() const;
     const GraphVertex * GetSrc() const;
     const Crossing * GetCrossing() const;

     double GetApproximateDistance() const;
     double GetApproximateDistance(const Point &) const;
     void SetFactor(double factor, std::string name);
     double GetRoomToFloorFactor() const;
     double GetFactor() const;
     double GetSpecificFactor(std::string name) const;
     double GetFactorWithDistance(double distance) const;
     double GetWeight(const Point &) const;
     bool IsExit() const;

private:
     const GraphVertex  * const _src;
     const GraphVertex  * const _dest;
     const Crossing  * const _crossing;

     /**
      * Factor Bag
      *
      * The Factor map is filled up by sensors. The key string is for identification while sharing information.
      * The second pair value is the GlobalTime value from Pedestrian Class of this information.
      * For calculating the weight we just iterate over the factors and dont care which factors we acutally use.
      */
     FactorContainer factors;

     //WEIGHTS DEPRECATED!
     double _approximate_distance;

};
