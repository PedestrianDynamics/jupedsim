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
#include "Wall.h"


#ifdef _SIMULATOR
#include "../pedestrian/Pedestrian.h"
#endif //_SIMULATOR

#include <cmath>

using namespace std;

/************************************************************
 SubRoom
 ************************************************************/

int SubRoom::_static_uid = 0;

SubRoom::SubRoom()
{
    _id        = -1;
    _roomID    = -1;
    _walls     = vector<Wall>();
    _poly      = vector<Point>();
    _obstacles = vector<Obstacle *>();

    _crossings   = vector<Crossing *>();
    _transitions = vector<Transition *>();
    _hlines      = vector<Hline *>();

    _planeEquation[0]            = 0.0;
    _planeEquation[1]            = 0.0;
    _planeEquation[2]            = 0.0;
    _cosAngleWithHorizontalPlane = 0;

    _goalIDs = vector<int>();
    _area    = 0.0;
    _uid     = _static_uid++;
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
    //return pRoomID * 1000 + pID;
}

double SubRoom::GetArea() const
{
    return _area;
}

int SubRoom::GetRoomID() const
{
    return _roomID;
}

const vector<Wall> & SubRoom::GetAllWalls() const
{
    return _walls;
}

const vector<Point> & SubRoom::GetPolygon() const
{
    return _poly;
}

const vector<Obstacle *> & SubRoom::GetAllObstacles() const
{
    return _obstacles;
}

int SubRoom::GetNumberOfGoalIDs() const
{
    return (int) _goalIDs.size();
}

const vector<int> & SubRoom::GetAllGoalIDs() const
{
    return _goalIDs;
}


bool SubRoom::AddWall(const Wall & w)
{
    //check for duplicated walls
    for(auto && w1 : _walls) {
        if(w == w1) {
            Log->Write(
                "ERROR:\t Duplicate wall found in Room/Subroom %d/%d  %s",
                _roomID,
                _id,
                w.toString().c_str());
            Log->Write("ERROR:\t will not be added");
            //exit(EXIT_FAILURE);
            return false;
        }
    }
    //checking for wall chunks.
    if(w.Length() < J_TOLERANZ) {
        Log->Write(
            "ERROR:\t Wall too small (length = %lf) found in Room/Subroom %d/%d  %s",
            w.Length(),
            _roomID,
            _id,
            w.toString().c_str());
        Log->Write("ERROR:\t will not be added");
        //exit(EXIT_FAILURE);
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
    _goalIDs.push_back(ID);
}

void SubRoom::AddCrossing(Crossing * line)
{
    _crossings.push_back(line);
    _goalIDs.push_back(line->GetUniqueID());
}

void SubRoom::AddTransition(Transition * line)
{
    _transitions.push_back(line);
    _goalIDs.push_back(line->GetUniqueID());
}

void SubRoom::AddNeighbor(SubRoom * sub)
{
    if(sub && (IsElementInVector(_neighbors, sub) == false)) {
        _neighbors.push_back(sub);
    }
}

void SubRoom::AddHline(Hline * line)
{
    for(unsigned int i = 0; i < _hlines.size(); i++) {
        if(line->GetID() == _hlines[i]->GetID()) {
            Log->Write("INFO:\tskipping duplicate hline [%d] with id [%d]", _id, line->GetID());
            return;
        }
    }

    _hlines.push_back(line);
    _goalIDs.push_back(line->GetUniqueID());
}

const vector<Crossing *> & SubRoom::GetAllCrossings() const
{
    return _crossings;
}

const vector<Transition *> & SubRoom::GetAllTransitions() const
{
    return _transitions;
}

const vector<Hline *> & SubRoom::GetAllHlines() const
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
            Log->Write("Removing goal");
            _goalIDs.erase(_goalIDs.begin() + i);
            return;
        }
    }
    Log->Write("There is no goal with that id to remove");
}


