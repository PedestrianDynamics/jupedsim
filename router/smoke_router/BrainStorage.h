/**
 * \file        CognitiveMapStorage.h
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
 * Cognitive Map Storage
 *
 *
 **/
#pragma once

#include "Brain.h"

#include "router/smoke_router/cognitiveMap/cognitivemap.h"

#include <unordered_map>
#include <vector>

class Building;
class Pedestrian;
class InternNavigationNetwork;


typedef const Pedestrian * BStorageKeyType;
typedef std::shared_ptr<Brain> BStorageValueType;
typedef std::unordered_map<BStorageKeyType, BStorageValueType> BStorageType;



/**
 * @brief Brain Storage
 *
 * Cares about Cognitive map storage, creation and delivery
 *
 */
class BrainStorage {
public:
     BrainStorage(const Building * const b, std::string cogMapStatus, std::string cogMapFiles="");
     virtual ~BrainStorage();


     BStorageValueType operator[] (BStorageKeyType key);


private:
     const Building * const _building;
     BStorageType _brains;


     //cognitive map
     std::vector<ptrRegion> _regions;
     std::string _cogMapStatus;
     std::string _cogMapFiles;

     //brain
     void CreateBrain(BStorageKeyType ped);
     void ParseCogMap(BStorageKeyType ped);


     // internal graph network in every room (for locomotion purposes)
     void InitInternalNetwork(const SubRoom *sub_room);
     std::unordered_map<const SubRoom*,ptrIntNetwork> _roominternalNetworks;

};
