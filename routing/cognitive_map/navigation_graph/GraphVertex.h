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
#include <set>
#include <string>
#include <utility>

#include "../../../geometry/Line.h"

class SubRoom;
class NavLine;
class Crossing;
class Transition;
class GraphEdge;

/**
 * @brief Graph Vertex.
 *
 */
class GraphVertex {

public:
typedef std::set<GraphEdge *> EdgesContainer;
    /****************************
     * Constructors & Destructors
     ****************************/

    GraphVertex(const SubRoom * const sub_room);
    GraphVertex(GraphVertex const & gv);

    virtual ~GraphVertex();

    const std::string GetCaption() const;
    const SubRoom * GetSubRoom() const;

    // add and remove edge pointer from vertex

    void AddOutEdge(const GraphVertex * const dest, const Crossing * const crossing);
    int RemoveOutEdge(const GraphVertex * dest);
    int RemoveOutEdge(GraphEdge * edge);
    const EdgesContainer * GetAllOutEdges() const;
    EdgesContainer * GetAllEdges();


    void AddExit(const Transition * transition);
    bool HasExit() const;


    const GraphEdge * GetCheapestDestinationByEdges(const Point & position) const;
    const GraphEdge * GetLocalCheapestDestination(const Point & position) const;


private:
    // edges wich are "known" from this vertex
    EdgesContainer out_edges;
    std::set<GraphEdge *> exits;
    const SubRoom * const sub_room;

};

#endif /* GRAPHVERTEX_H_ */
