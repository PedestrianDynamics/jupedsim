/**
 * \file        SubRoom.cpp
 * \date        Oct 8, 2010
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#include "Point.h"
#include "Line.h"
#include "Wall.h"
#include "Obstacle.h"
#include "SubRoom.h"
#include "Transition.h"
#include "Hline.h"


#ifdef _SIMULATOR
#include "../pedestrian/Pedestrian.h"
#endif //_SIMULATOR

#include <cmath>

using namespace std;

/************************************************************
 SubRoom
 ************************************************************/

int SubRoom::_static_uid=0;

SubRoom::SubRoom()
{
     _id = -1;
     _roomID=-1;
     _walls = vector<Wall > ();
     _poly = vector<Point > ();
     _obstacles=vector<Obstacle*> ();

     _crossings = vector<Crossing*>();
     _transitions = vector<Transition*>();
     _hlines = vector<Hline*>();

     _planeEquation[0]=0.0;
     _planeEquation[1]=0.0;
     _planeEquation[2]=0.0;
     _cosAngleWithHorizontalPlane=0;

     _goalIDs = vector<int> ();
     _area = 0.0;
     _closed=false;
     _uid = _static_uid++;

#ifdef _SIMULATOR
     _peds = vector<Pedestrian* > ();
#endif //_SIMULATOR

}

SubRoom::SubRoom(const SubRoom& orig)
{
     _id = orig.GetSubRoomID();
     _walls = orig.GetAllWalls();
     _poly = orig.GetPolygon();
     _goalIDs = orig.GetAllGoalIDs();
     _area = orig.GetArea();
     _closed=orig.GetClosed();
     _roomID=orig.GetRoomID();
     _uid = orig.GetUID();
     _cosAngleWithHorizontalPlane=orig.GetCosAngleWithHorizontal();

#ifdef _SIMULATOR
     _peds = orig.GetAllPedestrians();
#endif //_SIMULATOR
}

SubRoom::~SubRoom()
{
     if (_walls.size() > 0) _walls.clear();
     if (_poly.size() > 0) _poly.clear();
     for (unsigned int i = 0; i < _obstacles.size(); i++) {
          delete _obstacles[i];
     }
     _obstacles.clear();

#ifdef _SIMULATOR
     for (unsigned int i = 0; i < _peds.size(); i++) {
          delete _peds[i];
     }
#endif //_SIMULATOR

}

// Setter -Funktionen

void SubRoom::SetSubRoomID(int ID)
{
     _id = ID;
}
void SubRoom::SetClosed(double closed)
{
     _closed = closed;
}

void SubRoom::SetRoomID(int ID)
{
     _roomID = ID;
}

int SubRoom::GetSubRoomID() const
{
     return _id;
}

