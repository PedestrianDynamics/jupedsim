/*
 * DoorEvent.h
 *
 *  Created on: Jul 21, 2015
 *      Author: piccolo
 */

#pragma once

#include "Event.h"

#include <string>

class DoorEvent : public Event
{
private:
    /**
     * ID of transition which is effected by event.
     */
    int _doorID;

public:
    /**
      * Constructor
      * @param doorID
      * @param time
      * @param state
      */
    DoorEvent(int doorID, double time, EventAction action);

    /**
      * Destructor
      */
    virtual ~DoorEvent() = default;

    void Process() override;

    [[nodiscard]] int GetDoorID() const;
    [[nodiscard]] std::string GetDescription() const override;

private:
    /**
     * Closes the transition identified by \a _doorID.
     */
    void CloseDoor();

    /**
     * Temp closes the transition identified by \a _doorID.
     */
    void TempCloseDoor();

    /**
     * Opens the transition identified by \a _doorID.
     */
    void OpenDoor();

    /**
     * Resets the door usage of transition identified by \a _doorID.
     */
    void ResetDoor();
};
