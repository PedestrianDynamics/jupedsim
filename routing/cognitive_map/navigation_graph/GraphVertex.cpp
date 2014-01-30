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

GraphVertex::~GraphVertex()
{
    return;
};

void GraphVertex::AddOutEdge(GraphVertex const * const dest, SubRoom const * const sr)
{
    out_edges.emplace(this, this, dest, sr);
    return;

}

int GraphVertex::RemoveOutEdge(GraphEdge * edge)
{
    //  return out_edges.erase(edge->dest);
    return 2;

}

int GraphVertex::RemoveOutEdge(GraphVertex * dest)
{
    // return out_edges.erase(dest);
    return 2;


}
