/**
 * \file        JamSensor.h
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
 * detects jam in the next rooms
 *
 *
 **/
#pragma once

#include "AbstractSensor.h"

class JamSensor : public AbstractSensor
{

public:
    JamSensor(const Building * b) : AbstractSensor(b) { }

    virtual ~JamSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap& ) const;
private:

};
