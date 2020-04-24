/**
 * \file        SubRoom.cpp
 * \date        Oct 8, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#include "SubRoom.h"

#include "Hline.h"
#include "Line.h"
#include "Obstacle.h"
#include "Point.h"
#include "Transition.h"
#include "WaitingArea.h"
#include "Wall.h"
#include "general/Logger.h"

#ifdef _SIMULATOR
#include "pedestrian/Pedestrian.h"
#endif //_SIMULATOR

#include <cmath>


int SubRoom::_static_uid = 1; //must be positive (sideeffect for FloorfieldViaFM::isInside())


SubRoom::SubRoom()
{
    _id                  = -1;
    _roomID              = -1;
    _walls               = std::vector<Wall>();
    _poly                = std::vector<Point>();
    _poly_help_constatnt = std::vector<double>();
    _poly_help_multiple  = std::vector<double>();
    _obstacles           = std::vector<Obstacle *>();

    _crossings   = std::vector<Crossing *>();
    _transitions = std::vector<Transition *>();
    _hlines      = std::vector<Hline *>();

    _planeEquation[0]            = 0.0;
    _planeEquation[1]            = 0.0;
    _planeEquation[2]            = 0.0;
    _cosAngleWithHorizontalPlane = 0;
    _tanAngleWithHorizontalPlane = 0;
    _minElevation                = 0;
    _maxElevation                = 0;

    _goalIDs   = std::vector<int>();
    _area      = 0.0;
    _uid       = _static_uid++;
    _boostPoly = polygon_type();
}

SubRoom::SubRoom(const SubRoom & orig)
{
    _id                  = orig._id;
    _roomID              = orig._roomID;
    _walls               = orig._walls;
    _poly                = orig._poly;
    _poly_help_constatnt = orig._poly_help_constatnt;
    _poly_help_multiple  = orig._poly_help_multiple;
    _obstacles           = orig._obstacles;

    _crossings   = orig._crossings;
    _transitions = orig._transitions;
    _hlines      = orig._hlines;

    _planeEquation[0]            = orig._planeEquation[0];
    _planeEquation[1]            = orig._planeEquation[1];
    _planeEquation[2]            = orig._planeEquation[2];
    _cosAngleWithHorizontalPlane = orig._cosAngleWithHorizontalPlane;
    _tanAngleWithHorizontalPlane = orig._tanAngleWithHorizontalPlane;
    _minElevation                = orig._minElevation;
    _maxElevation                = orig._maxElevation;

    _goalIDs   = orig._goalIDs;
    _area      = orig._area;
    _uid       = orig._uid;
    _boostPoly = orig._boostPoly;
}

SubRoom::~SubRoom()
{
    for(unsigned int i = 0; i < _obstacles.size(); i++) {
        delete _obstacles[i];
    }
    _obstacles.clear();
}

void SubRoom::SetSubRoomID(int ID)
{
    _id = ID;
}

void SubRoom::SetRoomID(int ID)
{
    _roomID = ID;
}

int SubRoom::GetSubRoomID() const
{
    return _id;
}

// unique identifier for this subroom
int SubRoom::GetUID() const
{
    return _uid;
}

double SubRoom::GetArea() const
{
    return _area;
}

int SubRoom::GetRoomID() const
{
    return _roomID;
}

bool SubRoom::IsPointOnPolygonBoundaries(const Point & point, const Wall & exclude_wall) const
{
    // Incident with transition
    for(const auto & transition : _transitions) {
        if(transition->IsInLineSegment(point)) {
            return true;
        }
    }
    //Incident with crossing
    for(const auto & crossing : _crossings) {
        if(crossing->IsInLineSegment(point)) {
            return true;
        }
    }
    // Incident with wall in any point
    for(const auto & checkWall : _walls) {
        if(exclude_wall != checkWall && checkWall.IsInLineSegment(point)) {
            return true;
        }
    }
    return false;
}


bool SubRoom::HasWall(const Wall & wall) const
{
    return _walls.end() != std::find(_walls.begin(), _walls.end(), wall);
}

const std::vector<Wall> & SubRoom::GetAllWalls() const
{
    return _walls;
}

const std::vector<Point> & SubRoom::GetPolygon() const
{
    return _poly;
}

const std::vector<Obstacle *> & SubRoom::GetAllObstacles() const
{
    return _obstacles;
}


const std::vector<int> & SubRoom::GetAllGoalIDs() const
{
    return _goalIDs;
}

// return true is walls was erased, otherwise false.
bool SubRoom::RemoveWall(const Wall & w)
{
    auto it = std::find(_walls.begin(), _walls.end(), w);
    if(it != _walls.end()) {
        _walls.erase(it);
        return true;
    }
    return false;
}
bool SubRoom::AddWall(const Wall & w)
{
    //check for duplicated walls
    for(auto && w1 : _walls) {
        if(w == w1) {
            return false;
        }
    }
    //checking for wall chunks.
    if(w.Length() < J_TOLERANZ) {
        return false;
    }
    _walls.push_back(w);
    return true;
}


void SubRoom::AddObstacle(Obstacle * obs)
{
    _obstacles.push_back(obs);
    CheckObstacles();
}


void SubRoom::AddGoalID(int ID)
{
    LOG_ERROR(
        "Adding a Goal directly to a subroom is not allowed. Plz add Crossing/Transition instead!");
    if(std::find(_goalIDs.begin(), _goalIDs.end(), ID) != _goalIDs.end()) {
        LOG_WARNING("Added existing GoalID to Subroom {}", this->GetSubRoomID());
        //if occurs, plz assert, that ID is a UID of any line of the goal and not a number given by the user (ar.graf)
    }
    _goalIDs.push_back(ID);
}

bool SubRoom::AddCrossing(Crossing * line)
{
    _crossings.push_back(line);
    _goalIDs.push_back(line->GetUniqueID());
    return true;
}
// return true is walls was erased, otherwise false.
bool SubRoom::RemoveTransition(Transition * t)
{
    auto it = std::find(_transitions.begin(), _transitions.end(), t);
    if(it != _transitions.end()) {
        _transitions.erase(it);
        RemoveGoalID(t->GetUniqueID());
        return true;
    }
    return false;
}
bool SubRoom::AddTransition(Transition * line)
{
    _transitions.push_back(line);
    _goalIDs.push_back(line->GetUniqueID());
    return true;
}

void SubRoom::AddNeighbor(SubRoom * sub)
{
    if(sub && (IsElementInVector(_neighbors, sub) == false)) {
        _neighbors.push_back(sub);
    }
}

bool SubRoom::AddHline(Hline * line)
{
    for(unsigned int i = 0; i < _hlines.size(); i++) {
        if(line->GetID() == _hlines[i]->GetID()) {
            LOG_INFO("Skipping duplicate hline [{}] with id [{}]", _id, line->GetID());
            return false;
        }
    }

    _hlines.push_back(line);
    _goalIDs.push_back(line->GetUniqueID());
    return true;
}

const std::vector<Crossing *> & SubRoom::GetAllCrossings() const
{
    return _crossings;
}

const std::vector<Transition *> & SubRoom::GetAllTransitions() const
{
    return _transitions;
}

const std::vector<Hline *> & SubRoom::GetAllHlines() const
{
    return _hlines;
}

const std::vector<SubRoom *> & SubRoom::GetNeighbors() const
{
    return _neighbors;
}

const Crossing * SubRoom::GetCrossing(int i) const
{
    return _crossings[i];
}

const Transition * SubRoom::GetTransition(int i) const
{
    return _transitions[i];
}

const Hline * SubRoom::GetHline(int i) const
{
    return _hlines[i];
}

void SubRoom::RemoveGoalID(int ID)
{
    for(unsigned int i = 0; i < _goalIDs.size(); i++) {
        if(_goalIDs[i] == ID) {
            LOG_DEBUG("Removing goal");
            _goalIDs.erase(_goalIDs.begin() + i);
            return;
        }
    }
    LOG_WARNING("There is no goal with that id to remove");
}

bool SubRoom::IsAccessible()
{
    //at least one door is open
    for(auto && tran : _transitions) {
        if(tran->IsOpen() == true)
            return true;
    }
    for(auto && cros : _crossings) {
        if(cros->IsOpen() == true)
            return true;
    }
    return false;
}

void SubRoom::CalculateArea()
{
    double sum = 0;
    int n      = (int) _poly.size();
    for(int i = 0; i < n; i++) {
        sum += (_poly[i]._y + _poly[(i + 1) % n]._y) * (_poly[i]._x - _poly[(i + 1) % n]._x);
    }
    _area = (0.5 * fabs(sum));
}

Point SubRoom::GetCentroid() const
{
    double px = 0, py = 0;
    double signedArea = 0.0;
    double x0         = 0.0; // Current vertex X
    double y0         = 0.0; // Current vertex Y
    double x1         = 0.0; // Next vertex X
    double y1         = 0.0; // Next vertex Y
    double a          = 0.0; // Partial signed area

    if(_poly.size() == 0)
        return Point(0, 0);
    // For all vertices except last
    unsigned int i = 0;
    for(i = 0; i < _poly.size() - 1; ++i) {
        x0 = _poly[i]._x;
        y0 = _poly[i]._y;
        x1 = _poly[i + 1]._x;
        y1 = _poly[i + 1]._y;
        a  = x0 * y1 - x1 * y0;
        signedArea += a;
        px += (x0 + x1) * a;
        py += (y0 + y1) * a;
    }

    // Do last vertex
    x0 = _poly[i]._x;
    y0 = _poly[i]._y;
    x1 = _poly[0]._x;
    y1 = _poly[0]._y;
    a  = x0 * y1 - x1 * y0;
    signedArea += a;
    px += (x0 + x1) * a;
    py += (y0 + y1) * a;

    signedArea *= 0.5;
    px /= (6.0 * signedArea);
    py /= (6.0 * signedArea);

    return Point(px, py);
}

std::vector<Wall> SubRoom::GetVisibleWalls(const Point & position)
{
    std::vector<Wall> visible_walls;
    bool wall_is_vis;
    Point nearest_point;
    for(auto w : _walls) {
        // nearest_point = w.ShortestPoint(position);
        wall_is_vis = IsVisible(w, position);
        if(wall_is_vis) {
            visible_walls.push_back(w);
        }
    }
    return visible_walls;
}

// like ped_is_visible() but here we can exclude checking intersection
// with the same wall. This function should check if <position> can see the <Wall>
bool SubRoom::IsVisible(const Line & wall, const Point & position)
{
    bool wall_is_vis = true;
    // Point nearest_point =  wall.ShortestPoint(position);
    // in cases where nearest_point is endPoint of Wall, the wall becomes visible even if it is not..
    //  try with the center. If it is not visible then the wall is definitly not.
    const Point & nearest_point = wall.GetCentre();

    Line ped_wall = Line(position, nearest_point);
    for(auto & w : _walls) {
        if(w == wall) //ignore wall
            continue;
        if(wall_is_vis && ped_wall.IntersectionWith(w)) {
            wall_is_vis = false;
        }
    }

    for(const auto & obst : _obstacles) {
        for(const auto & w : obst->GetAllWalls()) {
            if(wall_is_vis && ped_wall.IntersectionWith(w)) {
                wall_is_vis = false;
            }
        }
    }
    return wall_is_vis;
}

// p1 and p2 are supposed to be pedestrian's positions. This function does not work properly
// for visibility checks with walls, since the line connecting the pedestrian's position
// with the nearest point on the wall IS intersecting with the wall.
bool SubRoom::IsVisible(const Point & p1, const Point & p2, bool considerHlines)
{
    //check intersection with Walls
    for(const auto & wall : _walls) {
        if(wall.IntersectionWith(p1, p2)) {
            return false;
        }
    }

    //check intersection with obstacles
    for(const auto & obstacle : _obstacles) {
        for(const auto & wall : obstacle->GetAllWalls()) {
            if(wall.IntersectionWith(p1, p2)) {
                return false;
            }
        }
    }

    if(considerHlines) {
        for(const auto & hline : _hlines) {
            if(hline->IsInLineSegment(p1) || hline->IsInLineSegment(p2))
                continue;
            if(hline->IntersectionWith(p1, p2)) {
                return false;
            }
        }
    }
    return true;
}

// this is the case if they share a transition or crossing
bool SubRoom::IsDirectlyConnectedWith(SubRoom * sub) const
{
    return IsElementInVector(_neighbors, sub);
}

void SubRoom::SetPlanEquation(double A, double B, double C)
{
    _planeEquation[0] = A;
    _planeEquation[1] = B;
    _planeEquation[2] = C;
    //compute and cache the cosine of angle with the plane z=h
    _cosAngleWithHorizontalPlane = (1.0 / sqrt(A * A + B * B + 1));
    _tanAngleWithHorizontalPlane = sqrt(A * A + B * B); // n1.n2 = -1
}

const double * SubRoom::GetPlaneEquation() const
{
    return _planeEquation;
}

double SubRoom::GetElevation(const Point & p) const
{
    return _planeEquation[0] * p._x + _planeEquation[1] * p._y + _planeEquation[2];
}

double SubRoom::GetCosAngleWithHorizontal() const
{
    return _cosAngleWithHorizontalPlane;
}

double SubRoom::GetTanAngleWithHorizontal() const
{
    return _tanAngleWithHorizontalPlane;
}

double SubRoom::GetMinElevation() const
{
    return _minElevation;
}

double SubRoom::GetMaxElevation() const
{
    return _maxElevation;
}

void SubRoom::SetMinElevation(double minElevation)
{
    _minElevation = minElevation;
}

void SubRoom::SetMaxElevation(double maxElevation)
{
    _maxElevation = maxElevation;
}


bool SubRoom::CheckObstacles()
{
    for(const auto & wall : _walls) {
        for(const auto & obst : _obstacles) {
            if(obst->IntersectWithLine(wall)) {
                LOG_ERROR(
                    "The obstacle [{}] intersects with subroom [{}] in room [{}]. The "
                    "triangulation will not work.",
                    obst->GetId(),
                    _id,
                    _roomID);
                return false;
            }
        }
    }

    return true;
}

bool SubRoom::Overlapp(const std::vector<Line *> & goals) const
{
    for(const auto & wall : _walls) {
        for(const auto & goal : goals) {
            if(wall.Overlapp(*goal)) {
                LOG_INFO(
                    "Wall: ({}, {}) -- ({}, {})",
                    wall.GetPoint1()._x,
                    wall.GetPoint1()._y,
                    wall.GetPoint2()._x,
                    wall.GetPoint2()._y);

                LOG_INFO(
                    "Goal ({}, {}) - ({}, {})",
                    goal->GetPoint1()._x,
                    goal->GetPoint1()._y,
                    goal->GetPoint2()._x,
                    goal->GetPoint2()._y);

                return true;
            }
        }
    }

    return false;
}

bool SubRoom::SanityCheck()
{
    if(_obstacles.size() == 0) {
        if((IsConvex() == false) && (_hlines.size() == 0)) {
            LOG_WARNING(
                "Room [{}] Subroom [{}] is not convex. You might consider adding extra "
                "hlines in your routing.xml file",
                _roomID,
                _id);
        } else {
            // everything is fine
        }
    } else {
        if(_hlines.size() == 0) {
            LOG_WARNING(
                "You have obstacles in room [{}] Subroom [{}], You might consider adding "
                "extra hlines in your routing.xml file",
                _roomID,
                _id);
        } else {
            // everything is fine
        }
    }
    //check if there are overlapping walls
    for(auto && w1 : _walls) {
        bool connected = false;

        for(auto && w2 : _walls) {
            if(w1 == w2)
                continue;
            if(w1.Overlapp(w2)) {
                LOG_ERROR("Overlapping between walls {} and {}", w1.toString(), w2.toString());
                exit(EXIT_FAILURE);
            }
            connected = connected || w1.ShareCommonPointWith(w2);
        }
        //overlapping with lines
        for(auto && hline : _hlines) {
            if(w1.Overlapp(*hline)) {
                LOG_ERROR(
                    "Overlapping between wall {} and  Hline {}", w1.toString(), hline->toString());
            }
        }
        //overlaping with crossings
        for(auto && c : _crossings) {
            if(w1.Overlapp(*c)) {
                LOG_ERROR(
                    "Overlapping between wall {} and  crossing {}", w1.toString(), c->toString());
                exit(EXIT_FAILURE);
            }
            connected = connected || w1.ShareCommonPointWith(*c);
        }
        //overlaping with transitions
        for(auto && t : _transitions) {
            if(w1.Overlapp(*t)) {
                LOG_ERROR(
                    "Overlapping between wall {} and  transition {}", w1.toString(), t->toString());
                exit(EXIT_FAILURE);
            }
            connected = connected || w1.ShareCommonPointWith(*t);
        }

        if(!connected) {
            LOG_ERROR("Loose wall found {} in Room/Subroom {}/{}", w1.toString(), _roomID, _id);
            exit(EXIT_FAILURE);
            //return false;
        }
    }

    //check overlaping between hline and crossings
    for(auto && h : _hlines) {
        for(auto && c : _crossings) {
            if(h->Overlapp(*c)) {
                LOG_ERROR("Overlapping hline {} and  crossing {}", h->toString(), c->toString());
                exit(EXIT_FAILURE);
            }
        }
    }
    //check overlaping between hlines and transitions
    for(auto && h : _hlines) {
        for(auto && t : _transitions) {
            if(h->Overlapp(*t)) {
                LOG_ERROR("Overlapping hline {} and  transition {}", h->toString(), t->toString());
                exit(EXIT_FAILURE);
            }
        }
    }
    //check overlaping between transitions and crossings
    for(auto && c : _crossings) {
        for(auto && t : _transitions) {
            if(c->Overlapp(*t)) {
                LOG_ERROR(
                    "Overlapping crossing {} and  transition {}", c->toString(), t->toString());
                exit(EXIT_FAILURE);
                return false;
            }
        }
    }

    return true;
}

bool SubRoom::Triangulate()
{
    if(IsClockwise())
        std::reverse(_poly.begin(), _poly.end());

    _delauneyTriangulator.SetOuterPolygone(_poly);

    for(const auto & obst : _obstacles) {
        auto outerhullObst = obst->GetPolygon();
        if(obst->IsClockwise())
            std::reverse(outerhullObst.begin(), outerhullObst.end());
        _delauneyTriangulator.AddHole(outerhullObst);
    }

    _delauneyTriangulator.Triangulate();
    return true;
}

const std::vector<p2t::Triangle *> SubRoom::GetTriangles()
{
    return _delauneyTriangulator.GetTriangles();
}

///http://stackoverflow.com/questions/471962/how-do-determine-if-a-polygon-is-complex-convex-nonconvex
bool SubRoom::IsConvex()
{
    unsigned int hsize = (unsigned int) _poly.size();
    unsigned int pos   = 0;
    unsigned int neg   = 0;

    if(hsize == 0) {
        LOG_WARNING("Cannot check empty polygon for convexification");
        LOG_WARNING("Did you forget to call ConvertLineToPoly() ?");
        return false;
    }

    for(unsigned int i = 0; i < hsize; i++) {
        Point vecAB = _poly[(i + 1) % hsize] - _poly[i % hsize];
        Point vecBC = _poly[(i + 2) % hsize] - _poly[(i + 1) % hsize];
        double det  = vecAB.Determinant(vecBC);
        if(fabs(det) < J_EPS)
            det = 0.0;

        if(det < 0.0) {
            neg++;
        } else if(det > 0.0) {
            pos++;
        } else {
            pos++;
            neg++;
        }
    }

    if((pos == hsize) || (neg == hsize)) {
        return true;
    }
    return false;
}

///http://stackoverflow.com/questions/9473570/polygon-vertices-clockwise-or-counterclockwise/
bool SubRoom::IsClockwise()
{
    //http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    if(_poly.size() < 3) {
        LOG_ERROR("You need at least 3 vertices to check for orientation. Obstacle ID [{}]", _id);
        return false;
    }
    double sum = 0;
    for(unsigned int i = 0; i < _poly.size() - 1; ++i) {
        Point a = _poly[i];
        Point b = _poly[i + 1];
        sum += (b._x - a._x) * (b._y + a._y);
    }
    Point first = _poly[0];
    Point last  = _poly[_poly.size() - 1];
    sum += (first._x - last._x) * (first._y + last._y);

    return (sum > 0.);
}

bool SubRoom::IsPartOfPolygon(const Point & ptw)
{
    if(false == IsElementInVector(_poly, ptw)) {
        //maybe the point was too closed to other points and got replaced
        //check that eventuality
        bool nah = false;
        for(const auto & pt : _poly) {
            if((pt - ptw).Norm() < J_TOLERANZ) {
                nah = true;
                break;
            }
        }

        if(nah == false) {
            return false;
        }
    }
    return true;
}

bool SubRoom::IsInObstacle(const Point & pt)
{
    for(polygon_type obs : _boostPolyObstacles) {
        if(boost::geometry::within(pt, obs)) {
            return true;
        }
    }
    return false;
}

bool SubRoom::CreateBoostPoly()
{
    std::vector<Point> copyPts;
    copyPts.insert(copyPts.begin(), _poly.begin(), _poly.end());
    if(!IsClockwise())
        std::reverse(copyPts.begin(), copyPts.end());

    boost::geometry::assign_points(_boostPoly, _poly);

    for(auto obsPtr : _obstacles) {
        std::vector<Point> obsPoints;
        obsPoints.insert(
            obsPoints.begin(), obsPtr->GetPolygon().begin(), obsPtr->GetPolygon().end());
        if(obsPtr->IsClockwise()) {
            std::reverse(obsPoints.begin(), obsPoints.end());
        }
        polygon_type newObstacle;
        boost::geometry::assign_points(newObstacle, obsPoints);
        _boostPolyObstacles.emplace_back(newObstacle);
    }
    return true;
}

NormalSubRoom::NormalSubRoom() : SubRoom() {}

NormalSubRoom::NormalSubRoom(const NormalSubRoom & orig) : SubRoom(orig) {}

NormalSubRoom::~NormalSubRoom() {}

std::string NormalSubRoom::WriteSubRoom() const
{
    std::string s;
    for(auto && w : _walls) {
        std::string geometry;
        char wall[1024] = "";
        geometry.append("\t\t<wall>\n");
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint1()._x),
            (w.GetPoint1()._y),
            GetElevation(w.GetPoint1()));
        geometry.append(wall);
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint2()._x),
            (w.GetPoint2()._y),
            GetElevation(w.GetPoint2()));
        geometry.append(wall);
        geometry.append("\t\t</wall>\n");

        s.append(geometry);
    }

    //add the subroom caption
    const Point & pos = GetCentroid();
    char tmp[1024];
    sprintf(
        tmp,
        "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" text=\"%d\" color=\"100\" "
        "/>\n",
        pos._x,
        pos._y,
        GetElevation(pos),
        GetSubRoomID());
    s.append(tmp);

    //write the obstacles
    for(auto && obst : GetAllObstacles()) {
        for(auto && w : obst->GetAllWalls()) {
            char wall[1024] = "";
            s.append("\t\t<wall>\n");
            sprintf(
                wall,
                "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                (w.GetPoint1()._x),
                (w.GetPoint1()._y),
                GetElevation(w.GetPoint1()));
            s.append(wall);
            sprintf(
                wall,
                "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                (w.GetPoint2()._x),
                (w.GetPoint2()._y),
                GetElevation(w.GetPoint2()));
            s.append(wall);
            s.append("\t\t</wall>\n");
        }

        const Point & obst_pos = obst->GetCentroid();

        //add the obstacle caption
        char tmp1[1024];
        sprintf(
            tmp1,
            "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" text=\"%d\" "
            "color=\"100\" />\n",
            obst_pos._x,
            obst_pos._y,
            GetElevation(obst_pos),
            obst->GetId());
        s.append(tmp1);
    }

    return s;
}

std::string NormalSubRoom::WritePolyLine() const
{
    char tmp[1024];
    std::string s;
    s.append("\t<Obstacle closed=\"1\" boundingbox=\"0\" class=\"1\">\n");
    for(const auto & p : _poly) {
        sprintf(tmp, "\t\t<Vertex p_x = \"%.2lf\" p_y = \"%.2lf\"/>\n", p._x, p._y);
        s.append(tmp);
    }
    s.append("\t</Obstacle>\n");

    //write the obstacles
    for(auto && obst : _obstacles) {
        s.append(obst->Write());
    }
    return s;
}


bool NormalSubRoom::ConvertLineToPoly(const std::vector<Line *> & goals)
{
    std::vector<Line *> copy;
    std::vector<Point> tmpPoly;
    Point point;
    Line * line;
    // Alle Linienelemente in copy speichern
    for(auto & w : _walls) {
        copy.push_back(&w);
    }

    // Transitions und Crossings sind in goal abgespeichert
    copy.insert(copy.end(), goals.begin(), goals.end());

    if(Overlapp(goals)) {
        LOG_ERROR("Overlapping between walls and goals");
        return false;
    }
    Point pIntsct(J_NAN, J_NAN);
    int itr = 1;
    for(auto & it : _walls) {
        int j = 0;
        for(unsigned int i = itr; i < copy.size(); ++i) {
            if(it.IntersectionWith(*copy[i], pIntsct) == true) {
                if(it.ShareCommonPointWith(*copy[i]) == false) {
                    LOG_ERROR(
                        "NormalSubRoom::ConvertLineToPoly(): SubRoom {} Room {}, walls "
                        "%s & %s intersect.",
                        GetSubRoomID(),
                        GetRoomID(),
                        it.toString(),
                        copy[i]->toString());

                    return false;
                } else
                    ++j; //number of lines, that share endpoints with wall "it"
            }
        }
        if(j <= 2)
            j = 0; //all good, set j back to 0 for next iteration
        else {
            LOG_ERROR(
                "NormalSubRoom::ConvertLineToPoly(): SubRoom {} Room {}, wall {} shares "
                "edge with multiple walls j={}.",
                GetSubRoomID(),
                GetRoomID(),
                it.toString(),
                j);
        }
        ++itr; // only check lines that have greater index than current "it" (inner loop goes from itr to size)
    }


    line = copy[0];
    tmpPoly.push_back(line->GetPoint1());
    point = line->GetPoint2();
    copy.erase(copy.begin());
    // Polygon aus allen Linen erzeugen
    for(int i = 0; i < (int) copy.size(); i++) {
        line = copy[i];
        if((point - line->GetPoint1()).Norm() < J_TOLERANZ) {
            tmpPoly.push_back(line->GetPoint1());
            point = line->GetPoint2();
            copy.erase(copy.begin() + i);
            // von vorne suchen
            i = -1;
        } else if((point - line->GetPoint2()).Norm() < J_TOLERANZ) {
            tmpPoly.push_back(line->GetPoint2());
            point = line->GetPoint1();
            copy.erase(copy.begin() + i);
            // von vorne suchen
            i = -1;
        }
    }
    if((tmpPoly[0] - point).Norm() > J_TOLERANZ) {
        LOG_ERROR(
            "NormalSubRoom::ConvertLineToPoly(): SubRoom {} Room {}, polygon not closed "
            "({}, {}) != ({}, {}), distance = {}",
            GetSubRoomID(),
            GetRoomID(),
            tmpPoly[0]._x,
            tmpPoly[0]._y,
            point._x,
            point._y,
            (tmpPoly[0] - point).Norm());

        return false;
    }
    _poly = StartLLCorner(tmpPoly);


    //check if all walls and goals were used in the polygon
    for(const auto & w : _walls) {
        for(const auto & ptw : {w.GetPoint1(), w.GetPoint2()}) {
            if(IsPartOfPolygon(ptw) == false) {
                LOG_ERROR(
                    "Wall {} was not used during polygon creation for room/subroom: {}/{}",
                    w.toString(),
                    GetRoomID(),
                    GetSubRoomID());
                return false;
            }
        }
    }

    for(const auto & g : goals) {
        for(const auto & ptw : {g->GetPoint1(), g->GetPoint2()}) {
            if(IsPartOfPolygon(ptw) == false) {
                LOG_ERROR(
                    "Exit/crossing/transition {} was not used during polygon creation "
                    "for room/subroom: {}/{}",
                    g->toString(),
                    GetRoomID(),
                    GetSubRoomID());
                return false;
            }
        }
    }
    return true;
}


// private Funktionen

// gibt zurück in welchen Quadranten vertex liegt, wobei hitPos der Koordinatenursprung ist

short NormalSubRoom::WhichQuad(const Point & vertex, const Point & hitPos) const
{
    return (vertex._x > hitPos._x) ? ((vertex._y > hitPos._y) ? 1 : 4) :
                                     ((vertex._y > hitPos._y) ? 2 : 3);
}

// x-Koordinate der Linie von einer Eccke zur nächsten

double NormalSubRoom::Xintercept(const Point & point1, const Point & point2, double hitY) const
{
    return (point2._x - (((point2._y - hitY) * (point1._x - point2._x)) / (point1._y - point2._y)));
}


// This method is called very often in DirectionFloorField, so it should be fast.
// we ignore
//@todo: ar.graf: UnivFF have subroomPtr Info for every gridpoint. Info should be used in DirectionFF instead of this
bool NormalSubRoom::IsInSubRoom(const Point & p) const
{
    // if pedestrian is stuck in obstacle or on obstacle line, return false
    for(polygon_type obs : _boostPolyObstacles) {
        if(boost::geometry::within(p, obs)) {
            return false;
        }
    }
    for(auto & obs : _obstacles) {
        for(auto & wall : obs->GetAllWalls()) {
            if(wall.IsInLineSegment(p)) {
                return false;
            }
        }
    }

    // check if point is on one of the doors
    for(auto & trans : GetAllTransitions()) {
        if(trans->IsInLineSegment(p)) {
            return true;
        }
    }
    for(auto & cross : GetAllCrossings()) {
        if(cross->IsInLineSegment(p)) {
            return true;
        }
    }

    // point is not in obstacle and on no door, so we can use within(...) on _boostPoly
    return boost::geometry::within(p, _boostPoly);
}

Stair::Stair() : NormalSubRoom()
{
    pUp   = Point();
    pDown = Point();
}

Stair::Stair(const Stair & orig) : NormalSubRoom(orig), pUp(orig.pUp), pDown(orig.pDown) {}


Stair::~Stair() {}

// Setter-Funktionen

void Stair::SetUp(const Point & p)
{
    pUp = p;
}

void Stair::SetDown(const Point & p)
{
    pDown = p;
}

// Getter-Funktionen

const Point & Stair::GetUp() const
{
    return pUp;
}

const Point & Stair::GetDown() const
{
    return pDown;
}

std::string Stair::WriteSubRoom() const
{
    std::string s;
    for(auto && w : _walls) {
        std::string geometry;
        char wall[1024] = "";
        geometry.append("\t\t<wall>\n");
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint1()._x),
            (w.GetPoint1()._y),
            GetElevation(w.GetPoint1()));
        geometry.append(wall);
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint2()._x),
            (w.GetPoint2()._y),
            GetElevation(w.GetPoint2()));
        geometry.append(wall);
        geometry.append("\t\t</wall>\n");

        s.append(geometry);
    }
    Point pos = GetCentroid();
    char tmp_c[1024];
    sprintf(
        tmp_c,
        "\t\t<sphere centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" radius=\"%.2f\" "
        "color=\"100\" />\n",
        GetUp()._x,
        GetUp()._y,
        GetElevation(GetUp()),
        0.2);
    s.append(tmp_c);

    //add the subroom caption
    sprintf(
        tmp_c,
        "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" text=\"%d\" color=\"100\" "
        "/>\n",
        pos._x,
        pos._y,
        GetElevation(pos),
        GetSubRoomID());
    s.append(tmp_c);

    return s;
}

std::string Stair::WritePolyLine() const
{
    std::string s;
    char tmp[1024];

    s.append("\t<Obstacle closed=\"1\" boundingbox=\"0\" class=\"1\">\n");
    for(unsigned int j = 0; j < _poly.size(); j++) {
        sprintf(tmp, "\t\t<Vertex p_x = \"%.2lf\" p_y = \"%.2lf\"/>\n", _poly[j]._x, _poly[j]._y);
        s.append(tmp);
    }
    s.append("\t</Obstacle>\n");

    //write the obstacles
    for(unsigned int j = 0; j < GetAllObstacles().size(); j++) {
        s.append(GetAllObstacles()[j]->Write());
    }

    return s;
}

/* prüft ob die Punkte p1, p2 und p3 auf einer Linie liegen, oder eine Ecke bilden.
 * Dabei liegt p2 IMMER in der Mitte und entspricht aktPoint
 * */