void SubRoom::CalculateArea()
{
    double sum = 0;
    int n      = (int) _poly.size();
    for(int i = 0; i < n; i++) {
        sum += (_poly[i].GetY() + _poly[(i + 1) % n].GetY()) *
               (_poly[i].GetX() - _poly[(i + 1) % n].GetX());
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
        x0 = _poly[i].GetX();
        y0 = _poly[i].GetY();
        x1 = _poly[i + 1].GetX();
        y1 = _poly[i + 1].GetY();
        a  = x0 * y1 - x1 * y0;
        signedArea += a;
        px += (x0 + x1) * a;
        py += (y0 + y1) * a;
    }

    // Do last vertex
    x0 = _poly[i].GetX();
    y0 = _poly[i].GetY();
    x1 = _poly[0].GetX();
    y1 = _poly[0].GetY();
    a  = x0 * y1 - x1 * y0;
    signedArea += a;
    px += (x0 + x1) * a;
    py += (y0 + y1) * a;

    signedArea *= 0.5;
    px /= (6.0 * signedArea);
    py /= (6.0 * signedArea);

    return Point(px, py);
}

vector<Wall> SubRoom::GetVisibleWalls(const Point & position)
{
#define DEBUG 0
    std::vector<Wall> visible_walls;
    bool wall_is_vis;
    Point nearest_point;
#if DEBUG
    printf("\n---------------------------\nEnter GetVisiblewalls\n");
#endif
    for(auto w : _walls) {
        // nearest_point = w.ShortestPoint(position);
        wall_is_vis = IsVisible(w, position);
        if(wall_is_vis) {
#if DEBUG
            printf(
                "  GetVisibleWalls: Wall (%f, %f)--(%f, %f)\n",
                w.GetPoint1().GetX(),
                w.GetPoint1().GetY(),
                w.GetPoint2().GetX(),
                w.GetPoint2().GetY());
            printf("  GetVisibleWalls: Ped position (%f, %f)\n", position.GetX(), position.GetY());
            printf("  GetVisibleWalls: wall is visible? = %d\n", wall_is_vis);
#endif
            visible_walls.push_back(w);
        }
    }
#if DEBUG
    printf(
        "Leave GetVisiblewalls with %d visible walls\n------------------------------\n",
        visible_walls.size());
#endif
    return visible_walls;
}

