/**
 * \file        DirectionLocalFloorfield.cpp
 * \copyright   <2009-2025> Forschungszentrum Jülich GmbH. All rights reserved.
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
 **/

#pragma  once

#include "DirectionStrategy.h"

class Building;
class UnivFFviaFM;

class DirectionLocalFloorfield : public DirectionStrategy {
public:
    DirectionLocalFloorfield();
    ~DirectionLocalFloorfield();

    virtual void Init(Building* building);
    virtual Point GetTarget(Room* room, Pedestrian* ped) const;
    virtual Point GetDir2Wall(Pedestrian* ped) const;
    virtual double GetDistance2Wall(Pedestrian* ped) const;
    virtual double GetDistance2Target(Pedestrian* ped, int UID);

protected:
    std::map<int, UnivFFviaFM*> _locffviafm;
    bool _initDone;
    Building* _building;
    double _stepsize;
    double _wallAvoidDistance;
    bool _useDistancefield;
    std::string _filename;
};