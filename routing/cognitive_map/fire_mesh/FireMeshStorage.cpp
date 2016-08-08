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
#include "FireMeshStorage.h"

#include "../../../geometry/Building.h"

FireMeshStorage::FireMeshStorage()
{

}

FireMeshStorage::FireMeshStorage(const Building * const b, const std::string &filepath, const double &updateintervall, const double &finalTime)
{
    _building=b;
    _filepath=filepath;
    _updateIntervall=updateintervall;
    _finalTime=finalTime;

    CreateTimeList(updateintervall, finalTime);
    IdentifyDoors();
    CreateFireMeshes();
}

FireMeshStorage::~FireMeshStorage()
{

}

void FireMeshStorage::CreateTimeList(const double& updateIntervall, const double& finalTime)
{
    //fulfill list with times when new Mesh should be used
    double i=0;
    while (i<=finalTime)
    {
        _timelist.push_back(i);
        i+=updateIntervall;

    }
}

void FireMeshStorage::IdentifyDoors()
{

    const std::map<int,Crossing*> doors = _building->GetAllCrossings();

    for (auto it=doors.begin(); it!= doors.end(); ++it)
    {
        _doors.push_back(*it->second);
    }

    const std::map<int,Transition*> exits = _building->GetAllTransitions();

    for (auto it=exits.begin(); it!= exits.end(); ++it)
    {
        _doors.push_back(*it->second);
    }

}

void FireMeshStorage::CreateFireMeshes()
{
    _fMContainer.clear();
    for (auto &door:_doors)
    {
        for (auto &i:_timelist)
        {
            std::string suffix = "Door_X_" + std::to_string(door.GetCentre()._x)
                      + "_Y_" + std::to_string(door.GetCentre()._y) + "/t_"+std::to_string(i)+".csv";
            FireMesh mesh(_filepath+suffix);
            std::string str = "Door_X_"+ std::to_string(door.GetCentre()._x)
                    + "_Y_" + std::to_string(door.GetCentre()._y) + "_t_"+std::to_string(i);


            _fMContainer.insert(std::make_pair(str, mesh));

        }

    }

}

const FireMesh &FireMeshStorage::get_FireMesh(const Point &doorCentre, const double &simTime) const
{
    int simT;
    simT = (int) (simTime/_updateIntervall);
    simT*=_updateIntervall;

    if (simT>=_finalTime)
        simT= (int) _finalTime;

    std::string str = "Door_X_"+ std::to_string(doorCentre._x)
            + "_Y_" + std::to_string(doorCentre._y) + "_t_"+std::to_string(simT)+".000000";

    return _fMContainer.at(str);
}

