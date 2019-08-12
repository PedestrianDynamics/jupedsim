/**
 * \file        FDSMeshStorage.h
 * \date        Jan 1, 2017
 * \version     v0.8.2
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
#pragma once

#include "FDSMesh.h"

#include "geometry/Building.h"

#include <unordered_map>
#include <vector>

class Point;
class OutputHandler;
extern OutputHandler* Log;
// Container to store all FDSMeshs. Sorted by simulation's global time
using FDSMeshContainer = std::unordered_map<std::string, FDSMesh>;


class FDSMeshStorage
{
public:
    FDSMeshStorage();
    FDSMeshStorage(const std::string &filepath, const double &finalTime, const double &updateIntervall, const std::string &study="", const std::string &irritant="");
    ~FDSMeshStorage();
    bool CreateQuantityList();
    void CreateTimeList();
    bool CreateElevationList();
    void CreateDoorList();
    void CreateFDSMeshes();
     const FDSMesh& GetFDSMesh(const double &simTime, const double &pedElev,const std::string &quantity);
     const FDSMesh& GetFDSMesh(const double &pedElev, const Point &doorCentre, const double &simTime);
    std::string GetStudy() const;
    std::string IrritantOrNot() const;


private:
    FDSMeshContainer _fMContainer;
    std::string _filepath;
    double _finalTime;
    double _updateIntervall;
    std::string _study;
    std::vector<double> _elevationlist;
    std::vector<double> _timelist;
    std::string _irritant;
    std::vector<std::string> _quantitylist;
    std::vector<std::string> _doorlist;
    double _PedEyeHeight;
    double _NearestHeight;
    double GetNearestHeight(double);
    // const char * glob_str;

};
