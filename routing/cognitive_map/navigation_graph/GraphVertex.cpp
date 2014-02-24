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
    return false;
}

std::pair<const GraphEdge *, double> GraphVertex::GetCheapestDestination(std::set<const GraphVertex *> visited) const
{
    if(visited.find(this) != visited.end()) {
        return std::pair<const GraphEdge *, double>(NULL, INFINITY);
    }

    double act_cheapest_dist = INFINITY;
    const GraphEdge * act_cheapest_edge;

    visited.insert(this);

    for(EdgesContainer::const_iterator it = out_edges.begin(); it != out_edges.end(); ++it) {
        if((*it)->GetCrossing()->IsExit()) {
            if((*it)->GetApproximateDistance() < act_cheapest_dist) {
                act_cheapest_edge = (*it);
                act_cheapest_dist = (*it)->GetApproximateDistance();
            }
        } else {
            std::pair<const GraphEdge *, double> cheapest_destination = (*it)->GetDest()->GetCheapestDestination(visited);
            if(cheapest_destination.second < act_cheapest_dist) {
                act_cheapest_dist = cheapest_destination.second + cheapest_destination.first->GetApproximateDistance();
                act_cheapest_edge = (*it);
            }
        }
    }
    return std::pair<const GraphEdge *, double>(act_cheapest_edge, act_cheapest_dist);
}
