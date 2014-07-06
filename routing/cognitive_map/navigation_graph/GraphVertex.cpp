/**
 * \file        GraphVertex.cpp
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
 *
 *
 **/


#include "GraphVertex.h"

#include <utility>
#include <cmath>
#include <set>
#include <map>
#include <queue>
#include <functional>

#include "GraphEdge.h"
#include "../../../geometry/SubRoom.h"
#include "../NavigationGraph.h"
#include "../../../geometry/Transition.h"


using namespace std;

/**
 * Constructors & Destructors
 */

GraphVertex::GraphVertex(const SubRoom * const sr)
     : sub_room(sr)
{
}


GraphVertex::GraphVertex(GraphVertex const & gv)
     : sub_room(gv.sub_room)
{
}

GraphVertex::~GraphVertex()
{
     return;
}

void GraphVertex::AddOutEdge(const GraphVertex * const dest, const Crossing * const crossing)
{
     out_edges.insert(new GraphEdge(this, dest, crossing));
     return;
}


void GraphVertex::AddExit(const Transition * transition)
{
     out_edges.insert(new GraphEdge(this, NULL, transition));
     return;
}

int GraphVertex::RemoveOutEdge(GraphEdge * edge)
{
     EdgesContainer::iterator it = out_edges.find(edge);
     if(it != out_edges.end()) {
          delete (*it);
          out_edges.erase(it);
          return 1;
     }
     return 0;
}


int GraphVertex::RemoveOutEdge(const GraphVertex * dest)
{
     //return out_edges.erase(dest);
     return 1;

}

const std::string GraphVertex::GetCaption() const
{
     return  std::to_string(sub_room->GetRoomID()) + "" + std::to_string(sub_room->GetSubRoomID());
}

const GraphVertex::EdgesContainer * GraphVertex::GetAllOutEdges() const
{
     return &out_edges;
}


GraphVertex::EdgesContainer * GraphVertex::GetAllEdges()
{
     return &out_edges;
}

const SubRoom * GraphVertex::GetSubRoom() const
{
     return sub_room;
}

bool GraphVertex::HasExit() const
{
     for(EdgesContainer::const_iterator it = out_edges.begin(); it != out_edges.end(); ++it) {
          if((*it)->IsExit()) return true;
     }
     return false;
}

/**
 * this is a modified version of disjkstra. it considers that the edges length depends on the predecessor edge!
 * this is used to calculate the "exact" path length.
 * for this vertices and edges are exchanged in dijkstra algorithm structure.
 * we do not visit vertices, but edges. after that we identify the edge.destination with the corresponding vertex.
 */
std::pair<const GraphEdge *, double> GraphVertex::GetCheapestDestinationByEdges(const Point & position) const
{
     std::set<const GraphEdge *> visited;
     // map with GrapEdges and their predecessors and distances
     std::map<const GraphEdge *,  std::pair<const GraphEdge *, double>> destinations;
     // priority queue with discovered Edges and their distance.
     std::priority_queue<
     std::pair<double, const GraphEdge *>,
         vector<std::pair<double, const GraphEdge *>>,
         std::greater<std::pair<double, const GraphEdge *>>
         > queue;
     const GraphEdge * exit_edge = NULL;


     // add all out edges from this vertex to priority queue and destinations.
     for(EdgesContainer::const_iterator it = this->GetAllOutEdges()->begin(); it != this->GetAllOutEdges()->end(); ++it) {
          double new_distance = (*it)->GetWeight(position);

          destinations[(*it)] = std::make_pair((const GraphEdge*) NULL, new_distance);
          queue.push(std::make_pair(new_distance, (*it)));
     }

     while(!queue.empty()) {
          const GraphEdge * act_edge = queue.top().second;
          double act_distance  = queue.top().first;
          queue.pop();

          //if we discovered an exit edge we are finished (queue is distance ordered)
          if(act_edge->IsExit()) {
               exit_edge = act_edge;
               break;
          }

          //discover new edges or shorter paths to old edges
          const EdgesContainer * new_edges = act_edge->GetDest()->GetAllOutEdges();

          for(EdgesContainer::const_iterator it = new_edges->begin(); it != new_edges->end(); ++it) {
               // if the destination edges was visited we already have the shortest path to this destination.
               if(visited.find((*it)) != visited.end()) continue;

               double new_distance = act_distance + (*it)->GetWeight(act_edge->GetCrossing()->GetCentre());
               //check if the destination edge was discovered before.
               if(destinations.find((*it)) == destinations.end()) {
                    //initialize the new discovered vertex with distance inifity and push it to the queue
                    destinations[(*it)] = std::make_pair<const GraphEdge*, double>(NULL, INFINITY);
                    queue.push(std::make_pair(new_distance, (*it)));
               }
               //check if we found a shorter path to the dest vertex
               if(destinations[(*it)].second > new_distance) {
                    destinations[(*it)].second = new_distance;
                    destinations[(*it)].first = act_edge;
               }
          }
          visited.insert(act_edge);
     }
     //did we found an exits?
     if(exit_edge != NULL) {
          const GraphEdge * act_edge = destinations[exit_edge].first;
          if(act_edge == NULL) {
               return std::make_pair(exit_edge, destinations[exit_edge].second);
          } else {
               while(this != act_edge->GetSrc()) {
                    act_edge = destinations[act_edge].first;
               }
               return std::make_pair(act_edge, destinations[exit_edge].second);
          }
     } else {
          return std::make_pair<const GraphEdge*, double>(NULL, INFINITY);
     }

}