// like ped_is_visible() but here we can exclude checking intersection
// with the same wall. This function should check if <position> can see the <Wall>
bool SubRoom::IsVisible(const Line & wall, const Point & position)
{
    // printf("\tEnter wall_is_visible\n");
    // printf(" \t  Wall (%f, %f)--(%f, %f)\n",wall.GetPoint1().GetX(), wall.GetPoint1().GetY(),wall.GetPoint2().GetX(), wall.GetPoint2().GetY() );

    bool wall_is_vis = true;
    // Point nearest_point =  wall.ShortestPoint(position);
    // in cases where nearest_point is endPoint of Wall, the wall becomes visible even if it is not..
    //  try with the center. If it is not visible then the wall is definitly not.
    const Point & nearest_point = wall.GetCentre();

    // printf("\t\t center of wall %f, %f\n",nearest_point.GetX(), nearest_point.GetY());
    Line ped_wall = Line(position, nearest_point);
    for(auto & w : _walls) {
        if(w == wall) //ignore wall
            continue;
        if(wall_is_vis && ped_wall.IntersectionWith(w)) {
            // fprintf (stdout, "\t\t Wall_is_visible: INTERSECTION WALL  L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), WALL(%.2f, %.2f)---(%.2f, %.2f)\n", ped_wall.GetPoint1().GetX(),ped_wall.GetPoint1().GetY(), ped_wall.GetPoint2().GetX(), ped_wall.GetPoint2().GetY(), w.GetPoint1().GetX(),w.GetPoint1().GetY(),w.GetPoint2().GetX(),w.GetPoint2().GetY());
            wall_is_vis = false;
        }
    }

    for(const auto & obst : _obstacles) {
        for(const auto & w : obst->GetAllWalls()) {
            if(wall_is_vis && ped_wall.IntersectionWith(w)) {
                // fprintf (stdout, "\t\t Wall_is_visible INTERSECTION OBS; L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", w.GetPoint1().GetX(), w.GetPoint1().GetY(), w.GetPoint2().GetX(), w.GetPoint2().GetY(), ped_wall.GetPoint1().GetX(), ped_wall.GetPoint1().GetY(), ped_wall.GetPoint2().GetX(), ped_wall.GetPoint2().GetY());
                wall_is_vis = false;
            }
        }
    }

    // printf("\tLeave wall_is_visible with %d\n", wall_is_vis);
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

    // printf("\t\t -- ped_is_visible; check obstacles\n");
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

    // printf("\t\tLeave ped_is_visible with %d\n",temp);
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

bool SubRoom::CheckObstacles()
{
    for(const auto & wall : _walls) {
        for(const auto & obst : _obstacles) {
            if(obst->IntersectWithLine(wall)) {
                Log->Write(
                    "ERROR: \tthe obstacle id [%d] is intersection with subroom [%d]",
                    obst->GetId(),
                    _id);
                Log->Write("     : \tthe triangulation will not work.");
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
            if(wall.Overlapp(*goal))
                return true;
        }
    }

    return false;
}

bool SubRoom::SanityCheck()
{
    if(_obstacles.size() == 0) {
        if((IsConvex() == false) && (_hlines.size() == 0)) {
            Log->Write("WARNING:\t Room [%d] Subroom [%d] is not convex!", _roomID, _id);
            Log->Write("\t\t you might consider adding extra hlines in your routing.xml file");
        } else {
            // everything is fine
        }
    } else {
        if(_hlines.size() == 0) {
            Log->Write("WARNING:\t you have obstacles in room [%d] Subroom [%d]!", _roomID, _id);
            Log->Write("\t\t you might consider adding extra hlines in your routing.xml file");
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
                Log->Write(
                    "ERROR: Overlapping between walls %s and %s ",
                    w1.toString().c_str(),
                    w2.toString().c_str());
                exit(EXIT_FAILURE);
                //return false;
            }
            connected = connected || w1.ShareCommonPointWith(w2);
        }
        //overlapping with lines
        for(auto && hline : _hlines) {
            if(w1.Overlapp(*hline)) {
                Log->Write(
                    "ERROR: Overlapping between wall %s and  Hline %s ",
                    w1.toString().c_str(),
                    hline->toString().c_str());
                exit(EXIT_FAILURE);
                //return false;
            }
        }
        //overlaping with crossings
        for(auto && c : _crossings) {
            if(w1.Overlapp(*c)) {
                Log->Write(
                    "ERROR: Overlapping between wall %s and  crossing %s ",
                    w1.toString().c_str(),
                    c->toString().c_str());
                exit(EXIT_FAILURE);
                //return false;
            }
            connected = connected || w1.ShareCommonPointWith(*c);
        }
        //overlaping with transitions
        for(auto && t : _transitions) {
            if(w1.Overlapp(*t)) {
                Log->Write(
                    "ERROR: Overlapping between wall %s and  transition %s ",
                    w1.toString().c_str(),
                    t->toString().c_str());
                exit(EXIT_FAILURE);
                //return false;
            }
            connected = connected || w1.ShareCommonPointWith(*t);
        }

        if(!connected) {
            Log->Write(
                "ERROR: loose wall found %s  in Room/Subroom %d/%d",
                w1.toString().c_str(),
                _roomID,
                _id);
            exit(EXIT_FAILURE);
            //return false;
        }
    }

    //check overlaping between hline and crossings
    for(auto && h : _hlines) {
        for(auto && c : _crossings) {
            if(h->Overlapp(*c)) {
                Log->Write(
                    "ERROR: Overlapping between hline %s and  crossing %s ",
                    h->toString().c_str(),
                    c->toString().c_str());
                exit(EXIT_FAILURE);
                //return false;
            }
        }
    }
    //check overlaping between hlines and transitions
    for(auto && h : _hlines) {
        for(auto && t : _transitions) {
            if(h->Overlapp(*t)) {
                Log->Write(
                    "ERROR: Overlapping between hline %s and  transition %s ",
                    h->toString().c_str(),
                    t->toString().c_str());
                exit(EXIT_FAILURE);
                //return false;
            }
        }
    }
    //check overlaping between transitions and crossings
    for(auto && c : _crossings) {
        for(auto && t : _transitions) {
            if(c->Overlapp(*t)) {
                Log->Write(
                    "ERROR: Overlapping between crossing %s and  transition %s ",
                    c->toString().c_str(),
                    t->toString().c_str());
                exit(EXIT_FAILURE);
                //return false;
            }
        }
    }

    return true;
}

///http://stackoverflow.com/questions/471962/how-do-determine-if-a-polygon-is-complex-convex-nonconvex
bool SubRoom::IsConvex()
{
    unsigned int hsize = (unsigned int) _poly.size();
    unsigned int pos   = 0;
    unsigned int neg   = 0;

    if(hsize == 0) {
        Log->Write("WARNING:\t cannot check empty polygon for convexification");
        Log->Write("WARNING:\t Did you forget to tall ConvertLineToPoly() ?");
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
    if(_poly.size() < 3) {
        Log->Write(
            "ERROR:\tYou need at least 3 vertices to check for orientation. Subroom ID [%d]");
        return false;
        //exit(EXIT_FAILURE);
    }

    Point vecAB = _poly[1] - _poly[0];
    Point vecBC = _poly[2] - _poly[1];

    double det = vecAB.Determinant(vecBC);
    return (fabs(det) < J_EPS);
}

bool SubRoom::IsPartOfPolygon(const Point & ptw)
{
    if(false == IsElementInVector(_poly, ptw)) {
        //maybe the point was too closed to other points and got replaced
        //check that eventuality
        bool near = false;
        for(const auto & pt : _poly) {
            if((pt - ptw).Norm() < J_TOLERANZ) {
                near = true;
                break;
            }
        }

        if(near == false) {
            return false;
        }
    }
    return true;
}

/************************************************************
 NormalSubRoom
 ************************************************************/
NormalSubRoom::NormalSubRoom() : SubRoom() {}


NormalSubRoom::~NormalSubRoom() {}

string NormalSubRoom::WriteSubRoom() const
{
    string s;
    for(auto && w : _walls) {
        string geometry;
        char wall[CLENGTH] = "";
        geometry.append("\t\t<wall>\n");
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint1().GetX()) * FAKTOR,
            (w.GetPoint1().GetY()) * FAKTOR,
            GetElevation(w.GetPoint1()) * FAKTOR);
        geometry.append(wall);
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint2().GetX()) * FAKTOR,
            (w.GetPoint2().GetY()) * FAKTOR,
            GetElevation(w.GetPoint2()) * FAKTOR);
        geometry.append(wall);
        geometry.append("\t\t</wall>\n");

        s.append(geometry);
        //s.append(GetWall(j).Write());
    }

    //add the subroom caption
    const Point & pos = GetCentroid();
    char tmp[CLENGTH];
    sprintf(
        tmp,
        "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" text=\"%d\" color=\"100\" "
        "/>\n",
        pos.GetX() * FAKTOR,
        pos.GetY() * FAKTOR,
        GetElevation(pos) * FAKTOR,
        GetSubRoomID());
    s.append(tmp);

    //write the obstacles
    for(auto && obst : GetAllObstacles()) {
        for(auto && w : obst->GetAllWalls()) {
            char wall[CLENGTH] = "";
            s.append("\t\t<wall>\n");
            sprintf(
                wall,
                "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                (w.GetPoint1().GetX()) * FAKTOR,
                (w.GetPoint1().GetY()) * FAKTOR,
                GetElevation(w.GetPoint1()) * FAKTOR);
            s.append(wall);
            sprintf(
                wall,
                "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                (w.GetPoint2().GetX()) * FAKTOR,
                (w.GetPoint2().GetY()) * FAKTOR,
                GetElevation(w.GetPoint2()) * FAKTOR);
            s.append(wall);
            s.append("\t\t</wall>\n");
        }

        const Point & posCenter = obst->GetCentroid();

        //add the obstacle caption
        char Tmp[CLENGTH];
        sprintf(
            Tmp,
            "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" text=\"%d\" "
            "color=\"100\" />\n",
            posCenter.GetX() * FAKTOR,
            posCenter.GetY() * FAKTOR,
            GetElevation(posCenter) * FAKTOR,
            obst->GetId());
        s.append(Tmp);
    }

    return s;
}

