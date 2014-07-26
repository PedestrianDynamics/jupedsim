/**
 * \file        CognitiveMapRouter.h
 * \date        Feb 1, 2014
 * \version     v0.5
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
 * Routing Engine for Cognitive Map
 *
 *
 **/


#ifndef COGNITIVEMAPROUTER_H_
#define COGNITIVEMAPROUTER_H_

#include "Router.h"
#include <string>

class Building;
class Router;
class CognitiveMapStorage;
class SensorManager;


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

protected:
     void CheckAndInitPedestrian(Pedestrian *);
private:

     Building * building;
     CognitiveMapStorage * cm_storage;
     SensorManager * sensor_manager;

};

#endif /* COGNITIVEMAPROUTER_H_ */
