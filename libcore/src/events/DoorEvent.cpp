/*
 * DoorEvent.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: piccolo
 */

#include "DoorEvent.h"

#include "pedestrian/Pedestrian.h"

#include <algorithm>

DoorEvent::DoorEvent(int doorID, double time, EventAction action)
{
    _doorID = doorID;
    _time   = time;
    _action = action;
}

std::string DoorEvent::GetDescription() const
{
    std::string action;
    switch(_action) {
        case EventAction::DOOR_OPEN:
            action = "open";
            break;
        case EventAction::DOOR_CLOSE:
            action = "close";
            break;
        case EventAction::DOOR_TEMP_CLOSE:
            action = "temp_close";
            break;
        case EventAction::DOOR_RESET_USAGE:
            action = "reset_usage";
            break;
        default:
            return std::string{"something went wrong"};
    }
    return fmt::format(FMT_STRING("After {} sec: {} door {}"), _time, action, _doorID);
}

void DoorEvent::Process()
{
    LOG_INFO("{}", GetDescription());
    switch(_action) {
        case EventAction::DOOR_OPEN:
            OpenDoor();
            break;
        case EventAction::DOOR_CLOSE:
            CloseDoor();
            break;
        case EventAction::DOOR_TEMP_CLOSE:
            TempCloseDoor();
            break;
        case EventAction::DOOR_RESET_USAGE:
            ResetDoor();
            break;
        default:
            throw std::runtime_error("Wrong EventAction for DoorEvent!");
    }
}

void DoorEvent::CloseDoor()
{
    _building->GetTransition(_doorID)->Close(true);
}

void DoorEvent::TempCloseDoor()
{
    _building->GetTransition(_doorID)->TempClose(true);
}

void DoorEvent::OpenDoor()
{
    _building->GetTransition(_doorID)->Open(true);
}

void DoorEvent::ResetDoor()
{
    _building->GetTransition(_doorID)->ResetDoorUsage();
}
