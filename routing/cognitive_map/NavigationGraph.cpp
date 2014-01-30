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

NavigationGraph::NavigationGraph()
    : building(b)
{

}

NavigationGraph::~NavigationGraph()
{

}

void NavigationGraph::AddVertex(NavLine const * const nl)
{
    vertices.emplace(nl, GraphVertex(nl));
    return;
}


void NavigationGraph::AddEdge(NavLine* src, NavLine * dest, SubRoom * sr)
{

}
