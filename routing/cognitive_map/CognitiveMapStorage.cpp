/**
 * @file   CognitiveMapStorage.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   February, 2014
 * @brief  Cognitive Map Storage
 *
 */

#include "CognitiveMapStorage.h"


CognitiveMapStorage::CognitiveMapStorage(const Building * const b)
     : building(b)
{
}

CognitiveMapStorage::~CognitiveMapStorage()
{

}

CMStorageValueType CognitiveMapStorage::operator[] (CMStorageKeyType key)
{
     CMStorageType::iterator it = cognitive_maps.find(key);
     if(it == cognitive_maps.end()) {
          return CreateCognitiveMap(key);
     } else {
          return it->second;
     }
}

CMStorageValueType CognitiveMapStorage::CreateCognitiveMap(CMStorageKeyType ped)
{
}
