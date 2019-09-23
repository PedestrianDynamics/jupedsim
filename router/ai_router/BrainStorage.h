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

#include "Cortex.h"

#include "router/ai_router/cognitiveMap/cognitivemap.h"
#include "router/ai_router/perception/visibleenvironment.h"

#include <unordered_map>
#include <vector>

class Building;
class Pedestrian;
class InternNavigationNetwork;


typedef const Pedestrian * BStorageKeyType;
typedef std::unique_ptr<Cortex> BStorageValueType;
typedef std::unordered_map<BStorageKeyType, BStorageValueType> BStorageType;



/**
 * @brief Brain Storage
 *
 * Cares about corteces, creation and delivery
 *
 */
class AIBrainStorage {
public:
     AIBrainStorage(const Building * const b, const std::string& cogMapFiles="", const std::string& signFiles="");


     Cortex* operator[] (BStorageKeyType key);

     void DeleteCortex(BStorageKeyType ped);


private:
     const Building * const _building;
     BStorageType _corteces;


     //perception
     //Complete environment
     VisibleEnvironment _visibleEnv;

     //cognitive map
     std::vector<AIRegion> _regions;
     std::string _cogMapFiles;
     std::string _signFiles;

     //Cortex
     void CreateCortex(BStorageKeyType ped);
     void ParseCogMap(BStorageKeyType ped);
     void ParseSigns();


     // internal graph network in every room (for locomotion purposes)
     void InitInternalNetwork(const SubRoom *sub_room);
     std::unordered_map<const SubRoom*,ptrIntNetwork> _roominternalNetworks;

};