string NormalSubRoom::WritePolyLine() const
{
    string s;
    char tmp[CLENGTH];

    s.append("\t<Obstacle closed=\"1\" boundingbox=\"0\" class=\"1\">\n");
    for(const auto & p : _poly) {
        sprintf(
            tmp,
            "\t\t<Vertex p_x = \"%.2lf\" p_y = \"%.2lf\"/>\n",
            p.GetX() * FAKTOR,
            p.GetY() * FAKTOR);
        s.append(tmp);
    }
    s.append("\t</Obstacle>\n");

    //write the obstacles
    for(auto && obst : _obstacles) {
        s.append(obst->Write());
    }
    return s;
}

void NormalSubRoom::WriteToErrorLog() const
{
    Log->Write("\t\tNormal SubRoom:\n");
    for(auto && w : _walls) {
        w.WriteToErrorLog();
    }
}

bool NormalSubRoom::ConvertLineToPoly(const vector<Line *> & goals)
{
    vector<Line *> copy;
    vector<Point> tmpPoly;
    Point point;
    Line * line;
    // Alle Linienelemente in copy speichern
    for(auto & w : _walls) {
        copy.push_back(&w);
    }

    // Transitions und Crossings sind in goal abgespeichert
    copy.insert(copy.end(), goals.begin(), goals.end());

    if(Overlapp(goals)) {
        Log->Write("ERROR:\t Overlapping between walls and goals");
        return false;
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
        char tmp[CLENGTH];
        sprintf(
            tmp,
            "ERROR: \tNormalSubRoom::ConvertLineToPoly(): SubRoom %d Room %d Anfangspunkt ungleich "
            "Endpunkt!!!\n"
            "\t(%f, %f) != (%f, %f)\n",
            GetSubRoomID(),
            GetRoomID(),
            tmpPoly[0].GetX(),
            tmpPoly[0].GetY(),
            point.GetX(),
            point.GetY());
        Log->Write(tmp);
        sprintf(
            tmp, "ERROR: \tDistance between the points: %lf !!!\n", (tmpPoly[0] - point).Norm());
        Log->Write(tmp);
        return false;
    }
    _poly = tmpPoly;


    //check if all walls and goals were used in the polygon
    for(const auto & w : _walls) {
        for(const auto & ptw : {w.GetPoint1(), w.GetPoint2()}) {
            if(IsPartOfPolygon(ptw) == false) {
                Log->Write(
                    "ERROR:\t Wall was not used during polygon creation for subroom: %s",
                    w.toString().c_str());
                return false;
            }
        }
    }

    for(const auto & g : goals) {
        for(const auto & ptw : {g->GetPoint1(), g->GetPoint2()}) {
            if(IsPartOfPolygon(ptw) == false) {
                Log->Write(
                    "ERROR:\t goal was not used during polygon creation for subroom: %s",
                    g->toString().c_str());
                return false;
            }
        }
    }
    return true;
}


