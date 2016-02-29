/**
 * \file        FDSMeshStorage.h
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
 * Container to store all FDSMeshs. Sorted first by coordinates of the corresponding door,
 * secondly by simulation's global time
 *
 *
 **/
#include "FDSMeshStorage.h"
#include "../../IO/OutputHandler.h"
#include "../../pedestrian/PedDistributor.h"
#include "../../geometry/Building.h"

FDSMeshStorage::FDSMeshStorage()
{

}

FDSMeshStorage::FDSMeshStorage(const Building * const b, const std::string &fds_path, const double &update_time, const double &final_time)
{
    _building=b;
    _filepath=fds_path;
    _updateIntervall=update_time;
    _finalTime=final_time;

    CreateTimeList(update_time,final_time);
    //IdentifyDoors();
    CreateFDSMeshes();
}

FDSMeshStorage::~FDSMeshStorage()
{

}

void FDSMeshStorage::CreateTimeList(const double &update_time, const double &final_time, const double &startTime)
{
    //fulfill list with times when new Mesh should be used
    double i=0;
    while (i<=_finalTime)
    {
        _timelist.push_back(i);
        i+=_updateIntervall;
        {
        }
    }
}

//void FDSMeshStorage::IdentifyDoors()
//{

//    const std::map<int,Crossing*> doors = _building->GetAllCrossings();

//    for (auto it=doors.begin(); it!= doors.end(); ++it)
//    {
//        _doors.push_back(*it->second);
//    }

//    const std::map<int,Transition*> exits = _building->GetAllTransitions();

//    for (auto it=exits.begin(); it!= exits.end(); ++it)
//    {
//        _doors.push_back(*it->second);
//    }

//}


void FDSMeshStorage::CreateFDSMeshes()
{
    _fMContainer.clear();
        for (auto &i:_timelist)
        {
            std::string suffix = "t_"+std::to_string(i)+".csv";
            //std::cout << i << std::endl;
            std::string str = "t_"+std::to_string(i);
            //std::cout << str << std::endl;

            //std::cout << _filepath+suffix << std::endl;
            FDSMesh mesh(_filepath+suffix);

            //std::string str = "t_"+std::to_string(i);
            _fMContainer.insert(std::make_pair(str, mesh));

        }
}

const FDSMesh &FDSMeshStorage::get_FDSMesh(const double &simTime) const
{
    //std::cout << "PASSED" << std::endl;
    int simT=simTime/_updateIntervall;
    simT*=_updateIntervall;

    if (simT>=_finalTime)
        simT=_finalTime;

    std::string str = "t_"+std::to_string(simT)+".000000";

    return _fMContainer.at(str);
}

