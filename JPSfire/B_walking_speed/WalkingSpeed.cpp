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
#include "../../tinyxml/tinyxml.h"
#include <set>

WalkingSpeed::WalkingSpeed(const Building * b)
{
    _FMStorage = nullptr;
    LoadJPSfireInfo(b->GetProjectFilename());
    Log->Write("INFO:\tInitialized FDSMeshStorage");
}

WalkingSpeed::~WalkingSpeed()
{
}



bool WalkingSpeed::LoadJPSfireInfo(const std::string &projectFilename )
{
   TiXmlDocument doc(projectFilename);
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
       std::string _study = xmltoa(JPSfireCompElem->Attribute("study"), "Jin1978");
       std::string _filepath = xmltoa(JPSfireCompElem->Attribute("extinction_grids"), "");
       double _updateIntervall = xmltof(JPSfireCompElem->Attribute("update_time"), 0.);
       double _finalTime = xmltof(JPSfireCompElem->Attribute("final_time"), 0.);
       Log->Write("INFO:\tModule B_walking_speed: study: %s \n\tdata: %s \n\tupdate time: %.1f final time: %.1f", _study.c_str(), _filepath.c_str(), _updateIntervall, _finalTime);
       _FMStorage = std::make_shared<FDSMeshStorage>(_filepath, _finalTime, _updateIntervall, _study);
       return true;
   }
   return false;
}


std::string WalkingSpeed::GetName() const
{
    return "WalkingSpeed";
}


double WalkingSpeed::GetExtinction(const Pedestrian * pedestrian)
{
    double ExtinctionCoefficient = _FMStorage->get_FDSMesh(pedestrian->GetGlobalTime(), pedestrian->GetElevation()).GetKnotValue(pedestrian->GetPos()._x , pedestrian->GetPos()._y);
    return ExtinctionCoefficient;
}

void WalkingSpeed::set_FMStorage(const std::shared_ptr<FDSMeshStorage> fmStorage)
{
    _FMStorage=fmStorage;
}

const std::shared_ptr<FDSMeshStorage> WalkingSpeed::get_FMStorage()
{
    return _FMStorage;
}

double WalkingSpeed::FrantzichNilsson2003(double &walking_speed, double ExtinctionCoefficient)
{
     //According to Frantzich+Nilsson2003
    walking_speed = -0.01192971*pow(ExtinctionCoefficient,3)+ 0.1621356*pow(ExtinctionCoefficient, 2)-0.75296314*ExtinctionCoefficient+1.6439047;
    return walking_speed;
}

double WalkingSpeed::Fridolf2013(double ExtinctionCoefficient, double &walking_speed)
{
    //According to Fridolf2013
    walking_speed = 1.19 - 0.14*ExtinctionCoefficient;
    return walking_speed;
}

double WalkingSpeed::Jin1978(double ExtinctionCoefficient, double &walking_speed)
{
    //According to Jin1978
    if(ExtinctionCoefficient > 5) {
       walking_speed = 0.2;
    }
    else {
    walking_speed = 0.96 - 0.30*ExtinctionCoefficient;
    }
    return walking_speed;
}

double WalkingSpeed::WalkingInSmoke(const Pedestrian* p, double &walking_speed)
{
    double ExtinctionCoefficient = GetExtinction(p);
    std::string study = _FMStorage->GetStudy();

    if((ExtinctionCoefficient < 10E-6) || (std::isnan(ExtinctionCoefficient)))   //no obstruction by smoke or NaN check
    {
        fprintf(stderr, "%f \t%f\n", ExtinctionCoefficient, p->GetEllipse().GetV0());
        return p->GetEllipse().GetV0();
    }
    else {
            if (study=="Frantzich+Nilsson2003"){
                FrantzichNilsson2003(walking_speed, ExtinctionCoefficient);
            }
            else if (study=="Fridolf2013"){
                Fridolf2013(ExtinctionCoefficient, walking_speed);
            }
            else if (study=="Jin1978"){
                Jin1978(ExtinctionCoefficient, walking_speed);
            }

    //Generally check if v0 < reduced walking_speed
    if(walking_speed > p->GetEllipse().GetV0()) {
       walking_speed = p->GetEllipse().GetV0();
    }
    }
   fprintf(stderr, "%f \t%f\n", ExtinctionCoefficient, walking_speed);
   return walking_speed;
}

bool WalkingSpeed::ReduceWalkingSpeed()
{
   return _FMStorage!=nullptr;
}