// private Funktionen

// gibt zurück in welchen Quadranten vertex liegt, wobei hitPos der Koordinatenursprung ist

int NormalSubRoom::WhichQuad(const Point & vertex, const Point & hitPos) const
{
    return (vertex.GetX() > hitPos.GetX()) ? ((vertex.GetY() > hitPos.GetY()) ? 1 : 4) :
                                             ((vertex.GetY() > hitPos.GetY()) ? 2 : 3);
}

// x-Koordinate der Linie von einer Eccke zur nächsten

double NormalSubRoom::Xintercept(const Point & point1, const Point & point2, double hitY) const
{
    return (
        point2.GetX() - (((point2.GetY() - hitY) * (point1.GetX() - point2.GetX())) /
                         (point1.GetY() - point2.GetY())));
}


// neue Version auch für konkave Polygone

bool NormalSubRoom::IsInSubRoom(const Point & ped) const
{
    //case when the point is on an edge
    // todo: this affect the runtime, and do we really need that
    // If we do not d othis check, then for a square for instance, half the points located on the edge will be inside and
    // the other half will be outside the polygon.
    for(auto & w : _walls) {
        if(w.IsInLineSegment(ped))
            return true;
    }


    short edge, first, next;
    short quad, next_quad, delta, total;

    /////////////////////////////////////////////////////////////
    edge = first = 0;
    quad         = (short) WhichQuad(_poly[edge], ped);
    total        = 0; // COUNT OF ABSOLUTE SECTORS CROSSED
    /* LOOP THROUGH THE VERTICES IN A SECTOR */
    do {
        next      = (edge + 1) % _poly.size();
        next_quad = WhichQuad(_poly[next], ped);
        delta     = next_quad - quad; // HOW MANY QUADS HAVE I MOVED

        // SPECIAL CASES TO HANDLE CROSSINGS OF MORE THEN ONE
        //QUAD

        switch(delta) {
            case 2:  // IF WE CROSSED THE MIDDLE, FIGURE OUT IF IT
                     //WAS CLOCKWISE OR COUNTER
            case -2: // US THE X POSITION AT THE HIT POINT TO
                // DETERMINE WHICH WAY AROUND
                if(Xintercept(_poly[edge], _poly[next], ped._y) > ped._x)
                    delta = -(delta);
                break;
            case 3: // MOVING 3 QUADS IS LIKE MOVING BACK 1
                delta = -1;
                break;
            case -3: // MOVING BACK 3 IS LIKE MOVING FORWARD 1
                delta = 1;
                break;
            default:
                break;
        }
        /* ADD IN THE DELTA */
        total += delta;
        quad = next_quad; // RESET FOR NEXT STEP
        edge = next;
    } while(edge != first);

    /* AFTER ALL IS DONE IF THE TOTAL IS 4 THEN WE ARE INSIDE */
    if(abs(total) == 4)
        return true;
    else
        return false;
}

