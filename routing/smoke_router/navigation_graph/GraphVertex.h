/**
 * \file        GraphVertex.h
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
 * Vertex of a Graph.
 *
 *
 **/
#pragma once

#include "geometry/Line.h"

#include <unordered_map>
#include <set>
#include <string>
#include <utility>

class SubRoom;
class NavLine;
class Crossing;
class Transition;
class GraphEdge;
class GraphVertex;

/**
 * @brief Graph Vertex.
 *
 */
class GraphVertex {

public:
     typedef std::set<GraphEdge *> EdgesContainer;

     /****************************
      * Constructors & Destructors
      ****************************/

     GraphVertex(const SubRoom * const sub_room);
     GraphVertex(GraphVertex const & gv);

     virtual ~GraphVertex();

     const std::string GetCaption() const;
     const SubRoom * GetSubRoom() const;

    GraphEdge * operator[](const Crossing *);
    // add and remove edge pointer from vertex

     void AddOutEdge(const GraphVertex * const dest, const Crossing * const crossing);
     int RemoveOutEdge(const GraphVertex * dest);
     int RemoveOutEdge(GraphEdge * edge);
     const EdgesContainer * GetAllOutEdges() const;
     EdgesContainer * GetAllEdges();


     void AddExit(const Transition * transition);
     bool HasExit() const;


    const GraphEdge * GetCheapestDestinationByEdges(const Point & position) const;
    const GraphEdge * GetLocalCheapestDestination(const Point & position) const;




private:
     // edges wich are "known" from this vertex
     EdgesContainer out_edges;
     std::set<GraphEdge *> exits;
     const SubRoom * const sub_room;

};
