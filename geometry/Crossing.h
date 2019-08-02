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


#ifndef _CROSSING_H
#define _CROSSING_H

#include "NavLine.h"
#include "Hline.h"

//class Line;
class Room;
class SubRoom;

class Crossing : public Hline {
private:
//     /// TODO ? unique between crossings and transitions ?
    int _doorUsage;
    double _lastPassingTime;
    double _outflowRate; // pedestrians / second
    int _maxDoorUsage;
    double _closingTime; // time to wait until door is reopened
    double _DT; // flow observation time
    int _DN; // number of pedestrians that pass the line to trigger measurement
    // of the flow
    int _partialDoorUsage; //door usage in _DT
    double  _lastFlowMeasurement;


    std::string _flowAtExit;
    DoorState _state;

public:
    // last ped that passed was in room {1,2} that many ticks
//     int _lastTickTime1 = 0;
//     int _lastTickTime2 = 0;
//     int _refresh1 = 0;
//     int _refresh2 = 0;


    /**
     * Constructor
     */
    Crossing();

    /**
     * Destructor
     */
    virtual ~Crossing();

    /**
     * Set/Get the second subroom
     */
    //void SetSubRoom2(SubRoom* r2);

    /**
     * Set/Get the second subroom
     */
    //SubRoom* GetSubRoom2() const;

    /**
     * Close the door
     */
    void Close();

    /**
     * Temporaily closes the door
     */
    void TempClose();

    /**
     * Open the door
     */
    void Open();

    /**
     * \return true if the subroomID is associated with the current crossing
     */
    bool IsInSubRoom(int subroomID) const;

    /**
     * @return true if the state == OPEN
     */
    virtual bool IsOpen() const;

    /**
     * @return true if the state == TEMP_CLOSE
     */
    virtual bool IsTempClose() const;

    /**
     * @return true if the state == CLOSE
     */
    virtual bool IsClose() const;

    /**
      * @return true if the crossing is an exit/transition. (Transitions are derived from this class)
      * @see Transition
      */
    virtual bool IsExit() const;

    /**
     *
     * @return true if it is a transition
     */
    virtual bool IsTransition() const;

    /**
     * @return the other subroom not matching the data
     */
    virtual SubRoom* GetOtherSubRoom(int roomID, int subroomID) const;

    /**
     * Debug output
     */
    virtual void WriteToErrorLog() const;

    /**
     * @return a nicely formatted string representation of the object
     */
    virtual std::string GetDescription() const;

    /**
     * @return the number of subrooms this has in common with other (0, 1, 2)
     * @param other the other crossing
     * @param subroom returns one of the common subrooms
     */
    virtual int CommonSubroomWith(Crossing* other, SubRoom* &subroom);

    /**
     * Increment the number of persons that used that crossing
     * @param number, how many person have passed the crossing
     * @param time, at which time
     */
    void IncreaseDoorUsage(int number, double time);
    void IncreasePartialDoorUsage(int number);
    void ResetPartialDoorUsage();
    int GetPartialDoorUsage() const;

    /**
     * Resets the number of pedestrians that used that crossing
     */
    void ResetDoorUsage();

    /**
     * @return the number of pedestrians that used that crossing.
     */
    int GetDoorUsage() const;

    /**
     * @return outflow rate of crossing/transition
     */
    double  GetOutflowRate() const;

    /**
     * @return max of number of agents that can pass crossing/transition
     */
    int  GetMaxDoorUsage() const;

    /**
     * set _maxDoorUsage
     * @param max door usage
     */
    void SetMaxDoorUsage(int mdu);

    /**
     * set outflow rate
     * @param outflow
     *
     */
    void SetOutflowRate(double outflow);

    /**
     * @return the flow curve for this crossing
     */
    const std::string & GetFlowCurve() const;

    /**
     * @return the last time this crossing was crossed
     */
    double GetLastPassingTime() const;

    /**
     * @return the time we should wait until the door is reopened
     *
     */
    double GetClosingTime() const;

    /**
     *  @param dt: time to decrease the waiting time until door is reopened
     */
    void UpdateClosingTime(double dt);

    /**
     * Updates the state of temp closed doors. Door will be openend if it is
     * closed due to flow regulation if maxDoorUsage is not reached yet.
     *
     * @param[in] dt
     */
    void UpdateTemporaryState(double dt);
    double GetDT();
    void SetDT(double dt);
    int GetDN();
    void SetDN(int dt);

    bool RegulateFlow(double time);

    DoorState GetState() const;

    void SetState(DoorState _state);

    std::string toString() const;
};

#endif  /* _CROSSING_H */