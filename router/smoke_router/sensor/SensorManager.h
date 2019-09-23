/**
 * \file        SensorManager.h
 * \date        Jan 1, 2014
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
 * The Sensor Manager cares about Sensor Registration and Execution.
 *
 *
 **/
#pragma once

#include<unordered_map>
#include<vector>
#include <set>
#include <string>

class Building;
class BrainStorage;
class AbstractSensor;
class Pedestrian;

//c++11 alias: Container to store options for the router (i. a. sensors)
using optStorage = std::unordered_map<std::string,std::vector<std::string> >;

class SensorManager {
public:
    typedef int EventType;
    static const EventType NONE = 0;
    static const EventType INIT = 1;
    static const EventType PERIODIC = 2;
    static const EventType NO_WAY = 4;
    static const EventType CHANGED_ROOM = 8;
    static const EventType NEW_DESTINATION = 16;


     typedef std::vector<std::pair<AbstractSensor *, EventType> > SensorContainer;

     /****************************
      * Constructors & Destructors
      ****************************/
     SensorManager(const Building * building, BrainStorage *);
     virtual ~SensorManager();

     void Register(AbstractSensor *, EventType);
     void execute(const Pedestrian *, EventType);

     static SensorManager * InitWithAllSensors(const Building *, BrainStorage *);
     static SensorManager * InitWithCertainSensors(const Building*, BrainStorage *, const optStorage& optSto);

     void SetIntVPeriodicUpdate(const double& interval);
     const double& GetIntVPeriodicUpdate();

private:
     //const Building * const building; //@todo: unused.
     BrainStorage * cm_storage;

     SensorContainer registered_sensors;

     double _periodicUpdateInterval;
};
