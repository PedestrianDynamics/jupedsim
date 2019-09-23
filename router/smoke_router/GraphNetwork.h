/**
 * \file        GraphNetwork.h
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
 * Cognitive Map models the pedestrian knowledge of building space in simulation.
 *
 *
 **/
#pragma once

#include "NavigationGraph.h"

#include <vector>

class SubRoom;
class Transition;
class Crossing;
class Building;
class NavLine;
class Pedestrian;



/**
 * @brief Graph Network
 *
 * The GraphNetwork is a simplified representation of human knowledge of building space.
 * It takes the different quantity and certainty of knowledge into account.
 * It is the basis for further developments like individual behavior and information sharing algorithms.
 *
 */
class GraphNetwork {

public:
     /****************************
      * Constructors & Destructors
      ****************************/
     GraphNetwork(const Building* buildg, const Pedestrian* ped);
     virtual ~GraphNetwork();

     void Add(const SubRoom * sub_room);
     void Add(const Crossing * crossing);
     void AddExit(const Transition * exit);

     NavigationGraph::VerticesContainer * GetAllVertices();

    NavigationGraph * GetNavigationGraph() const;

    const GraphEdge * GetDestination() const;
    const GraphEdge * GetLocalDestination();

    bool HadNoDestination() const;
    void AddDestination(const GraphEdge *);
    std::vector<const GraphEdge *> & GetDestinations();

    bool ChangedSubRoom() const;
    void UpdateSubRoom();

private:
    NavigationGraph * navigation_graph;
    const Building* building;
    const Pedestrian* pedestrian;
    const SubRoom * current_subroom = nullptr;
    std::vector<const GraphEdge *> destinations;
};
