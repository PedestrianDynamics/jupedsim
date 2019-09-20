/**
 * \file        GoalManager.h
 * \date        Feb 17, 2019
 * \version     v0.8
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * Class managing pedestrians who enter/leave waiting areas
 */
#pragma once

#include <map>

class Goal;
class Pedestrian;
class Building;

class GoalManager {

private:
    /**
     * Map containing goals which should be managed, key is the ID
     */
    std::map<int, Goal*> _allGoals;

    /**
     * Building which is observed by the GoalManager
     */
    Building* _building;

public:
    /**
     * Sets the goals which are managed
     * @param[in] goals map containing goals which should be managed
     */
    void SetGoals(const std::map<int, Goal*>& goals);

    /**
     * Checks whether a pedestrian has entered or left a goal/wa and
     * perform the corresponding actions
     * @param[in] ped pedestrian, which position is checked
     */
    void ProcessPedPosition(Pedestrian* ped);

    /**
     * Sets the building which should be managed by the GoalManager
     * @param[in] building building which should be managed
     */
    void SetBuilding(Building* building);

    /**
     * Processes the waiting area. The state of the waiting area will be set
     * according to the time and number of peds inside
     * @param time[in] global time of the simulation
     */
    void ProcessWaitingAreas(double time);

private:
    /**
     * Checks if pedestrian is inside a specific goal
     * @param[in] ped pedestrians, which position is checked
     * @param[in] goalID ID of the goal
     * @return ped is inside goalID
     */
    bool CheckInside(Pedestrian* ped, int goalID);

    /**
     * Checks if pedestrian is inside a specific waiting area
     * @param[in] ped pedestrians, which position is checked
     * @param[in] goalID ID of the waiting area
     * @return ped is inside goalID
     */
    bool CheckInsideWaitingArea(Pedestrian* ped, int goalID);

    /**
     * Sets the state of a specific goal and informs the other goals of changes
     * @param[in] goalID ID of the goal
     * @param[in] state open(true)/close(false)
     */
    void SetState(int goalID, bool state);
};
