/**
 * @file   GraphVertex.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 *
 */

#include "GraphVertex.h"


#include "../../../geometry/SubRoom.h"
#include "../NavigationGraph.h"

#include <unordered_map>

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
    out_edges.emplace(dest, new GraphEdge(this, dest, crossing));
    return;
}

int GraphVertex::RemoveOutEdge(GraphEdge * edge)
{
    EdgesContainer::iterator it = out_edges.find(edge->GetDest());
    if(it != out_edges.end())
        delete it->second;
    return out_edges.erase(edge->GetDest());
}

int GraphVertex::RemoveOutEdge(const GraphVertex * dest)
{
    return out_edges.erase(dest);

}

const std::string GraphVertex::GetCaption() const
{
    return  std::to_string(sub_room->GetRoomID()) + "" + std::to_string(sub_room->GetSubRoomID());
}

const GraphVertex::EdgesContainer * GraphVertex::GetAllOutEdges() const
{
    return &out_edges;
}
