/**
 * @file EmptyCognitiveMapCreator.cpp
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */

#include "EmptyCognitiveMapCreator.h"
#include "CognitiveMap.h"
#include <vector>
#include <map>
#include "../../geometry/Room.h"
#include "../../geometry/SubRoom.h"
#include "../../geometry/Building.h"
#include "../../geometry/Crossing.h"
#include "../../geometry/Transition.h"

EmptyCognitiveMapCreator::~EmptyCognitiveMapCreator()
{
     return;
}

CognitiveMap * EmptyCognitiveMapCreator::CreateCognitiveMap(const Pedestrian * ped)
{
     CognitiveMap * cm = new CognitiveMap(building, ped);

     return cm;
}
