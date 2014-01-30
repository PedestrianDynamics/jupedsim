/**
 * @file   NavigationGraph.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  The Navigation Graph is the metric abstraction layer in the cognitive map.
 *
 */

#ifndef NAVIGATIONGRAPH1_H_
#define NAVIGATIONGRAPH1_H_

#include <unordered_map>
#include "navigation_graph/GraphVertex.h"

class Building;
class NavLine;
class SubRoom;

/**
 * @brief Navigation Graph
 *
 * The Navigation Graph represents the metric layer in a cognitive map.
 * It is modeled as a graph with weights and information's related to the edges and vertexes.
 * The Navigation Graph could be sparse.
 *
 *
 */

class NavigationGraph {

public:
    /****************************
     * Constructors & Destructors
     ****************************/

    NavigationGraph(Building const * const b);
    virtual ~NavigationGraph();

    void AddVertex(NavLine const * const nl);

    void AddEdge(NavLine const * const src, NavLine const * const dest, SubRoom const * const sr);



private:
    /**
     * Vertices and Edges
     */
    std::unordered_map<NavLine const * , GraphVertex> vertices;

    Building const * const building;

};


#endif /* NAVIGATIONGRAPH1_H_ */
