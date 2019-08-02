/**
 * \file        Obstacle.cpp
 * \date        Jul 31, 2012
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
#include "Obstacle.h"

#include "Line.h"
#include "Point.h"
#include "Wall.h"

#include "IO/OutputHandler.h"

#include <cstdlib>

Obstacle::Obstacle()
{
     _height=0.0;
     _id=-1;
     _caption="obstacle";
     _walls = std::vector<Wall > ();
     _poly = std::vector<Point > ();
}

Obstacle::~Obstacle() {}


void Obstacle::AddWall(const Wall& w)
{
     _walls.push_back(w);
}

std::string Obstacle::GetCaption() const
{
     return _caption;
}

void Obstacle::SetCaption(std::string caption)
{
     _caption = caption;
}

double Obstacle::GetHeight() const
{
     return _height;
}

void Obstacle::SetHeight(double height)
{
     _height = height;
}

int Obstacle::GetId() const
{
     return _id;
}

void Obstacle::SetId(int id)
{
     _id = id;
}

const std::vector<Point>& Obstacle::GetPolygon() const
{
     return _poly;
}

std::string Obstacle::Write()
{
     std::string s;

     for (unsigned int j = 0; j < _walls.size(); j++) {
          const Wall& w = _walls[j];
          s.append(w.Write());
          //pos = pos + w.GetPoint1() + w.GetPoint2();
     }
     //pos = pos * (0.5 / _walls.size());

     Point pos = GetCentroid();

     //add the obstacle caption
     char tmp[CLENGTH];
     //sprintf(tmp, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" text=\"%s\" color=\"100\" />\n"
     //              , pos.GetX() * FAKTOR, pos.GetY() * FAKTOR, _caption.c_str());

     sprintf(tmp, "\t\t<label centerX=\"%.2f\" centerY=\"%.2f\" centerZ=\"0\" text=\"%d\" color=\"100\" />\n"
             , pos._x * FAKTOR, pos._y * FAKTOR, _id);
     s.append(tmp);

     return s;
}

const std::vector<Wall>& Obstacle::GetAllWalls() const
{
     return _walls;
}

int Obstacle::WhichQuad(const Point& vertex, const Point& hitPos) const
{
     return (vertex._x > hitPos._x) ?
               ((vertex._y > hitPos._y) ? 1 : 4) :
               ((vertex._y > hitPos._y) ? 2 : 3);

//     if ((vertex.GetX() - hitPos.GetX())>J_EPS)
//     {
//          if ((vertex.GetY() - hitPos.GetY())>J_EPS)
//          {
//               return 1;
//          } else
//          {
//               return 4;
//          }
//     } else
//     {
//          if ((vertex.GetY() - hitPos.GetY())>J_EPS)
//          {
//               return 2;
//          } else
//          {
//               return 3;
//          }
//
//     }

}

// x-Koordinate der Linie von einer Eccke zur nächsten
double Obstacle::Xintercept(const Point& point1, const Point& point2, double hitY) const
{
     return (point2._x- (((point2._y - hitY) * (point1._x - point2._x)) /
                              (point1._y - point2._y)));
}


bool Obstacle::Contains(const Point& ped) const
{
     //case when the point is on an edge
     // todo: this affect the runtime, and do we really need that
     // If we do not d othis check, then for a square for instance, half the points located on the edge will be inside and
     // the other half will be outside the polygon.
     for(auto& w: _walls)
     {
          if(w.IsInLineSegment(ped)) return true;
     }
     // in the case the obstacle is not a close surface, allow
     // pedestrians distribution 'inside'

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


bool Obstacle::ConvertLineToPoly()
{
     std::vector<Line*> copy;
     std::vector<Point> tmpPoly;
     Point point;
     Line* line;
     // Alle Linienelemente in copy speichern
     for (auto& w: _walls)
       	 copy.push_back(&w);

     Point pIntsct(J_NAN, J_NAN);
     int itr = 1;
     for (auto& it : _walls) {
    	 int j = 0;
    	 for (unsigned int i = itr; i < copy.size(); ++i) {
    		 if (it.IntersectionWith(*copy[i], pIntsct) == true) {
    			 if (it.ShareCommonPointWith(*copy[i]) == false) {
    				 char tmp[CLENGTH];
    				 sprintf(tmp, "ERROR: \tObstacle::ConvertLineToPoly(): ID %d !!!\n", _id);
    				 Log->Write(tmp);
    				 sprintf(tmp, "ERROR: \tWalls %s & %s intersect: !!!\n", it.toString().c_str(),
    						                                           copy[i]->toString().c_str());
    				 Log->Write(tmp);
    				 return false;
    			 }
    			 else
    				 ++j;
    		 }
    	 }
         if (j <= 2)
        	 j = 0;
         else {
        	 char tmp[CLENGTH];
        	 sprintf(tmp, "ERROR: \tObstacle::ConvertLineToPoly(): ID %d !!!\n", _id);
        	 Log->Write(tmp);
        	 sprintf(tmp, "ERROR: \tWall %s shares edge with multiple walls!!!\n", it.toString().c_str());
        	 Log->Write(tmp);
        	 return false;
         }
    	 ++itr;
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
          sprintf(tmp, "ERROR: \tObstacle::ConvertLineToPoly(): ID %d !!!\n", _id);
          Log->Write(tmp);
          sprintf(tmp, "ERROR: \tDistance between the points: %lf !!!\n", (tmpPoly[0] - point).Norm());
          Log->Write(tmp);
          return false;
     }
     _poly = tmpPoly;

     //check if all walls and goals were used in the polygon
     for (const auto& w: _walls)
          for (const auto & ptw: {w.GetPoint1(),w.GetPoint2()})
               if(IsPartOfPolygon(ptw)==false) {
                    Log->Write("ERROR:\t Edge was not used during polygon creation for obstacle: %s",w.toString().c_str());
                    return false;
               }

     return true;
}

const Point Obstacle::GetCentroid() const
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

     return Point (px,py);
}

bool Obstacle::IsClockwise() const
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

bool Obstacle::IntersectWithLine(const Line& line) const
{

     for (unsigned int i=0; i<_walls.size(); i++) {

          if(_walls[i].IntersectionWith(line)) return true;
     }

     return false;
}

bool Obstacle::IsPartOfPolygon(const Point& ptw)
{
     if ( false == IsElementInVector(_poly, ptw))
     {
          //maybe the point was too closed to other points and got replaced
          //check that eventuality
          bool nah = false;
          for (const auto & pt : _poly)
          {
               if ((pt - ptw).Norm() < J_TOLERANZ)
               {
                    nah = true;
                    break;
               }
          }

          if(nah==false)
          {
               return false;
          }
     }
     return true;
}