/************************************************************
 Stair
 ************************************************************/

Stair::Stair() : NormalSubRoom()
{
    pUp   = Point();
    pDown = Point();
}


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

string Stair::WriteSubRoom() const
{
    string s;
    for(auto && w : _walls) {
        string geometry;
        char wall[CLENGTH] = "";
        geometry.append("\t\t<wall>\n");
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint1().GetX()) * FAKTOR,
            (w.GetPoint1().GetY()) * FAKTOR,
            GetElevation(w.GetPoint1()) * FAKTOR);
        geometry.append(wall);
        sprintf(
            wall,
            "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
            (w.GetPoint2().GetX()) * FAKTOR,
            (w.GetPoint2().GetY()) * FAKTOR,
            GetElevation(w.GetPoint2()) * FAKTOR);
        geometry.append(wall);
        geometry.append("\t\t</wall>\n");

        s.append(geometry);
        //s.append(w.Write());
    }
    //Line tmp = Line(GetUp(), GetDown());
    // s.append(tmp.Write());
    Point pos = GetCentroid();
    char tmp_c[CLENGTH];
    sprintf(
        tmp_c,
        "\t\t<sphere centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" radius=\"%.2f\" "
        "color=\"100\" />\n",
        GetUp().GetX() * FAKTOR,
        GetUp().GetY() * FAKTOR,
        GetElevation(GetUp()) * FAKTOR,
        0.2 * FAKTOR);
    s.append(tmp_c);

    //add the subroom caption
    sprintf(
        tmp_c,
        "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" text=\"%d\" color=\"100\" "
        "/>\n",
        pos.GetX() * FAKTOR,
        pos.GetY() * FAKTOR,
        GetElevation(pos) * FAKTOR,
        GetSubRoomID());
    s.append(tmp_c);

    return s;
}

string Stair::WritePolyLine() const
{
    string s;
    char tmp[CLENGTH];

    s.append("\t<Obstacle closed=\"1\" boundingbox=\"0\" class=\"1\">\n");
    for(unsigned int j = 0; j < _poly.size(); j++) {
        sprintf(
            tmp,
            "\t\t<Vertex p_x = \"%.2lf\" p_y = \"%.2lf\"/>\n",
            _poly[j].GetX() * FAKTOR,
            _poly[j].GetY() * FAKTOR);
        s.append(tmp);
    }
    s.append("\t</Obstacle>\n");

    //write the obstacles
    for(unsigned int j = 0; j < GetAllObstacles().size(); j++) {
        s.append(GetAllObstacles()[j]->Write());
    }

    return s;
}
void Stair::WriteToErrorLog() const
{
    Log->Write("\t\tStair:\n");
    for(auto && w : _walls) {
        w.WriteToErrorLog();
    }
}

/* prüft ob die Punkte p1, p2 und p3 auf einer Linie liegen, oder eine Ecke bilden.
 * Dabei liegt p2 IMMER in der Mitte und entspricht aktPoint
 * */
