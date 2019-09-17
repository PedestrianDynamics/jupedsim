/**
 * \file        SmokeRouter.h
 * \date        Feb 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
#pragma once

#include "general/Filesystem.h"
#include "router/Router.h"

#include <string>
#include <unordered_map>
#include <memory>

class Building;
class Router;
class BrainStorage;
class SensorManager;
class NavLine;

//c++11 alias: Container to store options for the router (i. a. sensors)
using optStorage = std::unordered_map<std::string,std::vector<std::string> >;
/**
 * @brief Smoke router
 *\ingroup Router
 *
 */
class SmokeRouter: public Router {
public:
     SmokeRouter();
     SmokeRouter(int id, RoutingStrategy s);
     virtual ~SmokeRouter();


     virtual int FindExit(Pedestrian* p);
     virtual bool Init(Building* b);

     /**
      * @return options involved in the routing algorithm
      */
     const optStorage &getOptions() const;

     /**
      * Adds further options (key,value) to the optionContainer
      */
     void addOption(const std::string &key, const std::vector<std::string> &value);

     /**
      * Load extra routing information e.g navigation lines
      */
     bool LoadRoutingInfos(const fs::path &filename);

     /**
      * Each router is responsible of getting the correct filename
      * and doing other initializations
      */
     virtual fs::path GetRoutingInfoFile();

protected:

    int FindDestination(Pedestrian * );

private:

     Building * building;
     std::shared_ptr<BrainStorage>  brain_storage;
     SensorManager * sensor_manager;

     // Optional options which are supposed to be used
     optStorage options;

};
