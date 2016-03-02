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
#include "../../tinyxml/tinyxml.h"
#include "../../pedestrian/PedDistributor.h"
#include "../../geometry/Building.h"

bool FDSMeshStorage::LoadJPSfireInfo()
{
   string _projectFilename = _building->GetProjectFilename();

   TiXmlDocument doc(_projectFilename);
   if (!doc.LoadFile()) {
        Log->Write("ERROR: \t%s", doc.ErrorDesc());
        Log->Write("ERROR: \t could not parse the project file");
        return false;
   }

   TiXmlNode* JPSfireNode = doc.RootElement()->FirstChild("JPSfire");
   if( ! JPSfireNode ) {
        Log->Write("INFO:\tcould not find any JPSfire information");
        return true;
   }

   Log->Write("INFO:\tLoading JPSfire info");

   TiXmlElement* JPSfireCompElem = JPSfireNode->FirstChildElement("B_walking_speed");
   if(JPSfireCompElem) {

       _study = xmltoa(JPSfireCompElem->Attribute("study"), "Frantzich+Nilsson2003");
       _filepath = xmltoa(JPSfireCompElem->Attribute("extinction_grids"), "");
       _updateIntervall = xmltof(JPSfireCompElem->Attribute("update_time"), 0);
       _finalTime = xmltof(JPSfireCompElem->Attribute("final_time"), 0);
       Log->Write("INFO:\tModule B_walking_speed: study: %s \n\tdata: %s \n\tupdate time: %.1f final time: %.1f", _study.c_str(), _filepath.c_str(), _updateIntervall, _finalTime);
       return true;
   }

}

FDSMeshStorage::FDSMeshStorage()
{

}

FDSMeshStorage::FDSMeshStorage(const Building * const b)
{
    _building=b;
    LoadJPSfireInfo();
    CreateTimeList();
    CreateFDSMeshes();
}

FDSMeshStorage::~FDSMeshStorage()
{

}

void FDSMeshStorage::CreateTimeList()
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

string FDSMeshStorage::GetStudy()
{
   return _study;
}