double SubRoom::GetClosed() const
{
     return _closed;
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

int SubRoom::GetNumberOfWalls() const
{
     return _walls.size();
}

const vector<Wall>& SubRoom::GetAllWalls() const
{
     return _walls;
}

const Wall& SubRoom::GetWall(int index) const
{
     if ((index >= 0) && (index < GetNumberOfWalls()))
          return _walls[index];
     else {
          Log->Write("ERROR: Wrong 'index' in SubRoom::GetWall()");
          exit(0);
     }
}

const vector<Point>& SubRoom::GetPolygon() const
{
     return _poly;
}

const vector<Obstacle*>& SubRoom::GetAllObstacles() const
{
     return _obstacles;
}

int SubRoom::GetNumberOfGoalIDs() const
{
     return _goalIDs.size();
}

const vector<int>& SubRoom::GetAllGoalIDs() const
{
     return _goalIDs;
}


// Sonstiges

void SubRoom::AddWall(const Wall& w)
{
     _walls.push_back(w);
}


void SubRoom::AddObstacle(Obstacle* obs)
{
     _obstacles.push_back(obs);
     CheckObstacles();
}


void SubRoom::AddGoalID(int ID)
{
     _goalIDs.push_back(ID);
}

void SubRoom::AddCrossing(Crossing* line)
{
     _crossings.push_back(line);
     _goalIDs.push_back(line->GetUniqueID());
}

void SubRoom::AddTransition(Transition* line)
{
     _transitions.push_back(line);
     _goalIDs.push_back(line->GetUniqueID());
}

void SubRoom::AddHline(Hline* line)
{
    for(unsigned int i=0;i<_hlines.size();i++){
        if (line->GetID()==_hlines[i]->GetID()){
            Log->Write("INFO:\tskipping duplicate hline [%d] in subroom [%d]",_id,line->GetID());
            return;
        }
    }

    _hlines.push_back(line);
    _goalIDs.push_back(line->GetUniqueID());
}

const vector<Crossing*>& SubRoom::GetAllCrossings() const
{
     return _crossings;
}

const vector<Transition*>& SubRoom::GetAllTransitions() const
{
     return _transitions;
}

const vector<Hline*>& SubRoom::GetAllHlines() const
{
     return _hlines;
}

const Crossing* SubRoom::GetCrossing(int i) const
{
     return _crossings[i];
}

const Transition* SubRoom::GetTransition(int i) const
{
     return _transitions[i];
}

const Hline* SubRoom::GetHline(int i) const
{
     return _hlines[i];
}

void SubRoom::RemoveGoalID(int ID)
{
     for (unsigned int i=0; i<_goalIDs.size(); i++) {
          if(_goalIDs[i]==ID) {
               Log->Write("Removing goal");
               _goalIDs.erase(_goalIDs.begin()+i);
               return;
          }
     }
     Log->Write("There is no goal with that id to remove");
}


void SubRoom::CalculateArea()
{
     double sum = 0;
     int n = (int) _poly.size();
     for (int i = 0; i < n; i++) {
          sum += (_poly[i].GetY() + _poly[(i + 1) % n].GetY())*(_poly[i].GetX() - _poly[(i + 1) % n].GetX());
     }
     _area=(0.5 * fabs(sum));
}

Point SubRoom::GetCentroid() const
{

     double px=0,py=0;
     double signedArea = 0.0;
     double x0 = 0.0; // Current vertex X
     double y0 = 0.0; // Current vertex Y
     double x1 = 0.0; // Next vertex X
     double y1 = 0.0; // Next vertex Y
     double a = 0.0;  // Partial signed area

     // For all vertices except last
     unsigned int i=0;
     for (i=0; i<_poly.size()-1; ++i) {
          x0 = _poly[i].GetX();
          y0 = _poly[i].GetY();
          x1 = _poly[i+1].GetX();
          y1 = _poly[i+1].GetY();
          a = x0*y1 - x1*y0;
          signedArea += a;
          px += (x0 + x1)*a;
          py += (y0 + y1)*a;
     }

     // Do last vertex
     x0 = _poly[i].GetX();
     y0 = _poly[i].GetY();
     x1 = _poly[0].GetX();
     y1 = _poly[0].GetY();
     a = x0*y1 - x1*y0;
     signedArea += a;
     px += (x0 + x1)*a;
     py += (y0 + y1)*a;

     signedArea *= 0.5;
     px /= (6.0*signedArea);
     py /= (6.0*signedArea);

     return Point(px,py);
}

bool SubRoom::IsVisible(const Point& p1, const Point& p2, bool considerHlines)
{
     // generate certain connection lines
     // connecting p1 with p2
     Line cl = Line(p1,p2);
     bool temp =  true;
     //check intersection with Walls
     for(unsigned int i = 0; i < _walls.size(); i++) {
          if(temp  && cl.IntersectionWith(_walls[i]))
               temp = false;
     }


     //check intersection with obstacles
     for(unsigned int i = 0; i < _obstacles.size(); i++) {
          Obstacle * obs = _obstacles[i];
          for(unsigned int k = 0; k<obs->GetAllWalls().size(); k++) {
               const Wall& w = obs->GetAllWalls()[k];
               if(temp && cl.IntersectionWith(w))
                    temp = false;
          }
     }


     // check intersection with other hlines in room
     if(considerHlines)
          for(unsigned int i = 0; i < _hlines.size(); i++) {
               if(_hlines[i]->IsInLineSegment(p1)|| _hlines[i]->IsInLineSegment(p2)) continue;
               if(temp && cl.IntersectionWith(*(Line*)_hlines[i]))
                    temp = false;
          }

     return temp;
}

bool SubRoom::IsVisible(Line* l1, Line* l2, bool considerHlines)
{
     // generate certain connection lines
     // connecting p1 mit p1, p1 mit p2, p2 mit p1, p2 mit p2 und center mit center
     Line cl[5];
     cl[0] = Line(l1->GetPoint1(), l2->GetPoint1());
     cl[1] = Line(l1->GetPoint1(), l2->GetPoint2());
     cl[2] = Line(l1->GetPoint2(), l2->GetPoint1());
     cl[3] = Line(l1->GetPoint2(), l2->GetPoint2());
     cl[4] = Line(l1->GetCentre(), l2->GetCentre());
     bool temp[5] = {true, true, true, true, true};
     //check intersection with Walls
     for(unsigned int i = 0; i <  GetAllWalls().size(); i++) {
          for(int k = 0; k < 5; k++) {
               if(temp[k] && cl[k].IntersectionWith(_walls[i]) && (cl[k].NormalVec() != _walls[i].NormalVec() ||  l1->NormalVec() != l2->NormalVec()))
                    temp[k] = false;
          }
     }

     //check intersection with obstacles
     for(unsigned int i = 0; i <  GetAllObstacles().size(); i++) {
          Obstacle * obs =  GetAllObstacles()[i];
          for(unsigned int k = 0; k<obs->GetAllWalls().size(); k++) {
               const Wall& w = obs->GetAllWalls()[k];
               if((w.operator !=(*l1)) && (w.operator !=(*l2)))
                    for(int j = 0; j < 5; j++) {
                         if(temp[j] && cl[j].IntersectionWith(w))
                              temp[j] = false;
                    }
          }
     }

     // check intersection with other hlines in room
     if(considerHlines)
          for(unsigned int i = 0; i <  _hlines.size(); i++) {
               if ( (l1->operator !=(*(Line*)_hlines[i])) &&  (l2->operator !=(*(Line*)_hlines[i])) ) {
                    for(int k = 0; k < 5; k++) {
                         if(temp[k] && cl[k].IntersectionWith(*(Line*)_hlines[i]))
                              temp[k] = false;
                    }
               }
          }
     return temp[0] || temp[1] || temp[2] || temp[3] || temp[4];
}




// this is the case if they share a transition or crossing
bool SubRoom::IsDirectlyConnectedWith(const SubRoom* sub) const
{

     //check the crossings
     const vector<Crossing*>& crossings = sub->GetAllCrossings();
     for (unsigned int i = 0; i < crossings.size(); i++) {
          for (unsigned int j = 0; j < _crossings.size(); j++) {
               int uid1 = crossings[i]->GetUniqueID();
               int uid2 = _crossings[j]->GetUniqueID();
               // ignore my transition
               if (uid1 == uid2)
                    return true;
          }
     }

     // and finally the transitions
     const vector<Transition*>& transitions = sub->GetAllTransitions();
     for (unsigned int i = 0; i < transitions.size(); i++) {
          for (unsigned int j = 0; j < _transitions.size(); j++) {
               int uid1 = transitions[i]->GetUniqueID();
               int uid2 = _transitions[j]->GetUniqueID();
               // ignore my transition
               if (uid1 == uid2)
                    return true;
          }
     }

     return false;
}

void SubRoom::SetPlanEquation(double A, double B, double C)
{
     _planeEquation[0]=A;
     _planeEquation[1]=B;
     _planeEquation[2]=C;
     //compute and cache the cosine of angle with the plane z=h
     _cosAngleWithHorizontalPlane= (1.0/sqrt(A*A+B*B+1));
}

const double* SubRoom::GetPlanEquation() const
{
     return _planeEquation;
}

double SubRoom::GetElevation(const Point& p) const
{
     return _planeEquation[0] * p._x + _planeEquation[1] * p._y + _planeEquation[2];
}

double SubRoom::GetCosAngleWithHorizontal() const
{
     return _cosAngleWithHorizontalPlane;

}

void SubRoom::CheckObstacles()
{
     for(unsigned int i = 0; i<_walls.size(); i++) {
          for(unsigned int j = 0; j<_obstacles.size(); j++) {
               if(_obstacles[j]->IntersectWithLine(_walls[i])) {
                    Log->Write("INFO: \tthe obstacle id [%d] is intersection with subroom [%d]",_obstacles[j]->GetId(),_id);
                    Log->Write("INFO: \tthe triangulation will not work.");
                    exit(EXIT_FAILURE);
               }
          }
     }
}

void SubRoom::SanityCheck()
{
     if(_obstacles.size()==0) {
          if((IsConvex()==false) && (_hlines.size()==0)) {
               Log->Write("WARNING:\t Room [%d] Subroom [%d] is not convex!",_roomID,_id);
               Log->Write("\t\t you might consider adding extra hlines in your routing.xml file");
          } else {
               // everything is fine
          }
     } else {
          if(_hlines.size()==0) {
               Log->Write("WARNING:\t you have obstacles in room [%d] Subroom [%d]!",_roomID,_id);
               Log->Write("\t\t you might consider adding extra hlines in your routing.xml file");
          } else {
               // everything is fine
          }
     }

}

///http://stackoverflow.com/questions/471962/how-do-determine-if-a-polygon-is-complex-convex-nonconvex
bool SubRoom::IsConvex()
{
     unsigned int hsize=_poly.size();
     unsigned int pos=0;
     unsigned int neg=0;

     if(hsize==0) {
          Log->Write("WARNING:\t cannot check empty polygon for convexification");
          Log->Write("WARNING:\t Did you forget to tall ConvertLineToPoly() ?");
          return false;
     }

     for(unsigned int i=0; i<hsize; i++) {
          Point vecAB= _poly[(i+1)%hsize]-_poly[i%hsize];
          Point vecBC= _poly[(i+2)%hsize]-_poly[(i+1)%hsize];
          double det= vecAB.Det(vecBC);
          if(fabs(det)<J_EPS) det=0.0;

          if(det<0.0) {
               neg++;
          } else if(det>0.0) {
               pos++;
          } else {
               pos++;
               neg++;
          }

     }

     if ( (pos==hsize ) || (neg==hsize) ) {
          return true;
     }
     return false;
}

///http://stackoverflow.com/questions/9473570/polygon-vertices-clockwise-or-counterclockwise/
bool SubRoom::IsClockwise()
{
     if(_poly.size()<3) {
          Log->Write("ERROR:\tYou need at least 3 vertices to check for orientation. Subroom ID [%d]");
          return false;
          //exit(EXIT_FAILURE);
     }

     Point vecAB= _poly[1]-_poly[0];
     Point vecBC= _poly[2]-_poly[1];

     double det=vecAB.Det(vecBC);
     if(fabs(det)<J_EPS) det=0.0;

     return ( det<=0.0 );
}


/************************************************************
 NormalSubRoom
 ************************************************************/
NormalSubRoom::NormalSubRoom() : SubRoom()
{

}

NormalSubRoom::NormalSubRoom(const NormalSubRoom& orig) : SubRoom(orig)
{

}

NormalSubRoom::~NormalSubRoom()
{
}

string NormalSubRoom::WriteSubRoom() const
{
     string s;
     for (int j = 0; j < GetNumberOfWalls(); j++) {

          const Wall& w = GetWall(j);
          string geometry;
          char wall[CLENGTH] = "";
          geometry.append("\t\t<wall>\n");
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                  (w.GetPoint1().GetX()) * FAKTOR,
                  (w.GetPoint1().GetY()) * FAKTOR,
                  GetElevation(w.GetPoint1())*FAKTOR);
          geometry.append(wall);
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                  (w.GetPoint2().GetX()) * FAKTOR,
                  (w.GetPoint2().GetY()) * FAKTOR,
                  GetElevation(w.GetPoint2())*FAKTOR);
          geometry.append(wall);
          geometry.append("\t\t</wall>\n");

          s.append(geometry);
          //s.append(GetWall(j).Write());
     }
     //add the subroom caption
     Point pos = GetCentroid();
     char tmp[CLENGTH];
     sprintf(tmp, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" text=\"%d\" color=\"100\" />\n"
             , pos.GetX() * FAKTOR, pos.GetY() * FAKTOR, GetSubRoomID());
     s.append(tmp);

     //write the obstacles
     for( unsigned int j=0; j<GetAllObstacles().size(); j++) {
          s.append(GetAllObstacles()[j]->Write());
     }

     return s;
}