const Point *
Stair::CheckCorner(const Point ** otherPoint, const Point ** aktPoint, const Point * nextPoint)
{
    Point l1            = **otherPoint - **aktPoint;
    Point l2            = *nextPoint - **aktPoint;
    const Point * rueck = NULL;
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

bool Stair::ConvertLineToPoly(const vector<Line *> & goals)
{
    //return NormalSubRoom::ConvertLineToPoly(goals);
    vector<Line *> copy;
    vector<Point> orgPoly = vector<Point>();
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
        Log->Write("ERROR:\t Overlapping between walls and goals");
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
        nextPoint = NULL;
        if((*aktPoint - nextLine->GetPoint1()).Norm() < J_TOLERANZ) {
            nextPoint = &nextLine->GetPoint2();
        } else if((*aktPoint - nextLine->GetPoint2()).Norm() < J_TOLERANZ) {
            nextPoint = &nextLine->GetPoint1();
        }
        if(nextPoint != NULL) {
            const Point * rueck = CheckCorner(&otherPoint, &aktPoint, nextPoint);
            if(rueck != NULL)
                orgPoly.push_back(*rueck);
            copy.erase(copy.begin() + i);
            i = -1; // von vorne suchen
        }
    }
    if((*aktPoint - *firstOtherPoint).Norm() < J_TOLERANZ) {
        const Point * rueck = CheckCorner(&otherPoint, &aktPoint, firstAktPoint);
        if(rueck != NULL)
            orgPoly.push_back(*rueck);
    } else {
        char tmp[CLENGTH];
        double x1, y1, x2, y2;
        x1 = firstOtherPoint->GetX();
        y1 = firstOtherPoint->GetY();
        x2 = aktPoint->GetX();
        y2 = aktPoint->GetY();
        sprintf(
            tmp,
            "ERROR: \tStair::ConvertLineToPoly(): SubRoom %d Room %d Anfangspunkt ungleich "
            "Endpunkt!!!\n"
            "\t(%f, %f) != (%f, %f)\n",
            GetSubRoomID(),
            GetRoomID(),
            x1,
            y1,
            x2,
            y2);
        Log->Write(tmp);
        return false;
    }

    if(orgPoly.size() != 4) {
        char tmp[CLENGTH];
        sprintf(
            tmp,
            "ERROR: \tStair::ConvertLineToPoly(): Stair %d Room %d ist kein Viereck!!!\n"
            "Anzahl Ecken: %d\n",
            GetSubRoomID(),
            (int) GetRoomID(),
            (int) orgPoly.size());
        Log->Write(tmp);
        return false;
    }
    vector<Point> neuPoly = (orgPoly);
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

    //check if all walls and goals were used in the polygon
    // will not work here, since the stairs only consist of 4 vertices

    //     for (const auto& w: _walls)
    //     {
    //          for (const auto & ptw: {w.GetPoint1(),w.GetPoint2()})
    //          {
    //               if(IsPartOfPolygon(ptw)==false)
    //               {
    //                    Log->Write("ERROR:\t Wall was not used during polygon creation for subroom: %s",w.toString().c_str());
    //                    return false;
    //               }
    //          }
    //     }
    //
    //    for (const auto& g: goals)
    //    {
    //
    //         for (const auto & ptw: {g->GetPoint1(),g->GetPoint2()})
    //         {
    //              if(IsPartOfPolygon(ptw)==false)
    //              {
    //                   Log->Write("ERROR:\t goal was not used during polygon creation for subroom: %s",g->toString().c_str());
    //                   return false;
    //              }
    //         }
    //    }

    return true;
}

bool Stair::IsInSubRoom(const Point & ped) const
{
    bool rueck = false;
    int N      = (int) _poly.size();
    int sum    = 0;

    for(int i = 0; i < N; i++) {
        Line l  = Line(_poly[i], _poly[(i + 1) % N]);
        Point s = l.LotPoint(ped);
        if(l.IsInLineSegment(s))
            sum++;
    }
    if(sum == 4)
        rueck = true;

    return rueck;
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
    if(ped->GetExitLine()->DistTo(pos) <= J_EPS_GOAL)
        return true;
    else
        return IsInSubRoom(pos);
}


#endif // _SIMULATOR
