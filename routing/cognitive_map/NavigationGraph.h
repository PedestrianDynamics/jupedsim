/**
 * @file   NavigationGraph.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  The Navigation Graph is the metric abstraction layer in the cognitive map.
 *
 */

#ifndef NAVIGATIONGRAPH1_H_
#define NAVIGATIONGRAPH1_H_

#include <boost/graph/adjacency_list.hpp>

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS> nav_graph;

/**
 * @brief Cognitive Map
 *
 * The Navigation Graph represents the metric layer in a cognitive map.
 * It is modeled as a graph with weights and information's related to the edges and vertexes.
 * The Navigation Graph could be sparse.
 *
 * The BOOST Graph Library is used for the graph structure.
 *
 */

class NavigationGraph {

public:
    /****************************
     * Constructors & Destructors
     ****************************/

    NavigationGraph();

    virtual ~NavigationGraph();


private:


};


#endif /* NAVIGATIONGRAPH1_H_ */
