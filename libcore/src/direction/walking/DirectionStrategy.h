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
 **/
#pragma once

#include "geometry/Point.h"

#include <map>
#include <string>
#include <vector>

class Room;

class Building;

class Pedestrian;

class Point;

class UnivFFviaFM;

class FloorfieldViaFM;

/**
 * Interface of a direction strategy.
 * A DirectionStrategy returns the desired direction of a pedestrian at a time-step.
 * In most cases it returns how a pedestrian crosses a line L = [P1, P2] (mostly doors).
 */
class DirectionStrategy
{
public:
    /**
     * Default constructor for DirectionStrategies.
     */
    DirectionStrategy() = default;

    /**
     * Default deconstructor for DirectionStrategies.
     */
    virtual ~DirectionStrategy() = default;

    /**
     * Initialization method for DirectionStrategies.
     */
    virtual void Init(Building *){};

    /**
     * Getter for the goal at the current time-step.
     * @param room Room \p ped is in
     * @param ped Pedestrian whos goal is determined
     * @return Goal at the current time-step of \p ped
     */
    virtual Point GetTarget(Room * room, Pedestrian * ped) const = 0;

    /**
     * Returns the distance to the wall of a pedestrian.
     * Only used by GradientModel.
     * @return the distance to the wall of a pedestrian
     * TODO restructuring (TS)
     */
    virtual double GetDistance2Wall(Pedestrian *) const { return -1.; };

    /**
     * Returns the distance to the target of a pedestrian.
     * Only used by FF Router.
     * @return the distance to the target of a pedestrian
     * TODO restructuring (TS)
     */
    virtual double GetDistance2Target(Pedestrian *, int) const { return -1.; };

    /**
     * Returns the direction of the closest wall of a pedestrian.
     * Only used by GradientModel.
     * @return the direction to the wall of a pedestrian
     * TODO restructuring (TS)
     */
    virtual Point GetDir2Wall(Pedestrian *) const { return Point(); };
};

/**
 * DirectionStrategy for exit_crossing_strategy 1
 *
 * The direction of the pedestrian is towards the middle of  L (P1 + P2)/2.
 */
class DirectionMiddlePoint : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

/**
 * DirectionStrategy for exit_crossing_strategy 2
 *
 * The direction is given by the nearest point on L to the position of the pedestrian. L is shorten by 20 cm.
 */
class DirectionMinSeperationShorterLine : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

/**
 * DirectionStrategy for exit_crossing_strategy 3
 *
 * If the nearest point of the pedestrian on the segment line L is outside the segment, then chose the middle point as target. Otherwise the nearest point is chosen.
 */
class DirectionInRangeBottleneck : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

/**
 * DirectionStrategy for exit_crossing_strategy 4 BETA!
 *
 * This strategy is still beta. It assumes that the simulation scenario has no loops or U-shaped corridors. Pedestrians, target he exit, even if it is outside their visibility range. In case of intersection with walls or obstacles, the temporary direction is rotated away from the wall.
 */
class DirectionGeneral : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};

/**
 * DirectionStrategy for exit_crossing_strategy 6
 *
 * Target is determinded by the underlying floorfield
 */
class DirectionFloorfield : public DirectionStrategy
{
public:
    DirectionFloorfield() : _ffviafm(nullptr){};

    void Init(Building * building) override;

    ~DirectionFloorfield() override;

    Point GetTarget(Room * room, Pedestrian * ped) const override;

    Point GetDir2Wall(Pedestrian * ped) const override;

    double GetDistance2Wall(Pedestrian * ped) const override;

private:
    FloorfieldViaFM * _ffviafm;
};

/**
 * DirectionStrategy for exit_crossing_strategy 8
 *
 * Target is determinded by the underlying floorfield
 */
class DirectionLocalFloorfield : public DirectionStrategy
{
public:
    void Init(Building * building) override;

    ~DirectionLocalFloorfield() override;

    Point GetTarget(Room * room, Pedestrian * ped) const override;

    Point GetDir2Wall(Pedestrian * ped) const override;

    double GetDistance2Wall(Pedestrian * ped) const override;

    double GetDistance2Target(Pedestrian * ped, int UID) const override;

protected:
    std::map<int, UnivFFviaFM *> _locffviafm;
    Building * _building;
    double _stepsize;
    double _wallAvoidDistance;
    bool _useDistancefield;
};

/**
 * DirectionStrategy for exit_crossing_strategy 9
 *
 * Target is determinded by the underlying floorfield
 */
class DirectionSubLocalFloorfield : public DirectionStrategy
{
public:
    void Init(Building * building) override;

    ~DirectionSubLocalFloorfield() override;

    Point GetTarget(Room * room, Pedestrian * ped) const override;

    Point GetDir2Wall(Pedestrian * ped) const override;

    double GetDistance2Wall(Pedestrian * ped) const override;

    double GetDistance2Target(Pedestrian * ped, int UID) const override;

protected:
    std::map<int, UnivFFviaFM *> _locffviafm;
    Building * _building;
    double _stepsize;
    double _wallAvoidDistance;
    bool _useDistancefield;
};

/**
 * DirectionStrategy for exit_crossing_strategy 12
 *
 * Target is determined by the closest train door
 */
class DirectionTrain : public DirectionStrategy
{
public:
    Point GetTarget(Room * room, Pedestrian * ped) const override;
};