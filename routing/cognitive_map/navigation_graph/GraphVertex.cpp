/**
 * @file   GraphVertex.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 *
 */

#include "GraphVertex.h"


#include "../../../geometry/NavLine.h"
#include "../NavigationGraph.h"

#include <unordered_map>

using namespace std;

/**
 * Constructors & Destructors
 */

GraphVertex::GraphVertex(NavLine const * const nl)
    : nav_line(nl)
{
}


GraphVertex::GraphVertex(GraphVertex const & gv)
    : nav_line(gv.nav_line)
{
}

GraphVertex::~GraphVertex()
{
    return;
};

void GraphVertex::AddOutEdge(GraphVertex const * const dest, SubRoom const * const sr)
{
    out_edges.emplace(this, new GraphEdge(this, dest, sr));
    return;
}

int GraphVertex::RemoveOutEdge(GraphEdge * edge)
{
    EdgesContainer::iterator it = out_edges.find(edge->getDest());
    if(it != out_edges.end())
        delete it->second;
    return out_edges.erase(edge->getDest());
}

int GraphVertex::RemoveOutEdge(const GraphVertex * dest)
{
    return out_edges.erase(dest);

}
