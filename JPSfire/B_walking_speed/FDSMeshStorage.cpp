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
#include <unistd.h>
#include <stdio.h>
#include <glob.h>
#include <string>

FDSMeshStorage::FDSMeshStorage()
{

}

FDSMeshStorage::FDSMeshStorage(string filepath, double finalTime, double updateIntervall, string study) :
    _filepath(filepath), _finalTime(finalTime),
    _updateIntervall(updateIntervall), _study(study),
    _elevationlist(), _timelist()
{
    CreateTimeList();
    CreateElevationList();
    CreateFDSMeshes();
}

FDSMeshStorage::~FDSMeshStorage()
{

}

void FDSMeshStorage::CreateTimeList()
{
    /// Create time list for mesh refreshment
    double i=0;
    while (i<=_finalTime)
    {
        _timelist.push_back(i);
        i+=_updateIntervall;
    }
}


void FDSMeshStorage::CreateElevationList()
{
    /// Create elevation list out of the available Z_* dirs in _filepath
    glob_t paths;
    int retval;

    paths.gl_pathc = 0;
    paths.gl_pathv = NULL;
    paths.gl_offs = 0;

    const char * glob_str = (_filepath + "Z_*").c_str();
    retval = glob( (glob_str) , GLOB_NOCHECK | GLOB_NOSORT,

                   NULL, &paths );
    if( retval == 0 ) {
        int idx;

        for( idx = 0; idx < paths.gl_pathc; idx++ ) {
            std::string glob_str = (paths.gl_pathv[idx]);
            double elev_dir = std::stod( glob_str.substr( glob_str.rfind("_") + 1 ));
            _elevationlist.push_back(elev_dir);
        }
        globfree( &paths );
    } else {
        puts( "glob() failed" );
    }
    //for(auto elem : _elevationlist)
        //std::cout << elem << std::endl;
}


void FDSMeshStorage::CreateFDSMeshes()
{
    _fMContainer.clear();
        for (auto &i:_elevationlist)
        {
            //std::cout << "i " << i << std::endl;
            for (auto &j:_timelist)
            {
                std::string suffix = "Z_" + std::to_string(i) + "/t_"+std::to_string(j) + ".csv";
                //std::cout << "j " << j << std::endl;
                std::string str = "Z_" + std::to_string(i) + "/t_"+std::to_string(j);
                //std::cout << _filepath+suffix << std::endl;
                FDSMesh mesh(_filepath+suffix);
                //std::string str = "t_"+std::to_string(i);
                _fMContainer.insert(std::make_pair(str, mesh));
            }
        }
}

const FDSMesh &FDSMeshStorage::get_FDSMesh(const double &simTime, const double &pedElev)
{
    //std::cout << "PASSED" << std::endl;
    int simT=simTime/_updateIntervall;
    simT*=_updateIntervall;
    double _PedEyeHeight = pedElev + 1.8;

    GetNearestHeight(_PedEyeHeight);

    if (simT>=_finalTime)
        simT=_finalTime;

    std::string str = "Z_" +  std::to_string(_NearestHeight) + "/t_"+std::to_string(simT)+".000000";

    //std::cout << str << std::endl;
    return _fMContainer.at(str);
}

double FDSMeshStorage::GetNearestHeight(double _PedEyeHeight)
{
    ///find the nearest height in the JPSfire data related to the ped elevation
    double min_val = std::numeric_limits<double>::max();
    int index = 0;

    for(int i=0;i < _elevationlist.size() ;++i) {
        double diff = std::abs(_elevationlist[i] - _PedEyeHeight);
        if(diff < min_val) {
            min_val = diff;
            index = i;
        }
    }
    _NearestHeight = _elevationlist[index];
    //std::cout << "NEAREST" << std::endl << _NearestHeight << std::endl;
    return _NearestHeight;
}

std::string FDSMeshStorage::GetStudy() const
{
    return _study;
}