const Point *
Stair::CheckCorner(const Point ** otherPoint, const Point ** aktPoint, const Point * nextPoint)
{
    Point l1            = **otherPoint - **aktPoint;
    Point l2            = *nextPoint - **aktPoint;
    const Point * rueck = nullptr;
    // Punkte bilden eine Linie
    if(fabs(fabs(l1.ScalarProduct(l2) / (l1.Norm() * l2.Norm())) - 1) < 0.1) {
        *aktPoint = nextPoint;
    } else { // aktPoint/p2 ist eine Ecke
        rueck       = *aktPoint;
        *otherPoint = *aktPoint;
        *aktPoint   = nextPoint;
    }
    return rueck;
}

bool Stair::ConvertLineToPoly(const std::vector<Line *> & goals)
{
    std::vector<Line *> copy;
    std::vector<Point> orgPoly = std::vector<Point>();
    const Point * aktPoint;
    const Point * otherPoint;
    const Point * nextPoint;
    const Point * firstAktPoint;
    const Point * firstOtherPoint;
    Line * nextLine;

    // Alle Linienelemente in copy speichern
    for(auto & w : _walls) {
        copy.push_back(&w);
    }

    // Transitions und Crossings sind in goal abgespeichert
    copy.insert(copy.end(), goals.begin(), goals.end());

    if(Overlapp(goals)) {
        LOG_ERROR("Overlapping between walls and goals");
        return false;
    }

    aktPoint        = &copy[0]->GetPoint1();
    firstAktPoint   = aktPoint;
    otherPoint      = &copy[0]->GetPoint2();
    firstOtherPoint = otherPoint;
    copy.erase(copy.begin());

    // Polygon aus allen Linen erzeugen
    for(int i = 0; i < (int) copy.size(); i++) {
        nextLine  = copy[i];
        nextPoint = nullptr;
        if((*aktPoint - nextLine->GetPoint1()).Norm() < J_TOLERANZ) {
            nextPoint = &nextLine->GetPoint2();
        } else if((*aktPoint - nextLine->GetPoint2()).Norm() < J_TOLERANZ) {
            nextPoint = &nextLine->GetPoint1();
        }
        if(nextPoint != nullptr) {
            const Point * rueck = CheckCorner(&otherPoint, &aktPoint, nextPoint);
            if(rueck != nullptr)
                orgPoly.push_back(*rueck);
            copy.erase(copy.begin() + i);
            i = -1; // von vorne suchen
        }
    }
    if((*aktPoint - *firstOtherPoint).Norm() < J_TOLERANZ) {
        const Point * rueck = CheckCorner(&otherPoint, &aktPoint, firstAktPoint);
        if(rueck != nullptr)
            orgPoly.push_back(*rueck);
    } else {
        LOG_ERROR(
            "Stair::ConvertLineToPoly(): SubRoom {} Room {}, polygon not closed ({}, {}) "
            "!= ({}, {})",
            GetSubRoomID(),
            GetRoomID(),
            firstOtherPoint->_x,
            firstOtherPoint->_y,
            aktPoint->_x,
            aktPoint->_y);

        return false;
    }

    if(orgPoly.size() != 4) {
        LOG_ERROR(
            "Stair::ConvertLineToPoly(): Stair {} Room {} is not a quadrangle,  num "
            "angles: {}",
            GetSubRoomID(),
            GetRoomID(),
            orgPoly.size());

        return false;
    }
    std::vector<Point> neuPoly = (orgPoly);
    // ganz kleine Treppen (nur eine Stufe) nicht
    if((neuPoly[0] - neuPoly[1]).Norm() > 0.9 && (neuPoly[1] - neuPoly[2]).Norm() > 0.9) {
        for(int i1 = 0; i1 < (int) orgPoly.size(); i1++) {
            unsigned long i2 = (i1 + 1) % orgPoly.size();
            unsigned long i3 = (i2 + 1) % orgPoly.size();
            unsigned long i4 = (i3 + 1) % orgPoly.size();
            Point p1         = neuPoly[i1];
            Point p2         = neuPoly[i2];
            Point p3         = neuPoly[i3];
            Point p4         = neuPoly[i4];
            Point l1         = p2 - p1;
            Point l2         = p3 - p2;

            if(l1.Norm() < l2.Norm()) {
                neuPoly[i2] = neuPoly[i2] + l1.Normalized() * 2 * J_EPS_GOAL;
                l2          = p3 - p4;
                neuPoly[i3] = neuPoly[i3] + l2.Normalized() * 2 * J_EPS_GOAL;
            }
        }
    }
    _poly = neuPoly;
    return true;
}


