/**
 * \file        Crossing.h
 * \date        Nov 16, 2010
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
 *
 *
 **/
#pragma once

#include "Hline.h"

class SubRoom;

class Crossing : public Hline
{
private:
    //     /// TODO ? unique between crossings and transitions ?
    /**
     * Total door usage, counts all pedestrians passing the crossing.
     */
    int _doorUsage;

    /**
     * Temporary door usage, counts pedestrians until reset.
     */
    int _tempDoorUsage;

    /**
     * Last time a pedestrian passed this door.
     */
    double _lastPassingTime;

    /**
     * Allowed outflow rate in peds/second.
     */
    double _outflowRate;

    /**
     * Maximum number of pedestrians allowed to pass that door.
     */
    int _maxDoorUsage;

    /**
     * Time to wait until is reopened by flow regulation.
     */
    double _closingTime;

    /**
     * Flow observation time.
     */
    double _DT;

    /**
     * Number of pedestrians that pass the line to trigger measurement of the flow.
     */
    int _DN;

    /**
     * Number of pedestrians passing the door in \a _DT.
     */
    int _partialDoorUsage;

    /**
     * Time the last flow measurement was conducted/
     */
    double _lastFlowMeasurement;

    /**
     * Timestamp and number of pedestrians who have passed the door.
     */
    std::string _flowAtExit;

    /**
     * Current state of the door.
     */
    DoorState _state;

    /**
     * The closing operations was conducted by an event which always has priority.
     */
    bool _closeByEvent = false;

public:
    /**
     * Constructor
     */
    Crossing();

    /**
     * Destructor
     */
    ~Crossing() override = default;

    /**
     * Close the door.
     * @param event An event lead to the closing.
     */
    void Close(bool event = false);

    /**
     * Temporarily close the door.
     * @param event An event lead to the closing.
     */
    void TempClose(bool event = false);

    /**
     * Open the door.
     * @param event An event lead to the opening.
     */
    void Open(bool event = false);

    /**
     * Checks whether it is associated with the SubRoom with \p subroomID.
     * @return true if SubRoom with \p subroomID is associated with the current crossing
     */
    [[nodiscard]] bool IsInSubRoom(int subroomID) const override;

    /**
     * Checks whether the door is open.
     * @return true if \a _state == OPEN
     */
    [[nodiscard]] virtual bool IsOpen() const;

    /**
     * Checks whether the door is temp_close.
     * @return true if \a _state == TEMP_CLOSE
     */
    [[nodiscard]] virtual bool IsTempClose() const;

    /**
     * Checks whether the door is close.
     * @return true if \a _state == CLOSE
     */
    [[nodiscard]] virtual bool IsClose() const;

    /**
      * @return true if the crossing is an exit/transition. (Transitions are derived from this class)
      * @see Transition
      */
    [[nodiscard]] virtual bool IsExit() const;

    /**
     * Checks if door is a transition.
     * @return true if it is a transition
     */
    [[nodiscard]] virtual bool IsTransition() const;

    /**
     * Returns the second SubRoom the door is connected to, which is not matching \p roomID and \p subroomID.
     * @param roomID ID of one of the rooms the door is connecting.
     * @param subroomID ID of one of the rooms the door is connecting.
     * @return the other subroom not matching the data
     */
    [[nodiscard]] virtual SubRoom * GetOtherSubRoom(int roomID, int subroomID) const;

    /**
     * Dump to error log.
     */
    virtual void WriteToErrorLog() const;

    /**
     * Gets a description of the door.
     * @return a nicely formatted string representation of the object
     */
    [[nodiscard]] std::string GetDescription() const override;

    /**
     * Returns the number of common subrooms with door \p other.
     * @param[in] other the other door.
     * @param[out] subroom returns one of the common subrooms.
     * @return the number of subrooms this has in common with  \p other (0, 1, 2)
     */
    virtual int CommonSubroomWith(Crossing * other, SubRoom *& subroom);

    /**
     * Increment the number of persons that used that crossing.
     * @param number, how many person have passed the crossing.
     * @param time, at which time persons have passed.
     */
    void IncreaseDoorUsage(int number, double time);

