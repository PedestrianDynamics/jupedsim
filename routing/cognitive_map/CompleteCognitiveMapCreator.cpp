/**
 * \file        CompleteCognitiveMapCreator.cpp
 * \date        Feb 1, 2014
 * \version     v0.6
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


#include "CompleteCognitiveMapCreator.h"
#include "CognitiveMap.h"
#include <vector>
#include <map>
#include "../../geometry/Room.h"
#include "../../geometry/SubRoom.h"
#include "../../geometry/Building.h"
#include "../../geometry/Crossing.h"
#include "../../geometry/Transition.h"

CompleteCognitiveMapCreator::~CompleteCognitiveMapCreator()
{
}

CognitiveMap * CompleteCognitiveMapCreator::CreateCognitiveMap(const Pedestrian * ped)
{
     CognitiveMap * cm = new CognitiveMap(_building, ped);

     //adding all SubRooms as Vertex
     for(auto&& itr_room: _building->GetAllRooms())
     {
          for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
          {
               cm->Add(itr_subroom.second.get());
          }
     }

     //Add crossings as edges
     for(auto&& itr_cross: _building->GetAllCrossings())
     {
          cm->Add(itr_cross.second);
     }

     //Add transitions as edges
     for(auto&& itr_trans: _building->GetAllTransitions())
     {
          if(itr_trans.second->IsExit())
          {
               cm->AddExit(itr_trans.second);
          }
          else
          {
               cm->Add(itr_trans.second);
          }
     }
     return cm;
}
