/**
 * \file        WalkingSpeed.cpp
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
 * detects smoke in the next rooms
 *
 *
 **/

#include "WalkingSpeed.h"
//#include "../routing/cognitive_map/NavigationGraph.h"
#include "../../geometry/Building.h"
//#include "../routing/cognitive_map/cognitiveMap/cognitivemap.h"
#include "../../pedestrian/Pedestrian.h"
//#include "../../geometry/SubRoom.h"
#include "FDSMesh.h"
#include "FDSMeshStorage.h"
#include "../../pedestrian/PedDistributor.h"
#include <set>

//WalkingSpeed::WalkingSpeed(const Building *b, const std::string &filepath, const double &updateintervall, const double &finalTime)
WalkingSpeed::WalkingSpeed(const Building * b, const std::string &fds_path, const double &update_time, const double &final_time)
{

    _FMStorage = std::make_shared<FDSMeshStorage>(b,fds_path,update_time,final_time);
    Log->Write("INFO:\tInitialized FDSMeshStorage");

}

WalkingSpeed::~WalkingSpeed()
{
}

std::string WalkingSpeed::GetName() const
{
    return "WalkingSpeed";
}


double WalkingSpeed::GetOD(const Pedestrian * pedestrian, const Building * b, const std::string &fds_path, const double &update_time, const double &final_time)
{
    double OpticalDensity = _FMStorage->get_FDSMesh(pedestrian->GetGlobalTime()).GetKnotValue(pedestrian->GetPos().GetX(), pedestrian->GetPos().GetY());

    //std::cout << OpticalDensity << std::endl;
    return OpticalDensity;
}

void WalkingSpeed::set_FMStorage(const std::shared_ptr<FDSMeshStorage> fmStorage)
{
    _FMStorage=fmStorage;
}

const std::shared_ptr<FDSMeshStorage> WalkingSpeed::get_FMStorage()
{
    return _FMStorage;
}


