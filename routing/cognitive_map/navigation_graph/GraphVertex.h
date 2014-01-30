/**
 * @file   GraphVertex.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  Vertex of a Graph.
 *
 */

#ifndef GRAPHVERTEX_H_
#define GRAPHVERTEX_H_

#include <unordered_map>
#include "GraphEdge.h"

class SubRoom;
class NavLine;


/**
 * @brief Graph Vertex.
 *
 */
class GraphVertex {

public:
    /****************************
     * Constructors & Destructors
     ****************************/

    GraphVertex(NavLine const * const nl);
    virtual ~GraphVertex();

    // add and remove edge pointer from vertex
    void AddOutEdge(GraphVertex const * const dest, SubRoom const * const sr);
    int RemoveOutEdge(GraphVertex * dest);
    int RemoveOutEdge(GraphEdge * edge);



private:
    // edges wich are "known" from this vertex
    std::unordered_map<GraphVertex *, GraphEdge> out_edges;
    const NavLine * const nav_line;

};

#endif /* GRAPHVERTEX_H_ */
