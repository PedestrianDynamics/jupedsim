/**
 * \file        CognitiveMapStorage.cpp
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
 * Cognitive Map Storage
 *
 *
 **/


#include "CognitiveMapStorage.h"
#include "AbstractCognitiveMapCreator.h"
#include "EmptyCognitiveMapCreator.h"
#include "CompleteCognitiveMapCreator.h"
#include "CognitiveMap.h"
#include "../../geometry/Building.h"
#include "NavigationGraph.h"


CognitiveMapStorage::CognitiveMapStorage(const Building * const b, std::string cogMapStatus)
     : building(b)
{
    if (cogMapStatus == "empty")
    creator = new EmptyCognitiveMapCreator(b);
    else
    creator = new CompleteCognitiveMapCreator(b);
}

CognitiveMapStorage::~CognitiveMapStorage()
{
     delete creator;
}

CMStorageValueType CognitiveMapStorage::operator[] (CMStorageKeyType key)
{
     CMStorageType::iterator it = cognitive_maps.find(key);
     if(it == cognitive_maps.end()) {
          CreateCognitiveMap(key);
     }

     return cognitive_maps[key];
}

void CognitiveMapStorage::CreateCognitiveMap(CMStorageKeyType ped)
{
     //todo: the possibility to have more then one creator.
     cognitive_maps.insert(std::make_pair(ped, creator->CreateCognitiveMap(ped)));
    

     //debug
     //cognitive_maps[ped]->GetNavigationGraph()->WriteToDotFile(building->GetProjectRootDir());
}
