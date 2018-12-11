//
// Created by Tobias Schrödter on 2018-12-04.
//

#ifndef JPSCORE_TRIPSROUTER_H
#define JPSCORE_TRIPSROUTER_H

#include "../Router.h"
#include "../../general/Macros.h"
#include "../../geometry/Building.h"

#include "../../pedestrian/Pedestrian.h"

class Building;
class Pedestrian;
class OutputHandler;

//log output
extern OutputHandler* Log;

class TripsRouter : public Router {

public:
     TripsRouter();
     TripsRouter(int id, RoutingStrategy s, Configuration* config);

     virtual ~TripsRouter();

    virtual bool Init(Building* building);

    virtual bool ReInit();

    virtual int FindExit(Pedestrian* p);

};

#endif //JPSCORE_TRIPSROUTER_H
