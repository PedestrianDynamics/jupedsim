/**
 * \file        WalkingSpeed.h
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
#pragma once

#include <memory>
#include <string>

class Pedestrian;
class Building;
class Point;
class FDSMeshStorage;

class WalkingSpeed
{

public:

    explicit WalkingSpeed(const std::string & projectFilename);

    virtual ~WalkingSpeed();

    double WalkingInSmoke(const Pedestrian*, double);
    double GetExtinction(const Pedestrian *);

    void set_FMStorage(const std::shared_ptr<FDSMeshStorage>& fmStorage);
    const std::shared_ptr<FDSMeshStorage> get_FMStorage();


    bool LoadJPSfireInfo(const std::string & projectFilename);
    bool ReduceWalkingSpeed();
    double FrantzichNilsson2003(double &walking_speed, double ExtinctionCoefficient);
    double Jin1978(double &walking_speed, double ExtinctionCoefficient);
    double Fridolf2018(double &walking_speed, double ExtinctionCoefficient);

private:

    std::shared_ptr<FDSMeshStorage> _FMStorage;

};
