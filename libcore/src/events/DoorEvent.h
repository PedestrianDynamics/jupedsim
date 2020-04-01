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
public:
    /**
      * Constructor
      * @param id
      * @param time
      * @param state
      */
    DoorEvent(int id, double time, EventAction action);

    /**
      * Destructor
      */
    virtual ~DoorEvent() = default;

    void Process() override;

    [[nodiscard]] int GetDoorID() const;
    [[nodiscard]] std::string GetDescription() const override;

private:
    /**
     * ID of transition which is effected by event.
     */
    int _doorID;

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
