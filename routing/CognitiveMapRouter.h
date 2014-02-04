/**
 * @file   CognitiveMapRouter.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   February, 2014
 * @brief  Routing Engine for Cognitive Map
 *
 */

#ifndef COGNITIVEMAPROUTER_H_
#define COGNITIVEMAPROUTER_H_

#include "Router.h"

class Building;
class Router;

/**
 * @brief Routing Engine for Cognitive Map
 *
 *
 */

class CognitiveMapRouter: public Router {
public:
    CognitiveMapRouter();
    virtual ~CognitiveMapRouter();

    virtual int FindExit(Pedestrian* p);
    virtual void Init(Building* b);

private:
    const Building * building;
};

#endif /* COGNITIVEMAPROUTER_H_ */
