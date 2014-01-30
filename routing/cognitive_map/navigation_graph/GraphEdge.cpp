/**
 * @file   GraphEdge.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 *
 */

#include "GraphEdge.h"

#include "GraphVertex.h"
#include "../../../geometry/SubRoom.h"

using namespace std;

/**
 * Constructors & Destructors
 */

GraphEdge::~GraphEdge()
{

    return;
};
GraphEdge::GraphEdge(GraphVertex const * const s, GraphVertex const  * const d, SubRoom const * const  sr)
    : src(s), dest(d), subroom(sr)
{
}
