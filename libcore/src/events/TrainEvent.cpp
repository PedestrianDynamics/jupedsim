#include "TrainEvent.h"

#include "geometry/SubRoom.h"
#include "geometry/Wall.h"
#include "geometry/helper/CorrectGeometry.h"

#include <fmt/format.h>

TrainArrivalEvent::TrainArrivalEvent(double time, TrainEventInfo info) :
    Event(time), _info(std::move(info))
{
}

void TrainArrivalEvent::Process()
{
    const auto track = _info.building->GetTrack(_info.trackID);
    if(!track.has_value()) {
        std::string message = fmt::format(
            FMT_STRING("Could not find a track with ID {}. Please check your geometry."),
            _info.trackID);
        throw std::runtime_error(message);
    }

    const auto start = _info.building->GetTrackStart(_info.trackID);
    if(!start.has_value()) {
        std::string message = fmt::format(
            FMT_STRING("Could not find a start for track with ID {}. Please check your geometry."),
            _info.trackID);
        throw std::runtime_error(message);
    }

    int roomID    = track->_roomID;
    int subroomID = track->_subRoomID;
    auto subroom  = _info.building->GetRoom(roomID)->GetSubRoom(subroomID);

    geometry::helper::AddTrainDoors(
        _info.trainID,
        *_info.building,
        *subroom,
        _info.trainType,
        track.value(),
        _info.trainStartOffset,
        _info.reversed);
}

std::string TrainArrivalEvent::ToString() const
{
    return fmt::format(
        FMT_STRING("After {} sec: {}, type: {}, platform: {}"),
        _time,
        "Train arrives",
        _info.trainType._type,
        _info.trackID);
}

TrainDepartureEvent::TrainDepartureEvent(double time, TrainEventInfo info) :
    Event(time), _info(std::move(info))
{
}

void TrainDepartureEvent::Process()
{
    auto track = _info.building->GetTrack(_info.trackID);
    if(!track.has_value()) {
        throw std::runtime_error("");
    }

    int roomID    = track->_roomID;
    int subroomID = track->_subRoomID;
    auto subroom  = _info.building->GetRoom(roomID)->GetSubRoom(subroomID);

    // remove temp added walls
    auto tempAddedWalls = _info.building->GetTrainWallsAdded(_info.trainID);
    if(tempAddedWalls.has_value()) {
        std::for_each(
            std::begin(tempAddedWalls.value()),
            std::end(tempAddedWalls.value()),
            [&subroom](const Wall & wall) { subroom->RemoveWall(wall); });

        _info.building->ClearTrainWallsAdded(_info.trainID);
    }

    // add removed walls
    auto tempRemovedWalls = _info.building->GetTrainWallsRemoved(_info.trainID);
    if(tempRemovedWalls.has_value()) {
        std::for_each(
            std::begin(tempRemovedWalls.value()),
            std::end(tempRemovedWalls.value()),
            [&subroom](const Wall & wall) { subroom->AddWall(wall); });

        _info.building->ClearTrainWallsRemoved(_info.trainID);
    }

    // remove added doors
    auto tempDoors = _info.building->GetTrainDoorsAdded(_info.trainID);
    if(tempDoors.has_value()) {
        std::for_each(
            std::begin(tempDoors.value()),
            std::end(tempDoors.value()),
            [&subroom, this](Transition & door) {
                subroom->RemoveTransitionByUID(door.GetUniqueID());
                _info.building->RemoveTransition(&door);
            });

        _info.building->ClearTrainDoorsAdded(_info.trainID);
    }

    subroom->Update();
}

std::string TrainDepartureEvent::ToString() const
{
    return fmt::format(
        FMT_STRING("After {} sec: {}, type: {}, platform: {}"),
        _time,
        "Train departs",
        _info.trainType._type,
        _info.trackID);
}
