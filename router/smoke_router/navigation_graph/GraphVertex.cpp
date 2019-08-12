/**
 * \file        GraphVertex.cpp
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
#include "GraphVertex.h"

#include "GraphEdge.h"

#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "router/smoke_router/NavigationGraph.h"

#include <queue>

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
    //check if an edge through the same crossing already exists.
    for(EdgesContainer::iterator it = out_edges.begin(); it != out_edges.end(); ++it) {
        //if we find an edge with the same crossing we could jump out here!
        if((*it)->GetCrossing() == crossing) return;
    }
    //no edge found, so add it.
    out_edges.insert(new GraphEdge(this, dest, crossing));
}


void GraphVertex::AddExit(const Transition * transition)
{
    //check if an edge through the same crossing already exists.
    for(EdgesContainer::iterator it = out_edges.begin(); it != out_edges.end(); ++it) {
        //if we find an edge with the same crossing we could jump out here!
        if((*it)->GetCrossing() == transition) return;
    }
    out_edges.insert(new GraphEdge(this, nullptr, transition));
    return;
}

GraphEdge * GraphVertex::operator[](const Crossing * crossing)
{
    for(EdgesContainer::iterator it = out_edges.begin(); it != out_edges.end(); ++it) {
        if((*it)->GetDest() != nullptr && (*it)->GetCrossing() == crossing) {
            return (*it);
        }
    }
    return nullptr;
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


int GraphVertex::RemoveOutEdge(const GraphVertex * /*dest*/)
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
const GraphEdge * GraphVertex::GetCheapestDestinationByEdges(const Point & position) const
{
    std::set<const GraphEdge *> visited;
    // map with GrapEdges and their predecessors and distances
    std::map<const GraphEdge *,  std::pair<const GraphEdge *, double> > destinations;
    // priority queue with discovered Edges and their distance.
    std::priority_queue<
        std::pair<double, const GraphEdge *>,
        std::vector<std::pair<double, const GraphEdge *> >,
        std::greater<std::pair<double, const GraphEdge *> >
        > queue;
    const GraphEdge * exit_edge = nullptr;


    // add all out edges from this vertex to priority queue and destinations.
    for(EdgesContainer::const_iterator it = this->GetAllOutEdges()->begin(); it != this->GetAllOutEdges()->end(); ++it) {
        double new_distance = (*it)->GetWeight(position);

        destinations[(*it)] = std::make_pair((const GraphEdge*) nullptr, new_distance);
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
            if(visited.find((*it)) != visited.end() || (*it)->GetDest() == act_edge->GetSrc()) continue;

            double new_distance = act_distance + (*it)->GetWeight(act_edge->GetCrossing()->GetCentre());
            //check if the destination edge was discovered before.
            if(destinations.find((*it)) == destinations.end()) {
                //initialize the new discovered vertex with distance inifity and push it to the queue
                destinations[(*it)] = std::make_pair<const GraphEdge*, double>(nullptr, INFINITY);
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
    //did we found an exit?
    if(exit_edge != nullptr) {
        const GraphEdge * act_edge = destinations[exit_edge].first;

        if(act_edge == nullptr) {
            return exit_edge;
        } else {
            while(this != act_edge->GetSrc()) {
                act_edge = destinations[act_edge].first;
            }
            return act_edge;
        }
    } else {
        return nullptr;
    }

}

const GraphEdge * GraphVertex::GetLocalCheapestDestination(const Point & position) const
{
    //int maximum_factor_distance = 1;

    std::priority_queue<
        std::pair<double, const GraphEdge *>,
        std::vector<std::pair<double, const GraphEdge *> >,
        std::greater<std::pair<double, const GraphEdge *> >
        > edges;

    std::priority_queue<
        std::pair<double, const GraphEdge *>,
        std::vector<std::pair<double, const GraphEdge *> >,
        std::greater<std::pair<double, const GraphEdge *> >
        > exitEdges;

    std::priority_queue<
        std::pair<double, const GraphEdge *>,
        std::vector<std::pair<double, const GraphEdge *> >,
        std::greater<std::pair<double, const GraphEdge *> >
        > sameFactorEdges;

    for(EdgesContainer::const_iterator it = this->GetAllOutEdges()->begin(); it != this->GetAllOutEdges()->end(); ++it) {
        if ((*it)->GetCrossing()->IsExit())
            exitEdges.push(std::make_pair((*it)->GetWeight(position), (*it)));
         edges.push(std::make_pair((*it)->GetFactor(), (*it)));
    }

    // if exit(s) are available
    if (exitEdges.size()==1)
        return exitEdges.top().second;

//    else if (exitEdges.size()>1)
//    {
//        /*
//        // if two edges possess the same (lowest) weight
//        for(EdgesContainer::const_iterator itedge = this->GetAllOutEdges()->begin(); itedge != this->GetAllOutEdges()->end(); ++itedge)
//        {
//            if ((*itedge)->GetCrossing()->IsExit())
//            {

//                sameFactorEdges.push(std::make_pair((*itedge)->GetWeight(position),(*itedge)));
//            }
//            //else if ((*itedge)->GetFactor()==edges.top().second->GetFactor())
//             //   sameFactorEdges.push(std::make_pair((*itedge)->GetApproximateDistance(position),(*itedge)));
//        }*/
//    }
    else if (exitEdges.empty())
    {
        // if two edges possess the same (lowest) weight
        for(EdgesContainer::const_iterator itedge = this->GetAllOutEdges()->begin(); itedge != this->GetAllOutEdges()->end(); ++itedge)
        {
            if ((*itedge)->GetFactor()==edges.top().second->GetFactor())
                sameFactorEdges.push(std::make_pair((*itedge)->GetApproximateDistance(position),(*itedge)));
            //else if ((*itedge)->GetFactor()==edges.top().second->GetFactor())
             //   sameFactorEdges.push(std::make_pair((*itedge)->GetApproximateDistance(position),(*itedge)));
        }
    }

    if(sameFactorEdges.size() >= 1)
        return sameFactorEdges.top().second;
    else
        return edges.top().second;


//    if(edges.size() > 1) {
//        double best_factor = edges.top().first;
//        const GraphEdge * act_edge = nullptr;
//        //take the best  edges and choose the nearest
//        while(!edges.empty()) {

//            //Log->Write("Best factor: %f ; act edge factor: %f", best_factor, edges.top().first);

//            //if the factor is worse than maximum_factor_distance times the best_factor the edges are rejected
//            if(edges.top().first > maximum_factor_distance * best_factor) break;

//            if(act_edge == nullptr || act_edge->GetWeight(position) > edges.top().second->GetWeight(position)) {
//                act_edge = edges.top().second;
//            }
//            else if(act_edge->GetWeight(position) == edges.top().second->GetWeight(position))
//            {
//                if(act_edge->GetApproximateDistance(position) > edges.top().second->GetApproximateDistance(position))
//                    act_edge = edges.top().second;
//            }
//            edges.pop();
//        }

//        return act_edge;
//    }

//}

}
