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

            double new_distance = act_distance + (*it)->GetWeight();
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
