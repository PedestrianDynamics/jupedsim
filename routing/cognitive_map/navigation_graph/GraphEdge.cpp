/**
 * @file   GraphEdge.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 *
 */

#include "GraphEdge.h"

#include "GraphVertex.h"
#include "../../../geometry/SubRoom.h"
#include "../../../geometry/Crossing.h"

using namespace std;

/**
 * Constructors & Destructors
 */

GraphEdge::~GraphEdge()
{
    return;
}

GraphEdge::GraphEdge(const GraphVertex * const s, const GraphVertex  * const d, const Crossing * const crossing)
    : src(s), dest(d), crossing(crossing)
{
}

GraphEdge::GraphEdge(GraphEdge const &ge)
    : src(ge.src), dest(ge.dest), crossing(ge.crossing)
{
}
const GraphVertex * GraphEdge::GetDest() const
{
    return dest;
}
const GraphVertex * GraphEdge::GetSrc() const
{
    return src;
}

const Crossing * GraphEdge::GetCrossing() const
{
    return crossing;
}