void SubRoom::SetType(const std::string & type)
{
    _type = type;
}

const std::string & SubRoom::GetType() const
{
    return _type;
}

#ifdef _SIMULATOR

bool SubRoom::IsInSubRoom(Pedestrian * ped) const
{
    const Point & pos = ped->GetPos();
    if((ped->GetExitLine()) && (ped->GetExitLine()->DistTo(pos) <= J_EPS_GOAL))
        return IsInSubRoom(pos);
    else
        return IsInSubRoom(pos);
}


std::vector<Point> SubRoom::StartLLCorner(const std::vector<Point> & polygon)
{
    // detecting point which is in the lower left corner
    Point startingpoint = polygon[0];
    size_t id_start     = 0;

    for(size_t i = 1; i < polygon.size(); ++i) {
        if(polygon[i]._x <= startingpoint._x) {
            if(polygon[i]._y <= startingpoint._y) {
                startingpoint = polygon[i];
                id_start      = i;
            }
        }
    }

    std::vector<Point> cwPolygon;
    for(size_t i = id_start; i < polygon.size(); ++i) {
        cwPolygon.push_back(polygon[i]);
    }
    for(size_t i = 0; i < id_start; ++i) {
        cwPolygon.push_back(polygon[i]);
    }

    return cwPolygon;
}

