/**
 * @file   GraphEdge.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 *
 */

#include "GraphEdge.h"

#include<vector>

#include "GraphVertex.h"
#include "../../../geometry/SubRoom.h"
#include "../../../geometry/Crossing.h"
#include "../../../geometry/Transition.h"

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
    CalcApproximateDistance();
}

GraphEdge::GraphEdge(GraphEdge const &ge)
    : src(ge.src), dest(ge.dest), crossing(ge.crossing), approximate_distance(ge.approximate_distance)
{
}

void GraphEdge::CalcApproximateDistance()
{
    double distance = 0.0;
    int count = 0;
    for(std::vector<Crossing*>::const_iterator it = src->GetSubRoom()->GetAllCrossings().begin(); it != src->GetSubRoom()->GetAllCrossings().end(); ++it) {
        if(crossing->GetUniqueID() == (*it)->GetUniqueID()) continue;
        if(GetDest() != NULL && ((*it)->GetSubRoom1() == GetDest()->GetSubRoom() || (*it)->GetSubRoom2() == GetDest()->GetSubRoom())) continue;
        count++;
        distance = distance + (((*it)->GetCentre() - crossing->GetCentre()).Norm());
    }

    for(std::vector<Transition*>::const_iterator it = src->GetSubRoom()->GetAllTransitions().begin(); it != src->GetSubRoom()->GetAllTransitions().end(); ++it) {
        if(crossing->GetUniqueID() == (*it)->GetUniqueID()) continue;
        if(GetDest() != NULL && ((*it)->GetSubRoom1() == GetDest()->GetSubRoom() || (*it)->GetSubRoom2() == GetDest()->GetSubRoom())) continue;
        count++;
        distance = distance + (((*it)->GetCentre() - crossing->GetCentre()).Norm());
    }
    if(count == 0) approximate_distance = 0;
    else approximate_distance = distance/count;
}


double GraphEdge::GetWeight(const Point & position) const
{
    if(factors.empty()) {
        return GetApproximateDistance(position);
    }
    double weight = GetApproximateDistance(position);

    for(FactorContainer::const_iterator it = factors.begin(); it != factors.end(); ++it) {
        weight = weight * it->second.first;
    }

    return weight;
}

void GraphEdge::SetFactor(double factor, std::string name)
{
    //TODO: set global time as second double
    factors[name] = std::make_pair(factor, 0.0);
}



/**
 * GETTER AND SETTER
 */

double GraphEdge::GetRoomToFloorFactor() const
{
    if(GetDest() == NULL || GetDest()->GetSubRoom()->GetType() == GetSrc()->GetSubRoom()->GetType()) return 1.0;
    if(GetDest()->GetSubRoom()->GetType() == "floor") return 1.0;
    else return 5.0;
}

double GraphEdge::GetApproximateDistance(const Point & position) const
{
    return (crossing->GetCentre()-position).Norm();
}

double GraphEdge::GetApproximateDistance() const
{
    return approximate_distance;
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

bool GraphEdge::IsExit() const
{
    return crossing->IsExit();
}
