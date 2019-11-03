/**
 * \file        DirectionStrategy.h
 * \date        Dec 13, 2010
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
 *
 * \section Description
 *
 *
 **/
#pragma once

#include <map>
#include <string>
#include <vector>

class Room;

class Building;

class Pedestrian;

class Point;

class UnivFFviaFM;

class FloorfieldViaFM;

class LocalFloorfieldViaFM;

class SubLocalFloorfieldViaFM;

class DirectionStrategy
{
public:
    DirectionStrategy();

    virtual ~DirectionStrategy();

    virtual void Init(Building *){};

    virtual Point GetTarget(Room * room, Pedestrian * ped) const = 0;

    virtual double GetDistance2Wall(Pedestrian * ped) const;

    virtual double GetDistance2Target(Pedestrian * ped, int UID);
};

class DirectionMiddlePoint : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

class DirectionMinSeperation : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

class DirectionTrain : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

class DirectionMinSeperationShorterLine : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

class DirectionInRangeBottleneck : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

class DirectionGeneral : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

class DirectionFloorfield : public DirectionStrategy
{
public:
    DirectionFloorfield();

    void Init(Building * building) override;

    ~DirectionFloorfield();

    //void Init();
    Point GetTarget(Room * room, Pedestrian * ped) const override;

    Point GetDir2Wall(Pedestrian * ped) const;

    double GetDistance2Wall(Pedestrian * ped) const override;

private:
    FloorfieldViaFM * _ffviafm;
    bool _initDone;
};

class DirectionLocalFloorfield : public DirectionStrategy
{
public:
    DirectionLocalFloorfield();

    void Init(Building * building) override;

    ~DirectionLocalFloorfield();

    Point GetTarget(Room * room, Pedestrian * ped) const override;

    Point GetDir2Wall(Pedestrian * ped) const;

    double GetDistance2Wall(Pedestrian * ped) const override;

    double GetDistance2Target(Pedestrian * ped, int UID) override;

protected:
    std::map<int, UnivFFviaFM *> _locffviafm;
    bool _initDone;
    Building * _building;
    double _stepsize;
    double _wallAvoidDistance;
    bool _useDistancefield;
    std::string _filename;
};

class DirectionSubLocalFloorfield : public DirectionStrategy
{
public:
    DirectionSubLocalFloorfield();

    void Init(Building * building) override;

    ~DirectionSubLocalFloorfield();

    Point GetTarget(Room * room, Pedestrian * ped) const override;

    virtual Point GetDir2Wall(Pedestrian * ped) const;

    double GetDistance2Wall(Pedestrian * ped) const override;

    double GetDistance2Target(Pedestrian * ped, int UID) override;

protected:
    std::map<int, UnivFFviaFM *> _locffviafm;
    bool _initDone;
    Building * _building;
    double _stepsize;
    double _wallAvoidDistance;
    bool _useDistancefield;
    std::string _filename;
};
