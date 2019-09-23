/**
 * \file        Goal.cpp
 * \date        Spe 12, 2013
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
#include "Goal.h"

#include "Point.h"
#include "Wall.h"
#include "Crossing.h"
#include "../pedestrian/Pedestrian.h"

Goal::Goal()
{
     _id=-1;
     _caption="Goal";
     _isFinalGoal=0;
     _walls = std::vector<Wall > ();
     _poly = std::vector<Point > ();
     _crossing = new Crossing();
     _roomID = -1;
     _subRoomID = -1;
}

Goal::~Goal()
{

}

void Goal::AddWall(const Wall& w)
{
     _walls.push_back(w);
}

std::string Goal::GetCaption() const
{
     return _caption;
}

void Goal::SetCaption(std::string caption)
{
     _caption = caption;
}

int Goal::GetId() const
{
     return _id;
}

void Goal::SetId(int id)
{
     _id = id;
     _crossing->SetID(id);
}

const std::vector<Point>& Goal::GetPolygon() const
{
     return _poly;
}

std::string Goal::Write()
{
     std::string s;
     Point pos;

     for (unsigned int j = 0; j < _walls.size(); j++) {
          const Wall& w = _walls[j];
          s.append(w.Write());
          pos = pos + w.GetPoint1() + w.GetPoint2();
     }
     pos = pos * (0.5 / _walls.size());

     // add some fancy stuffs
     if(_poly.size()>=4) {
          s.append(Wall(_poly[0],_poly[2]).Write());
          s.append(Wall(_poly[1],_poly[3]).Write());
     }
     //add the Goal caption
     char tmp[CLENGTH];
     sprintf(tmp, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" text=\"%s\" color=\"100\" />\n"
             , pos._x * FAKTOR, pos._y * FAKTOR, _caption.c_str());
     s.append(tmp);

     return s;
}

const std::vector<Wall>& Goal::GetAllWalls() const
{
     return _walls;
}

int Goal::WhichQuad(const Point& vertex, const Point& hitPos) const
{
     return (vertex._x > hitPos._x) ? ((vertex._y > hitPos._y) ? 1 : 4) :
                 ((vertex._y > hitPos._y) ? 2 : 3);

}

int Goal::GetIsFinalGoal() const
{
     return _isFinalGoal;
}

void Goal::SetIsFinalGoal(int isFinalGoal)
{
     _isFinalGoal = isFinalGoal;
}

// x-Koordinate der Linie von einer Eccke zur nächsten
double Goal::Xintercept(const Point& point1, const Point& point2, double hitY) const
{
     return (point2._x - (((point2._y - hitY) * (point1._x - point2._x)) /
                              (point1._y - point2._y)));
}


bool Goal::Contains(const Point& ped) const
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

bool Goal::ConvertLineToPoly()
{
     std::vector<Line*> copy;
     std::vector<Point> tmpPoly;
     Point point;
     Line* line;
     // Alle Linienelemente in copy speichern
     for (unsigned int i = 0; i < _walls.size(); i++) {
          copy.push_back(&_walls[i]);
     }

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
          sprintf(tmp, "ERROR: \tGoal::ConvertLineToPoly(): ID %d !!!\n", _id);
          Log->Write(tmp);
          return false;
     }
     _poly = tmpPoly;
     CreateBoostPoly();
     ComputeCentroid();

     //compute dummy crossing in the middle
     Point point1, point2, tmp, diff, diff1, diff2;
     if (_poly.size() %2 == 0){
          point1 = _poly[0];
          tmp = _poly[_poly.size()/2];
          diff = point1 - tmp;

          point1 = tmp +  diff * 0.51;
          point2 = tmp +  diff * 0.49;

          _crossing->SetPoint1(point1);
          _crossing->SetPoint2(point2);
     }else{
          _crossing->SetPoint1(_poly[0]);
          Line tmp_line(_poly[_poly.size()/2], _poly[(_poly.size()/2)+1], 0);
          _crossing->SetPoint2(tmp_line.GetCentre());
     }



//     std::cout << "Crossing goal: " << _crossing->GetUniqueID() << _crossing->toString() << std::endl;
     return true;
}

const Point& Goal::GetCentroid() const
{
     return _centroid;
}

void  Goal::ComputeCentroid()
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
          x0 = _poly[i]._x;
          y0 = _poly[i]._y;
          x1 = _poly[i+1]._x;
          y1 = _poly[i+1]._y;
          a = x0*y1 - x1*y0;
          signedArea += a;
          px += (x0 + x1)*a;
          py += (y0 + y1)*a;
     }

     // Do last vertex
     x0 = _poly[i]._x;
     y0 = _poly[i]._y;
     x1 = _poly[0]._x;
     y1 = _poly[0]._y;
     a = x0*y1 - x1*y0;
     signedArea += a;
     px += (x0 + x1)*a;
     py += (y0 + y1)*a;

     signedArea *= 0.5;
     px /= (6*signedArea);
     py /= (6*signedArea);

     _centroid._x=px;
     _centroid._y=py;
}

Crossing* Goal::GetCentreCrossing()
{
     return _crossing;
}

bool Goal::IsInsideGoal(Pedestrian* ped) const
{
     if (_roomID == ped->GetRoomID() && _subRoomID == ped->GetSubRoomID()){
          return IsInsideGoal(ped->GetPos());
     }
     return false;
}

bool Goal::IsInsideGoal(const Point& point) const
{
     return boost::geometry::within(point, _boostPoly);
}

bool Goal::CreateBoostPoly() {
     std::vector<Point> copyPts;
     copyPts.insert(copyPts.begin(), _poly.begin(), _poly.end());

     if(!IsClockwise()){
          std::reverse(copyPts.begin(), copyPts.end());
     }

     boost::geometry::assign_points(_boostPoly, _poly);

     return true;
}

bool Goal::IsClockwise()
{
     //http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
     if(_poly.size()<3) {
          Log->Write("ERROR:\tYou need at least 3 vertices to check for orientation. Obstacle ID [%d]",_id);
          return false;
          //exit(EXIT_FAILURE);
     }
     double sum = 0;
     for (unsigned int i = 0; i < _poly.size() - 1; ++i) {
          Point a = _poly[i];
          Point b = _poly[i+1];
          sum += (b._x - a._x) * (b._y + a._y);
     }
     Point first = _poly[0];
     Point last = _poly[_poly.size()-1];
     sum += (first._x - last._x) * (first._y + last._y);

     return (sum > 0.);
}

int Goal::GetRoomID() const
{
     return _roomID;
}

void Goal::SetRoomID(int roomID)
{
     _roomID = roomID;
}

int Goal::GetSubRoomID() const
{
     return _subRoomID;
}

void Goal::SetSubRoomID(int subRoomID)
{
     _subRoomID = subRoomID;
}
