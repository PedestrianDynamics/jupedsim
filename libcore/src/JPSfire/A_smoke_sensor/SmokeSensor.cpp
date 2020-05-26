/**
 * \file        SmokeSensor.cpp
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
#include "SmokeSensor.h"

#include "JPSfire/generic/FDSMesh.h"
#include "JPSfire/generic/FDSMeshStorage.h"
#include "general/Filesystem.h"
#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "routing/smoke_router/NavigationGraph.h"
#include "routing/smoke_router/cognitiveMap/cognitivemap.h"

#include <Logger.h>
#include <tinyxml.h>

SmokeSensor::SmokeSensor(const Building * b) : AbstractSensor(b)
{
    _building = b;
    LoadJPSfireInfo();
}

SmokeSensor::~SmokeSensor() = default;

bool SmokeSensor::LoadJPSfireInfo()
{
    TiXmlDocument doc(_building->GetProjectFilename().string());
    if(!doc.LoadFile()) {
        LOG_ERROR("Could not parse project file: {}", doc.ErrorDesc());
        return false;
    }

    TiXmlNode * JPSfireNode = doc.RootElement()->FirstChild("JPSfire");
    if(!JPSfireNode) {
        LOG_INFO("Could not find any JPSfire information");
        return true;
    }

    TiXmlElement * JPSfireCompElem = JPSfireNode->FirstChildElement("A_smoke_sensor");
    if(JPSfireCompElem) {
        if(JPSfireCompElem->FirstAttribute()) {
            fs::path file_path(_building->GetProjectRootDir());
            file_path /= xmltoa(JPSfireCompElem->Attribute("smoke_factor_grids"), "");
            std::string filepath   = file_path.string();
            double updateIntervall = xmltof(JPSfireCompElem->Attribute("update_time"), 0.);
            double finalTime       = xmltof(JPSfireCompElem->Attribute("final_time"), 0.);
            LOG_INFO(
                "JPSfire Module A_smoke_sensor, tdata: {} tupdate time: {:.1f}s, final time: "
                "{:.1f}s",
                filepath.c_str(),
                updateIntervall,
                finalTime);
            _FMStorage = std::make_shared<FDSMeshStorage>(filepath, finalTime, updateIntervall);
            return true;
        }
    }
    return false;
}

std::string SmokeSensor::GetName() const
{
    return "SmokeSensor";
}

void SmokeSensor::execute(const Pedestrian * pedestrian, CognitiveMap & cognitive_map) const
{
    SubRoom * sub_room =
        building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
    GraphVertex * vertex =
        cognitive_map.GetGraphNetwork()->GetNavigationGraph()->operator[](sub_room);
    const GraphVertex::EdgesContainer * edges = vertex->GetAllEdges();
    /// for every egde connected to the pedestrian's current vertex (room)


    for(auto & item : *edges) {
        /// first: find Mesh corresponding to current edge and current simTime. Secondly get knotvalue from that mesh depending
        /// on the current position of the pedestrian

        double RiskTolerance = pedestrian->GetRiskTolerance();
        double weight        = 1;

        //FMStorage is nullptr if the section JPSfire is not parsed
        if(_FMStorage) {
            double SmokeFactor =
                _FMStorage
                    ->GetFDSMesh(
                        pedestrian->GetElevation(),
                        item->GetCrossing()->GetCentre(),
                        pedestrian->GetGlobalTime())
                    .GetKnotValue(pedestrian->GetPos()._x, pedestrian->GetPos()._y);
            weight = 1 + (1 - RiskTolerance) * SmokeFactor;
        }
        item->SetFactor(weight, GetName());
    }
}
