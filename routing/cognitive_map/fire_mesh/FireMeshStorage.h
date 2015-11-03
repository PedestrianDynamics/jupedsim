/**
 * \file        FireMeshStorage.h
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
 * Container to store all fireMeshs. Sorted first by coordinates of the corresponding door,
 * secondly by simulation's global time
 *
 *
 **/

#ifndef FIREMESHSTORAGE_H
#define FIREMESHSTORAGE_H

#include <unordered_map>
#include <vector>
#include "FireMesh.h"


class Building;
class Point;
class Crossing;

// Container to store all fireMeshs. Sorted first by coordinates of the corresponding door, secondly by simulation's global time
using fireMeshContainer = std::unordered_map<std::string, FireMesh>;

class FireMeshStorage
{
public:
    FireMeshStorage();
    FireMeshStorage(const Building * const b, const std::string &filepath, const double &updateintervall, const double &finalTime);
    ~FireMeshStorage();
    void CreateTimeList(const double &updateIntervall, const double &finalTime, const double &startTime=0);
    void IdentifyDoors();
    void CreateFireMeshes();
    const FireMesh& get_FireMesh(const Point &doorCentre, const double &simTime) const;

private:
    const Building *_building;
    fireMeshContainer _fMContainer;
    std::string _filepath;
    double _updateIntervall;
    double _finalTime;
    std::vector<double> _timelist;
    std::vector<Crossing> _doors;


};

#endif // FIREMESHSTORAGE_H