string NormalSubRoom::WritePolyLine() const
{

     string s;
     char tmp[CLENGTH];

     s.append("\t<Obstacle closed=\"1\" boundingbox=\"0\" class=\"1\">\n");
     for (unsigned int j = 0; j < _poly.size(); j++) {
          sprintf(tmp, "\t\t<Vertex p_x = \"%.2lf\" p_y = \"%.2lf\"/>\n",_poly[j].GetX()* FAKTOR,_poly[j].GetY()* FAKTOR);
          s.append(tmp);
     }
     s.append("\t</Obstacle>\n");

     //write the obstacles
     for( unsigned int j=0; j<GetAllObstacles().size(); j++) {
          s.append(GetAllObstacles()[j]->Write());
     }

     return s;
}

void NormalSubRoom::WriteToErrorLog() const
{
     Log->Write("\t\tNormal SubRoom:\n");
     for (int i = 0; i < GetNumberOfWalls(); i++) {
          Wall w = GetWall(i);
          w.WriteToErrorLog();
     }
}

void NormalSubRoom::ConvertLineToPoly(vector<Line*> goals)
{
     vector<Line*> copy;
     vector<Point> tmpPoly;
     Point point;
     Line* line;
     // Alle Linienelemente in copy speichern
     for (int i = 0; i < GetNumberOfWalls(); i++) {
          copy.push_back(&_walls[i]);
     }
     // Transitions und Crossings sind in goal abgespeichert
     copy.insert(copy.end(), goals.begin(), goals.end());

     line = copy[0];
     tmpPoly.push_back(line->GetPoint1());
     point = line->GetPoint2();
     copy.erase(copy.begin());
     // Polygon aus allen Linen erzeugen
     for (int i = 0; i < (int) copy.size(); i++) {
          line = copy[i];
          if ((point - line->GetPoint1()).Norm() < J_TOLERANZ) {
               tmpPoly.push_back(line->GetPoint1());
               point = line->GetPoint2();
               copy.erase(copy.begin() + i);
               // von vorne suchen
               i = -1;
          } else if ((point - line->GetPoint2()).Norm() < J_TOLERANZ) {
               tmpPoly.push_back(line->GetPoint2());
               point = line->GetPoint1();
               copy.erase(copy.begin() + i);
               // von vorne suchen
               i = -1;
          }
     }
     if ((tmpPoly[0] - point).Norm() > J_TOLERANZ) {
          char tmp[CLENGTH];
          sprintf(tmp, "ERROR: \tNormalSubRoom::ConvertLineToPoly(): SubRoom %d Room %d Anfangspunkt ungleich Endpunkt!!!\n"
                  "\t(%f, %f) != (%f, %f)\n", GetSubRoomID(), GetRoomID(), tmpPoly[0].GetX(), tmpPoly[0].GetY(), point.GetX(),
                  point.GetY());
          Log->Write(tmp);
          sprintf(tmp, "ERROR: \tDistance between the points: %lf !!!\n", (tmpPoly[0] - point).Norm());
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
     _poly = tmpPoly;
}


// private Funktionen

// gibt zuruck in welchen Quadranten vertex liegt, wobei hitPos der Koordinatenursprung ist

int NormalSubRoom::WhichQuad(const Point& vertex, const Point& hitPos) const
{
     return (vertex.GetX() > hitPos.GetX()) ? ((vertex.GetY() > hitPos.GetY()) ? 1 : 4) :
                 ((vertex.GetY() > hitPos.GetY()) ? 2 : 3);

}

// x-Koordinate der Linie von einer Eccke zur nächsten

double NormalSubRoom::Xintercept(const Point& point1, const Point& point2, double hitY) const
{
     return (point2.GetX() - (((point2.GetY() - hitY) * (point1.GetX() - point2.GetX())) /
                              (point1.GetY() - point2.GetY())));
}


// neue Version auch für konkave Polygone

bool NormalSubRoom::IsInSubRoom(const Point& ped) const
{
     short edge, first, next;
     short quad, next_quad, delta, total;

     /////////////////////////////////////////////////////////////
     edge = first = 0;
     quad = WhichQuad(_poly[edge], ped);
     total = 0; // COUNT OF ABSOLUTE SECTORS CROSSED
     /* LOOP THROUGH THE VERTICES IN A SECTOR */
     do {
          next = (edge + 1) % _poly.size();
          next_quad = WhichQuad(_poly[next], ped);
          delta = next_quad - quad; // HOW MANY QUADS HAVE I MOVED

          // SPECIAL CASES TO HANDLE CROSSINGS OF MORE THEN ONE
          //QUAD

          switch (delta) {
          case 2: // IF WE CROSSED THE MIDDLE, FIGURE OUT IF IT
               //WAS CLOCKWISE OR COUNTER
          case -2: // US THE X POSITION AT THE HIT POINT TO
               // DETERMINE WHICH WAY AROUND
               if (Xintercept(_poly[edge], _poly[next], ped._y) > ped._x)
                    delta = -(delta);
               break;
          case 3: // MOVING 3 QUADS IS LIKE MOVING BACK 1
               delta = -1;
               break;
          case -3: // MOVING BACK 3 IS LIKE MOVING FORWARD 1
               delta = 1;
               break;
          }
          /* ADD IN THE DELTA */
          total += delta;
          quad = next_quad; // RESET FOR NEXT STEP
          edge = next;
     } while (edge != first);

     /* AFTER ALL IS DONE IF THE TOTAL IS 4 THEN WE ARE INSIDE */
     if (abs(total) == 4)
          return true;
     else
          return false;
}

/************************************************************
 Stair
 ************************************************************/

Stair::Stair() : NormalSubRoom()
{
     pUp = Point();
     pDown = Point();
}

Stair::Stair(const Stair & orig) : NormalSubRoom(orig)
{
     pUp = orig.GetUp();
     pDown = orig.GetDown();
}

Stair::~Stair()
{
}

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

     for (int j = 0; j < GetNumberOfWalls(); j++) {
          const Wall& w = GetWall(j);

          string geometry;
          char wall[CLENGTH] = "";
          geometry.append("\t\t<wall>\n");
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                  (w.GetPoint1().GetX()) * FAKTOR,
                  (w.GetPoint1().GetY()) * FAKTOR,
                  GetElevation(w.GetPoint1())*FAKTOR);
          geometry.append(wall);
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\"/>\n",
                  (w.GetPoint2().GetX()) * FAKTOR,
                  (w.GetPoint2().GetY()) * FAKTOR,
                  GetElevation(w.GetPoint2())*FAKTOR);
          geometry.append(wall);
          geometry.append("\t\t</wall>\n");

          s.append(geometry);
          //s.append(w.Write());
     }
     //Line tmp = Line(GetUp(), GetDown());
     // s.append(tmp.Write());
     Point pos = GetCentroid();
     char tmp_c[CLENGTH];
     sprintf(tmp_c, "\t\t<sphere centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" radius=\"%.2f\" color=\"100\" />\n"
             , GetUp().GetX() * FAKTOR, GetUp().GetY() * FAKTOR,GetElevation(GetUp())*FAKTOR, 0.2*FAKTOR);
     s.append(tmp_c);

     //add the subroom caption
     sprintf(tmp_c, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"%.2f\" text=\"%d\" color=\"100\" />\n"
             , pos.GetX() * FAKTOR, pos.GetY() * FAKTOR,GetElevation(pos)*FAKTOR ,GetSubRoomID());
     s.append(tmp_c);

     return s;
}

