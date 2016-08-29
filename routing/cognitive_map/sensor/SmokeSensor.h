/**
 * \file        SmokeSensor.h
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

#ifndef SMOKESENSOR_H
#define SMOKESENSOR_H 1

#include "AbstractSensor.h"
#include <memory>

class Point;
class FireMeshStorage;

class SmokeSensor : public AbstractSensor
{

public:
    SmokeSensor(const Building * b, const std::string &filepath, const double &updateintervall, const double &finalTime);

    virtual ~SmokeSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap *) const;

    void set_FMStorage(const std::shared_ptr<FireMeshStorage> fmStorage);
    const std::shared_ptr<FireMeshStorage> get_FMStorage();


private:

    std::shared_ptr<FireMeshStorage> _FMStorage;

};

#endif // SMOKESENSOR_H
