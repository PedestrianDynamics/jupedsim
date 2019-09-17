/**
 * \file        WaitingArea.h
 * \date        Oct 15, 2018
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
 * Class providing inside goals which can be used to model intermediate goals
 * within the geometry. Additionally the pedestrians may wait for a certain
 * time or till a transition opens inside the waiting area.
 */
#pragma once

#include "Goal.h"

#include <cstdio>
#include <map>
#include <random>
#include <vector>

class Building;
class WaitingArea : public Goal {

protected:
    /**
     * Random device
     */
    std::random_device _rd;

    /**
     * Number of pedestrians which are allowed inside the waiting area
     */
    unsigned long _maxNumPed = -1;

    /**
     * Number of Pedestrians which is needed to start timer
     */
    unsigned long _minNumPed = -1;

    /**
     * Time after which the waiting area releases the pedestrians inside
     */
    int _waitingTime = -1;

    /**
     * Time at which the timer is started
     */
    double _startTime = -1;

    /**
     * ID of transition the waiting area is waiting for
     */
    int _transitionID = -1;

    /**
     * If true, _waitingTime is considered with respect to the start of the simulation
     */
    bool _globalTimer = false;

    /**
     * Defines whether pedestrians can enter the waiting area
     */
    bool _open;

    /**
     * Map of possible next goals/waiting areas (id) with corresponding probability ([0., 1.])
     */
    std::map<int, double> _nextGoals;

    /**
     * Map of possible next goals/waiting areas (id) with corresponding state
     */
    std::map<int, bool> _nextGoalsOpen;

    /**
     * Set of pedestrians who are currently in waiting area
     */
    std::set<int> _pedInside;

public:

    /**
     * Returns the max. number of peds allowed inside waiting area
     * @return max number of peds allowed inside waiting area
     */
    int GetMaxNumPed() const;

    /**
     * Sets the max. number of peds allowed inside waiting area
     * @param[in] maxNumPed max number of peds allowed inside waiting area
     */
    void SetMaxNumPed(int maxNumPed);

    /**
     * Returns the min. number of peds needed inside the waiting area to start the timer
     * @return min number of peds needed inside the waiting area to start the timer
     */
    int GetMinNumPed() const;

    /**
     * Sets the min. number of peds needed inside the waiting area to start the timer
     * @param[in] minNumPed min number of peds needed inside the waiting area to start the timer
     */
    void SetMinNumPed(int minNumPed);

    /**
     * Returns if waiting area is open for further peds
     * @return waiting area is open for further peds
     */
    bool IsOpen() const;

    /**
     * Sets the waiting area is open fur further peds
     * @param[in] open waiting area is open fur further peds
     */
    void SetOpen(bool open);

    /**
     * Returns if global timer is used for the waiting area
     * @return global timer is used for the waiting area
     */
    bool IsGlobalTimer() const;

    /**
     * Sets if a global timer is used for the waiting area
     * @param[in] timer global timer is used for the waiting area
     */
    void SetGlobalTimer(bool timer);

    /**
     * Returns the waiting time of the waiting area
     * @return waiting time of the waiting area
     */
    int GetWaitingTime() const;

    /**
     * Sets the waiting time of the waiting area
     * @param[in] waitingTime waiting time of the waiting area
     */
    void SetWaitingTime(int waitingTime);

    /**
     * Returns the ID of the next goal based on the probability. Only
     * considers open goals.
     * @return ID of the next goal or own ID if no open goal available
     */
    int GetNextGoal();

    /**
     * Adds ped to the pedestrians inside the waiting area
     * @param[in] ped ID of ped who entered waiting area
     */
    void AddPed(int ped);

    /**
     * Removes ped from the pedestrians inside the waiting area
     * @param[in] ped ID of ped who left waiting area
     */
    void RemovePed(int ped);

    /**
     * Starts the timer for the waiting time
     * @param[in] time the time the timer is started
     */
    void StartTimer(double time);

    /**
     * Returns the number of peds inside the waiting area
     * @return number of peds inside the waiting area
     */
    int GetNumPed();

    /**
     * Returns if the waiting area is waiting
     * @param[in] time time at which the waiting area is checkes
     * @param[in] building information of the surrounding
     * @return the waiting area is waiting
     */
    bool IsWaiting(double time, const Building* building);

    /**
     * Sets the state of waiting area (id) to isOpen
     * @param[in] isOpen further peds are allowed inside waiting area (id)
     * @param[in] id ID of the waiting area which state is changed
     */
    void UpdateProbabilities(bool isOpen, int id);

    /**
     * Returns the transition ID the waiting area is waiting for
     * @return ID of the transition the waiting area is waiting for
     */
    int GetTransitionID() const;

    /**
     * Sets the transition ID the waiting area is waiting for
     * @param[in] transitionID ID of the transition the waiting area is waiting for
     */
    void SetTransitionID(int transitionID);

    /**
     * Returns the map with the following waitings areas and their corresponding probabilities
     * @return map with the following waitings areas and their corresponding probabilities
     */
    const std::map<int, double>& GetNextGoals() const;

    /**
     * Sets the map with the following waitings areas and their corresponding probabilities
     * @param[in] nextGoals map with the following waitings areas and their corresponding probabilities
     */
    bool SetNextGoals(const std::map<int, double>& nextGoals);

    /**
     * Checks if probabilities combined are 100 percent (1.)
     * @return probabilities correctly set
     */
    bool CheckProbabilities();

    /**
     * Returns representation of waiting area as string
     * @return string representation of waiting area
     */
    std::string toString();

    const std::set<int>& GetPedInside() const;
};