    /**
     * Increment the number of persons that used that crossing in time span \a _DT.
     * @param number, how many person have passed the crossing.
     * @param time, at which time they have passed.
     */
    void IncreasePartialDoorUsage(int number);

    /**
     * Resets number which have passed the door \a _DT to 0.
     */
    void ResetPartialDoorUsage();

    /**
     * Returns the number of pedestrians that have passed the door in time span \a _DT.
     * @return the number of pedestrians that have passed the door in time span \a _DT.
     */
    [[nodiscard]] int GetPartialDoorUsage() const;

    /**
     * Resets the number of pedestrians that used that door (\a _tempDoorUsage) to 0.
     */
    void ResetDoorUsage();

    /**
     * Returns the total number of pedestrians that have passed that door.
     * @return the total number of pedestrians that have passed that door.
     */
    [[nodiscard]] int GetDoorUsage() const;

    /**
     * Returns the outflow rate of the door.
     * @return outflow rate of the door.
     */
    [[nodiscard]] double GetOutflowRate() const;

    /**
     * Returns the maximum number of pedestrians which are allowed to pass that door.
     * @return max of number of agents that can pass the door.
     */
    [[nodiscard]] int GetMaxDoorUsage() const;

    /**
     * Sets the maximum number of pedestrians which are allowed to pass that door.
     * @param mdu maximum number of pedestrians which are allowed to pass that door.
     */
    void SetMaxDoorUsage(int mdu);

    /**
     * Sets the outflow rate of the door.
     * @param outflow allowed outflow of the door in peds\second.
     */
    void SetOutflowRate(double outflow);

    /**
     * Returns the time stamps and number of pedestrians passing the door.
     * @return the time stamps and number of pedestrians passing the door.
     */
    [[nodiscard]] const std::string & GetFlowCurve() const;

    /**
     * Returns the last time a ped used the door.
     * @return the last time a ped used the door.
     */
    [[nodiscard]] double GetLastPassingTime() const;

    /**
     * Returns the time to wait until the door is reopened by flow regulations again.
     * @return the time to wait until the door is reopened.
     */
    [[nodiscard]] double GetClosingTime() const;

    /**
     * Updates the closing time to \p dt.
     * @param dt time to decrease the waiting time until door is reopened.
     */
    void UpdateClosingTime(double dt);

    /**
     * Updates the state of temp closed doors. Door will be opened if it is
     * closed due to flow regulation if maxDoorUsage is not reached yet.
     *
     * @param[in] dt current time.
     */
    void UpdateTemporaryState(double dt);

    /**
     * Returns the time span until the flow regulation is triggered.
     * @return the time span until the flow regulation is triggered.
     */
    double GetDT();

    /**
     * Sets the flow observation time.
     * @param dt time span \a _DT in which the flow is observed.
     */
    void SetDT(double dt);

    /**
     * Returns the number of pedestrians that pass the line to trigger measurement of the flow.
     * @return number of pedestrians that pass the line to trigger measurement of the flow.
     */
    int GetDN();

    /**
     * Sets the number of pedestrians that pass the line to trigger measurement of the flow.
     * @param dn number of pedestrians that pass the line to trigger measurement of the flow.
     */
    void SetDN(int dn);

    /**
     * Regulates the flow at a crossing. To assure a specific flow at a door the door may need to closes
     * temporarily.
     *
     * If _outFlowRate is set by user:
     *  - If the current flow between the last measurement and now is greater than _outflowRate
     *      - change door state to temp_close
     *
     * If _maxDoorUsage is set by user:
     *  - If _doorUsage is greater equal than _maxDoorUsage
     *      - change door state to close
     *
     * @post changes \a _lastFlowMeasurement, \a _closingTime, \a _state
     * @param time time the flow is regulated
     * @return true, if a change to door state was made.
     */
    bool RegulateFlow(double time);

    /**
     * Returns the state of the door.
     * @return state of the door.
     */
    [[nodiscard]] DoorState GetState() const;

    /**
     * Sets the state of the door.
     * @param state state the door should be set to.
     */
    void SetState(DoorState state);

    /**
     * Returns a std::string representation of the door.
     * @return formatted string representing the door
     */
    [[nodiscard]] std::string toString() const override;
};
