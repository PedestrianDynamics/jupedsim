/**
 * @file   CognitiveMapRouter.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   February, 2014
 * @brief  Routing Engine for Cognitive Map
 *
 */

#include "CognitiveMapRouter.h"
#include "Router.h"
#include "../geometry/Building.h"


CognitiveMapRouter::CognitiveMapRouter()
{
}

CognitiveMapRouter::~CognitiveMapRouter()
{
}

int CognitiveMapRouter::FindExit(Pedestrian * p)
{

}

void CognitiveMapRouter::Init(Building * b)
{
    building = b;

}