std::vector<WaitingArea *> SubRoom::GetAllWaitingAreas()
{
    return std::vector<WaitingArea *>();
}

bool SubRoom::HasGoal(int id)
{
    return std::find(_goalIDs.begin(), _goalIDs.end(), id) != _goalIDs.end();
}

void SubRoom::Update()
{
    std::vector<Line *> goals;
    goals.insert(goals.end(), _crossings.begin(), _crossings.end());
    goals.insert(goals.end(), _transitions.begin(), _transitions.end());
    if(!ConvertLineToPoly(goals)) {
        std::string message =
            fmt::format(FMT_STRING("SubRoom {}, {} could not converted to polygon"), _roomID, _id);
        throw std::runtime_error(message);
    }

    std::for_each(std::begin(_obstacles), std::end(_obstacles), [this](Obstacle * obstacle) {
        if(!obstacle->ConvertLineToPoly()) {
            std::string message = fmt::format(
                FMT_STRING("obstacle {}  in SubRoom {}, {} could not converted to polygon"),
                _roomID,
                _id,
                obstacle->GetId());
            throw std::runtime_error(message);
        }
    });

    CalculateArea();
    CreateBoostPoly();
}


/// Escalator

Escalator::Escalator() : Stair() {}

Escalator::Escalator(const Escalator & orig) : Stair(orig), isEscalator_Up(orig.isEscalator_Up) {}

Escalator::~Escalator() {}

// Setter-Funktionen
void Escalator::SetEscalatorUp()
{
    isEscalator_Up = true;
}
void Escalator::SetEscalatorDown()
{
    isEscalator_Up = false;
}

// Getter-Funktionen
bool Escalator::IsEscalatorUp() const
{
    return isEscalator_Up;
}
bool Escalator::IsEscalatorDown() const
{
    return !isEscalator_Up;
}
#endif // _SIMULATOR