std::pair<const GraphEdge *, double> GraphVertex::GetCheapestDestination(const Point & position) const
{
     std::set<const GraphVertex *> visited;
     std::map<const GraphVertex *,  std::pair<const GraphEdge *, double>> destinations;
     std::priority_queue<std::pair<double, const GraphVertex *>, vector<std::pair<double, const GraphVertex *>>, std::greater<std::pair<double, const GraphVertex *>>> queue;

     destinations[this] = std::make_pair<const GraphEdge *, double>(NULL, 0.0);


     //first edges should be handled separated to recalculate edge weight
     for(EdgesContainer::const_iterator it = this->GetAllOutEdges()->begin(); it != this->GetAllOutEdges()->end(); ++it) {
          double new_distance = (*it)->GetWeight(position);
          //check if the destination vertex was discovered before.
          if(destinations.find((*it)->GetDest()) == destinations.end()) {
               //initialize the new discovered vertex with distance inifity and push it to the queue
               destinations[(*it)->GetDest()] = std::make_pair<const GraphEdge*, double>(NULL, INFINITY);
               queue.push(std::make_pair(new_distance, (*it)->GetDest()));
          }

          //check if we found a shorter path to the dest vertex
          if(destinations[(*it)->GetDest()].second > new_distance) {
               destinations[(*it)->GetDest()].second = new_distance;
               destinations[(*it)->GetDest()].first = (*it);
          }
     }
     visited.insert(this);

     while(!queue.empty()) {
          const GraphVertex * act_vertex = queue.top().second;
          double act_distance = queue.top().first;
          queue.pop();
          const EdgesContainer * act_edges = act_vertex->GetAllOutEdges();

          // NULL Pointer is used for exit node
          // this only works if the priority queue is sorted with lowest weight.
          // so if we have null pointer as act. vertex there is no shorter path to the exit
          if(act_vertex == NULL) {
               break;
          }

          for(EdgesContainer::const_iterator it = act_edges->begin(); it != act_edges->end(); ++it) {
               // if the dest vertex was visited we already have the shortest path to this dest.
               if(visited.find((*it)->GetDest()) != visited.end()) continue;

               double new_distance = act_distance + (*it)->GetWeight((*it)->GetSrc()->GetSubRoom()->GetCentroid());
               //check if the destination vertex was discovered before.
               if(destinations.find((*it)->GetDest()) == destinations.end()) {
                    //initialize the new discovered vertex with distance inifity and push it to the queue
                    destinations[(*it)->GetDest()] = std::make_pair<const GraphEdge*, double>(NULL, INFINITY);
                    queue.push(std::make_pair(new_distance, (*it)->GetDest()));
               }

               //check if we found a shorter path to the dest vertex
               if(destinations[(*it)->GetDest()].second > new_distance) {
                    destinations[(*it)->GetDest()].second = new_distance;
                    destinations[(*it)->GetDest()].first = (*it);
               }
          }
          visited.insert(act_vertex);
     }
     //NULL Pointer is outside the building, so the edge is the Exit
     if(destinations.find(NULL) != destinations.end()) {
          const GraphEdge * act_edge = destinations[NULL].first;
          while(this != act_edge->GetSrc()) {
               act_edge = destinations[act_edge->GetSrc()].first;
          }
          return std::make_pair(act_edge, destinations[NULL].second);
     } else {
          return std::make_pair<const GraphEdge*, double>(NULL, INFINITY);
     }
}