string Stair::WritePolyLine() const
{

     string s;
     char tmp[CLENGTH];

     s.append("\t<Obstacle closed=\"1\" boundingbox=\"0\" class=\"1\">\n");
     for (unsigned int j = 0; j < _poly.size(); j++) {
          sprintf(tmp, "\t\t<Vertex p_x = \"%.2lf\" p_y = \"%.2lf\"/>\n",_poly[j].GetX()* FAKTOR,_poly[j].GetY()* FAKTOR);
          s.append(tmp);
     }
     s.append("\t</Obstacle>\n");

     //write the obstacles
     for( unsigned int j=0; j<GetAllObstacles().size(); j++) {
          s.append(GetAllObstacles()[j]->Write());
     }

     return s;
}
void Stair::WriteToErrorLog() const
{
     Log->Write("\t\tStair:\n");
     for (int i = 0; i < GetNumberOfWalls(); i++) {
          Wall w = GetWall(i);
          w.WriteToErrorLog();
     }
}

/* prüft ob die Punkte p1, p2 und p3 auf einer Linie liegen, oder eine Ecke bilden.
 * Dabei liegt p2 IMMER in der Mitte und entspricht aktPoint
 * */
const Point* Stair::CheckCorner(const Point** otherPoint, const Point** aktPoint, const Point* nextPoint)
{

     Point l1 = **otherPoint - **aktPoint;
     Point l2 = *nextPoint - **aktPoint;
     const Point* rueck = NULL;
     // Punkte bilden eine Linie
     if (fabs(fabs(l1.ScalarP(l2) / (l1.Norm() * l2.Norm())) - 1) < 0.1) {
          *aktPoint = nextPoint;
     } else { // aktPoint/p2 ist eine Ecke
          rueck = *aktPoint;
          *otherPoint = *aktPoint;
          *aktPoint = nextPoint;
     }
     return rueck;
}

