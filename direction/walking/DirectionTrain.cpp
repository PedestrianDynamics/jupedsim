/**
 * \file        DirectionTrain.cpp
 * \copyright   <2009-2025> Forschungszentrum Jülich GmbH. All rights reserved.
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
 **/

#include "DirectionTrain.h"
#include "geometry/Line.h"
#include "geometry/NavLine.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/SubRoom.h"

Point DirectionTrain::GetTarget(Room* /*room*/, Pedestrian* ped) const{
     Point p1 = ped->GetExitLine()->GetPoint1();
     Point p2 = ped->GetExitLine()->GetPoint2();
     Line ExitLine = Line(p1, p2, 0);
     auto TrainTypes = ped->GetBuilding()->GetTrainTypes();
     auto TrainTimeTables = ped->GetBuilding()->GetTrainTimeTables();
     auto now = ped->GetGlobalTime();
     std::string type_delme="";
     // std::cout << ">>> Enter with ped at " << ped->GetPos().toString().c_str() << "\n";
     for(auto && t: TrainTimeTables)
     {
          if(ped->GetRoomID() != t.second->rid) continue;

          if( (now>=t.second->tin) && (now<=t.second->tout) )
          {
               auto doors = TrainTypes[t.second->type]->doors;
               int i=-1, imin=0;
               double dist_min = 10000;
               for(auto door: doors)
               {
                    i++;
                    const Point & d1 = door.GetPoint1();
                    const Point & d2 = door.GetPoint2();
                    const Point & c = (d1+d2)*0.5;

                    double dist = (ped->GetPos()-c).Norm();
                    // std::cout << "door id: " << door.GetID()<< " dist: " << dist<< "\n";

                    if(dist <= dist_min)
                    {
                         dist_min = dist;
                         imin=i;
                         type_delme=t.second->type;
                         // std::cout << "    > imin " << imin << "  mindist " << dist_min << "\n";
                    }
               }// doors
               p1  = doors[imin].GetPoint1();
               p2 = doors[imin].GetPoint2();
               // std::cout << "\n>>> train: now " << now << ", type: " << type_delme.c_str() << "\n";
               // std::cout << ">>> p1=" << p1.toString().c_str() << ". p2=" << p2.toString().c_str()<< "\n";
               // std::cout << ">>> ped at " << ped->GetPos().toString().c_str() << "\n";
               // getc(stdin);

          }// if time in
     }


     return (p1+ p2)*0.5;
}
