/**
 * \file        CognitiveMap.h
 * \date        Jan 1, 2014
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * Cognitive Map models the pedestrian knowledge of building space in simulation.
 *
 *
 **/


#ifndef COGNITIVEMAP_H_
#define COGNITIVEMAP_H_

#include "NavigationGraph.h"

class SubRoom;
class Transition;
class Crossing;
class Building;
class NavLine;
class Pedestrian;



/**
 * @brief Cognitive Map
 *
 * The CognitiveMap is a simplified representation of human knowledge of building space.
 * It takes the different quantity and certainty of knowledge into account.
 * It is the basis for further developments like individual behavior and information sharing algorithms.
 *
 */
class CognitiveMap {

public:
     /****************************
      * Constructors & Destructors
      ****************************/
     CognitiveMap(const Building * building, const Pedestrian * pedestrian);
     virtual ~CognitiveMap();

     void Add(const SubRoom * sub_room);
     void Add(const Crossing * crossing);
     void AddExit(const Transition * exit);

     NavigationGraph::VerticesContainer * GetAllVertices();

     const NavigationGraph * GetNavigationGraph() const;

     const NavLine * GetDestination();
     const NavLine * GetLocalDestination();

private:
     NavigationGraph * navigation_graph;
     const Building * const building;
     const Pedestrian * const pedestrian;

};


#endif /* COGNITIVEMAP_H_ */
