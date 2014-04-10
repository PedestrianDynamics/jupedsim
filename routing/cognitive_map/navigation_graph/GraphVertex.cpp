/**
 * @file   GraphVertex.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 *
 */

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
#include <algorithm>

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

GraphEdge * GraphVertex::operator[](const SubRoom * const sub_room)
{
    for(EdgesContainer::iterator it = out_edges.begin(); it != out_edges.end(); ++it) {
        if((*it)->GetDest() != NULL && (*it)->GetDest()->GetSubRoom() == sub_room) {
            return (*it);
        }
    }
    return NULL;
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
    for(EdgesContainer::const_iterator it = out_edges.begin(); it != out_edges.end(); ++it)
    {
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
            return exit_edge;
        } else {
            while(this != act_edge->GetSrc()) {
                act_edge = destinations[act_edge].first;
            }
            return act_edge;
        }
    } else {
        return NULL;
    }

}

const GraphEdge * GraphVertex::GetLocalCheapestDestination(const Point & position) const
{
    std::priority_queue<
        std::pair<double, const GraphEdge *>,
        vector<std::pair<double, const GraphEdge *>>,
        std::greater<std::pair<double, const GraphEdge *>>
        > edges;
    for(EdgesContainer::const_iterator it = this->GetAllOutEdges()->begin(); it != this->GetAllOutEdges()->end(); ++it) {
        edges.push(std::make_pair((*it)->GetFactor(), (*it)));
    }

    if(edges.size() == 1) return edges.top().second;
    if(edges.size() > 1) {
        const GraphEdge * act_edge = NULL;
        //take the best 3 edges and choose the nearest
        for(int i = 1; i<= std::min(3, (int) edges.size()); i++) {
            const GraphEdge * edge = edges.top().second;
            edges.pop();
            if(act_edge == NULL || act_edge->GetWeight(position) > edge->GetWeight(position)) {
                act_edge = edge;
            }
        }
        return act_edge;
    }


}
