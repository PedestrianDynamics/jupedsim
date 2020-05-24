#include "DoorEvent.h"

#include <fmt/format.h>


//============================================================================
// DoorCloseEvent
//============================================================================
DoorCloseEvent::DoorCloseEvent(Building * building, int doorID, double time) :
    Event(time), _building(building), _doorID(doorID)
{
}

void DoorCloseEvent::Process()
{
    _building->GetTransition(_doorID)->Close(true);
}

std::string DoorCloseEvent::ToString() const
{
    return fmt::format(FMT_STRING("After {} sec: close door {}"), _time, _doorID);
}

//============================================================================
// DoorTempCloseEvent
//============================================================================
DoorTempCloseEvent::DoorTempCloseEvent(Building * building, int doorID, double time) :
    Event(time), _building(building), _doorID(doorID)
{
}

void DoorTempCloseEvent::Process()
{
    _building->GetTransition(_doorID)->TempClose(true);
}

std::string DoorTempCloseEvent::ToString() const
{
    return fmt::format(FMT_STRING("After {} sec: temp_close door {}"), _time, _doorID);
}

//============================================================================
// DoorOpenEvent
//============================================================================
DoorOpenEvent::DoorOpenEvent(Building * building, int doorID, double time) :
    Event(time), _building(building), _doorID(doorID)
{
}

void DoorOpenEvent::Process()
{
    _building->GetTransition(_doorID)->Open(true);
}

std::string DoorOpenEvent::ToString() const
{
    return fmt::format(FMT_STRING("After {} sec: open door {}"), _time, _doorID);
}

//============================================================================
// DoorResetEvent
//============================================================================
DoorResetEvent::DoorResetEvent(Building * building, int doorID, double time) :
    Event(time), _building(building), _doorID(doorID)
{
}

void DoorResetEvent::Process()
{
    _building->GetTransition(_doorID)->ResetDoorUsage();
}

std::string DoorResetEvent::ToString() const
{
    return fmt::format(FMT_STRING("After {} sec: reset_usage door {}"), _time, _doorID);
}
