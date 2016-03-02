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

#ifndef FDSMeshSTORAGE_H
#define FDSMeshSTORAGE_H

#include <unordered_map>
#include <vector>
#include "FDSMesh.h"


class Building;
class Point;
class Crossing;

// Container to store all FDSMeshs. Sorted first by coordinates of the corresponding door, secondly by simulation's global time
using FDSMeshContainer = std::unordered_map<std::string, FDSMesh>;

class FDSMeshStorage
{
public:
    FDSMeshStorage();
    FDSMeshStorage(const Building * const b);
    ~FDSMeshStorage();
    void CreateTimeList();
    void CreateFDSMeshes();
    const FDSMesh& get_FDSMesh(const double &simTime) const;
    bool LoadJPSfireInfo();
    std::string GetStudy();

private:
    const Building *_building;
    FDSMeshContainer _fMContainer;
    std::string _filepath;
    std::string _study;
    double _updateIntervall;
    double _finalTime;
    std::vector<double> _timelist;
    std::vector<Crossing> _doors;
};

#endif // FDSMeshSTORAGE_H
