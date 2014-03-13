/**
 * @file   CognitiveMapStorage.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   February, 2014
 * @brief  Cognitive Map Storage
 *
 */

#include "CognitiveMapStorage.h"
#include "AbstractCognitiveMapCreator.h"
#include "EmptyCognitiveMapCreator.h"
#include "CompleteCognitiveMapCreator.h"
#include "CognitiveMap.h"
#include "../../geometry/Building.h"
#include "NavigationGraph.h"


CognitiveMapStorage::CognitiveMapStorage(const Building * const b)
     : building(b)
{
    creator = new EmptyCognitiveMapCreator(b);
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
    cognitive_maps.emplace(ped, creator->CreateCognitiveMap(ped));

    //debug
    //cognitive_maps[ped]->GetNavigationGraph()->WriteToDotFile(building->GetProjectRootDir());
}
