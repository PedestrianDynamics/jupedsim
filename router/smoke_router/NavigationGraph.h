/**
 * \file        NavigationGraph.h
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
 * The Navigation Graph is the metric abstraction layer in the cognitive map.
 *
 *
 **/
#pragma once

#include "router/smoke_router/navigation_graph/GraphVertex.h"
#include "router/smoke_router/navigation_graph/GraphEdge.h"

#include <string>
#include <unordered_map>

class Building;
class NavLine;
class SubRoom;
class Crossing;
class Transition;

/**
 * @brief Navigation Graph
 *
 * The Navigation Graph represents the metric layer in a cognitive map.
 * It is modeled as a graph with weights and information's related to the edges and vertexes.
 * The Navigation Graph could be sparse.
 *
 *
 */

class NavigationGraph {
public:
     typedef std::unordered_map<const SubRoom * , GraphVertex *> VerticesContainer;

     /****************************
      * Constructors & Destructors
      ****************************/
     NavigationGraph(const Building * building);
     NavigationGraph(const NavigationGraph & ng);

     virtual ~NavigationGraph();

     void AddVertex(const SubRoom * const sub_room);
     void AddEdge(const Crossing * crossing);
     void AddExit(const Transition * transition);

     GraphVertex * operator[](const SubRoom * const sub_room);

     NavigationGraph::VerticesContainer * GetAllVertices();

     void WriteToDotFile(const std::string filepath) const;


private:
     /**
      * Vertices and Edges
      */
     NavigationGraph::VerticesContainer vertices;
     const Building * const building;


};