void Stair::ConvertLineToPoly(vector<Line*> goals)
{

     //return NormalSubRoom::ConvertLineToPoly(goals);

     vector<Line*> copy;
     vector<Point> orgPoly = vector<Point > ();
     const Point* aktPoint;
     const Point* otherPoint;
     const Point* nextPoint;
     const Point* firstAktPoint;
     const Point* firstOtherPoint;
     Line *nextLine;

     // Alle Linienelemente in copy speichern
     for (int i = 0; i < GetNumberOfWalls(); i++) {
          copy.push_back(&_walls[i]);
     }
     // Transitions und Crossings sind in goal abgespeichert
     copy.insert(copy.end(), goals.begin(), goals.end());

     aktPoint = &copy[0]->GetPoint1();
     firstAktPoint = aktPoint;
     otherPoint = &copy[0]->GetPoint2();
     firstOtherPoint = otherPoint;
     copy.erase(copy.begin());

     // Polygon aus allen Linen erzeugen
     for (int i = 0; i < (int) copy.size(); i++) {
          nextLine = copy[i];
          nextPoint = NULL;
          if ((*aktPoint - nextLine->GetPoint1()).Norm() < J_TOLERANZ) {
               nextPoint = &nextLine->GetPoint2();
          } else if ((*aktPoint - nextLine->GetPoint2()).Norm() < J_TOLERANZ) {
               nextPoint = &nextLine->GetPoint1();
          }
          if (nextPoint != NULL) {
               const Point* rueck = CheckCorner(&otherPoint, &aktPoint, nextPoint);
               if (rueck != NULL)
                    orgPoly.push_back(*rueck);
               copy.erase(copy.begin() + i);
               i = -1; // von vorne suchen
          }
     }
     if ((*aktPoint - *firstOtherPoint).Norm() < J_TOLERANZ) {
          const Point* rueck = CheckCorner(&otherPoint, &aktPoint, firstAktPoint);
          if (rueck != NULL)
               orgPoly.push_back(*rueck);
     } else {
          char tmp[CLENGTH];
          double x1, y1, x2, y2;
          x1 = firstOtherPoint->GetX();
          y1 = firstOtherPoint->GetY();
          x2 = aktPoint->GetX();
          y2 = aktPoint->GetY();
          sprintf(tmp, "ERROR: \tStair::ConvertLineToPoly(): SubRoom %d Room %d Anfangspunkt ungleich Endpunkt!!!\n"
                  "\t(%f, %f) != (%f, %f)\n", GetSubRoomID(), GetRoomID(), x1, y1, x2, y2);
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }

     if (orgPoly.size() != 4) {
          char tmp[CLENGTH];
          sprintf(tmp, "ERROR: \tStair::ConvertLineToPoly(): Stair %d Room %d ist kein Viereck!!!\n"
                  "Anzahl Ecken: %d\n", GetSubRoomID(), (int)GetRoomID(), (int)orgPoly.size());
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
     vector<Point> neuPoly = (orgPoly);
     // ganz kleine Treppen (nur eine Stufe) nicht
     if ((neuPoly[0] - neuPoly[1]).Norm() > 0.9 && (neuPoly[1] - neuPoly[2]).Norm() > 0.9) {
          for (int i1 = 0; i1 < (int) orgPoly.size(); i1++) {
               int i2 = (i1 + 1) % orgPoly.size();
               int i3 = (i2 + 1) % orgPoly.size();
               int i4 = (i3 + 1) % orgPoly.size();
               Point p1 = neuPoly[i1];
               Point p2 = neuPoly[i2];
               Point p3 = neuPoly[i3];
               Point p4 = neuPoly[i4];

               Point l1 = p2 - p1;
               Point l2 = p3 - p2;

               if (l1.Norm() < l2.Norm()) {
                    neuPoly[i2] = neuPoly[i2] + l1.Normalized() * 2 * J_EPS_GOAL;
                    l2 = p3 - p4;
                    neuPoly[i3] = neuPoly[i3] + l2.Normalized() * 2 * J_EPS_GOAL;
               }
          }
     }
     _poly = neuPoly;
}

bool Stair::IsInSubRoom(const Point& ped) const
{
     bool rueck = false;
     int N = (int) _poly.size();
     int sum = 0;

     for (int i = 0; i < N; i++) {
          Line l = Line(_poly[i], _poly[(i + 1) % N]);
          Point s = l.LotPoint(ped);
          if (l.IsInLineSegment(s))
               sum++;
     }
     if (sum == 4)
          rueck = true;

     return rueck;
}


void SubRoom::SetType(const std::string& type)
{
     _type = type;
}

const std::string& SubRoom::GetType() const
{
     return _type;
}

#ifdef _SIMULATOR

void SubRoom::SetAllPedestrians(const vector<Pedestrian*>& peds)
{
     _peds = peds;
}

void SubRoom::SetPedestrian(Pedestrian* ped, int index)
{
     if ((index >= 0) && (index < GetNumberOfPedestrians())) {
          _peds[index] = ped;
     } else {
          Log->Write("ERROR: Wrong Index in SubRoom::SetPedestrian()");
          exit(0);
     }
}

bool SubRoom::IsInSubRoom(Pedestrian* ped) const
{
     const Point& pos = ped->GetPos();
     if (ped->GetExitLine()->DistTo(pos) <= J_EPS_GOAL)
          return true;
     else
          return IsInSubRoom(pos);
}


int SubRoom::GetNumberOfPedestrians() const
{
     return _peds.size();
}

const vector<Pedestrian*>& SubRoom::GetAllPedestrians() const
{
     return _peds;
}

Pedestrian* SubRoom::GetPedestrian(int index) const
{
     if ((index >= 0) && (index < (int) GetNumberOfPedestrians()))
          return _peds[index];
     else {
          Log->Write("ERROR: Wrong 'index' in SubRoom::GetPedestrian()");
          exit(0);
     }
}


void SubRoom::AddPedestrian(Pedestrian* ped)
{
     _peds.push_back(ped);
}


void SubRoom::DeletePedestrian(int index)
{
     if ((index >= 0) && (index < (int) GetNumberOfPedestrians())) {
          _peds.erase(_peds.begin() + index);

     } else {
          Log->Write("ERROR: Wrong Index in SubRoom::DeletePedestrian()");
          exit(0);
     }
}


void SubRoom::ClearAllPedestrians()
{
     for(unsigned int p=0; p<_peds.size(); p++) {
          delete _peds[p];
     }
     _peds.clear();
}

#endif // _SIMULATOR
