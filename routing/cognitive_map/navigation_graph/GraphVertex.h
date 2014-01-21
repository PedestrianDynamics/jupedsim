/**
 * @file   GraphVertex.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  Vertex of a Graph.
 *
 */

#ifndef GRAPHVERTEX_H_
#define GRAPHVERTEX_H_

#include <map>
#include "GraphEdge.h"
/**
 * @brief Graph Vertex.
 *
 */

class GraphVertex {

public:
    /****************************
     * Constructors & Destructors
     ****************************/

    GraphVertex();
    virtual ~GraphVertex();


private:
    std::map<int, GraphEdge> edges;

};


#endif /* GRAPHVERTEX_H_ */
