/**
 * @file   NavigationGraph.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  The Navigation Graph is the metric abstraction layer in the cognitive map.
 *
 */

#include "NavigationGraph.h"

#include "navigation_graph/GraphEdge.h"
#include "../../geometry/Building.h"

using namespace std;

/**
 * Constructors & Destructors
 */

NavigationGraph::NavigationGraph(const Building * const b)
    : building(b)
{
}

NavigationGraph::NavigationGraph(const NavigationGraph & ng)
    : building(ng.building)
{
}

NavigationGraph::~NavigationGraph()
{
}

void NavigationGraph::AddVertex(NavLine const * const nl)
{
    vertices.emplace(nl, new GraphVertex(nl));
    return;
}

void NavigationGraph::AddEdge(NavLine const * const src, NavLine const * const dest, SubRoom const * const sr)
{
    VerticesContainer::iterator src_it = vertices.find(src);
    VerticesContainer::iterator dest_it = vertices.find(dest);

    if(src_it != vertices.end() && dest_it != vertices.end())
    {
        src_it->second->AddOutEdge(dest_it->second, sr);
    }
}
