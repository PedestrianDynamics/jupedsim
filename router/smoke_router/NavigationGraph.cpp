/**
 * \file        NavigationGraph.cpp
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
#include "NavigationGraph.h"

#include "geometry/Building.h"
#include "geometry/SubRoom.h"

/**
 * Constructors & Destructors
 */

NavigationGraph::NavigationGraph(const Building * Building)
     : building(Building)
{
}

NavigationGraph::NavigationGraph(const NavigationGraph & ng)
     : building(ng.building)
{
}

NavigationGraph::~NavigationGraph()
{
     //remove all vertices
     for(VerticesContainer::iterator it = vertices.begin(); it != vertices.end(); ++it) {
          delete it->second;
     }
}

void NavigationGraph::AddVertex(const SubRoom * const sub_room)
{
     vertices.insert(std::pair<const SubRoom  *, GraphVertex*>(sub_room, new GraphVertex(sub_room)));
}

void NavigationGraph::AddEdge(const Crossing * crossing)
{

    this->operator[](crossing->GetSubRoom1())->AddOutEdge(this->operator[](crossing->GetSubRoom2()), crossing);
    this->operator[](crossing->GetSubRoom2())->AddOutEdge(this->operator[](crossing->GetSubRoom1()), crossing);
//    dest_it->second->AddOutEdge(src_it->second, crossing);

}

void NavigationGraph::AddExit(const Transition * transition)
{
//     VerticesContainer::iterator src_it = vertices.find(transition->GetSubRoom1());
//     if(src_it != vertices.end()) {
//          src_it->second->AddExit(transition);
//     }
    this->operator[](transition->GetSubRoom1())->AddExit(transition);

}

GraphVertex * NavigationGraph::operator[](const SubRoom * const sub_room)
{
     VerticesContainer::iterator it = vertices.find(sub_room);
     if(it == vertices.end()) {
          AddVertex(sub_room);
     }
     return vertices[sub_room];
}

NavigationGraph::VerticesContainer * NavigationGraph::GetAllVertices()
{
     return & vertices;
}

void NavigationGraph::WriteToDotFile(const std :: string filepath) const
{
    std::ofstream dot_file;
    dot_file.open (filepath + "navigation_graph.dot");
    dot_file << " digraph graphname \n {\n";
    for(VerticesContainer::const_iterator it = vertices.begin(); it != vertices.end(); ++it) {
        dot_file << it->second->GetCaption();
        dot_file << " [ \n pos =\"" + std::to_string(it->second->GetSubRoom()->GetCentroid()._x) +"," + std::to_string(it->second->GetSubRoom()->GetCentroid()._y) +"!\"\n shape=box \n";
        if(it->second->HasExit())
            dot_file << "style=filled, color=red\n";

        dot_file << "]\n" ;
        const GraphVertex::EdgesContainer * edges = it->second->GetAllOutEdges();
        for(GraphVertex::EdgesContainer::const_iterator it2 = edges->begin(); it2 != edges->end(); ++it2)
        {
            if(!(*it2)->GetCrossing()->IsExit()) {
                dot_file << it->second->GetCaption() + " -> " + (*it2)->GetDest()->GetCaption() + "\n [";
                dot_file << "label = "+ std::to_string((*it2)->GetFactor()) + "] \n";
            }
        }

     }
     dot_file << "} \n";

     dot_file.close();
     return;
}
