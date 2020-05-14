#include "TrainEvent.h"

#include "geometry/SubRoom.h"
#include "geometry/Wall.h"
#include "geometry/helper/CorrectGeometry.h"

void TrainEvent::Process()
{
    LOG_INFO("{}", GetDescription());
    switch(_action) {
        case EventAction::TRAIN_ARRIVAL:
            TrainArrival();
            break;
        case EventAction::TRAIN_DEPARTURE:
            TrainDeparture();
            break;
        default:
            throw std::runtime_error("Wrong EventAction for TrainEvent!");
    }
}

std::string TrainEvent::GetDescription() const
{
    std::string action;
    switch(_action) {
        case EventAction::TRAIN_ARRIVAL:
            action = "Train arrives";
            break;
        case EventAction::TRAIN_DEPARTURE:
            action = "Train departs";
            break;
        default:
            throw std::runtime_error("Wrong EventAction for TrainEvent!");
    }
    return fmt::format(
        FMT_STRING("After {} sec: {}, type: {}, track_id: {}"),
        _time,
        action,
        _trainType._type,
        _trackID);
}

void TrainEvent::TrainArrival()
{
    auto track = _building->GetTrack(_trackID);
    if(!track.has_value()) {
        std::string message = fmt::format(
            FMT_STRING("Could not find a track with ID {}. Please check your geometry."), _trackID);
        throw std::runtime_error(message);
    }

    auto start = _building->GetTrackStart(_trackID);
    if(!start.has_value()) {
        std::string message = fmt::format(
            FMT_STRING("Could not find a start for track with ID {}. Please check your geometry."),
            _trackID);
        throw std::runtime_error(message);
    }

    int roomID    = track->_roomID;
    int subroomID = track->_subRoomID;
    auto subroom  = _building->GetRoom(roomID)->GetSubRoom(subroomID);

    geometry::helper::AddTrainDoors(
        _trainID, *_building, *subroom, _trainType, track.value(), _trainStartOffset, _fromEnd);
}

void TrainEvent::TrainDeparture()
{
    auto track = _building->GetTrack(_trackID);
    if(!track.has_value()) {
        throw std::runtime_error("");
    }

    int roomID    = track->_roomID;
    int subroomID = track->_subRoomID;
    auto subroom  = _building->GetRoom(roomID)->GetSubRoom(subroomID);

    // remove temp added walls
    auto tempAddedWalls = _building->GetTrainWallsAdded(_trainID);
    if(tempAddedWalls.has_value()) {
        std::for_each(
            std::begin(tempAddedWalls.value()),
            std::end(tempAddedWalls.value()),
            [&subroom](const Wall & wall) { subroom->RemoveWall(wall); });

        _building->ClearTrainWallsAdded(_trainID);
    }

    // add removed walls
    auto tempRemovedWalls = _building->GetTrainWallsRemoved(_trainID);
    if(tempRemovedWalls.has_value()) {
        std::for_each(
            std::begin(tempRemovedWalls.value()),
            std::end(tempRemovedWalls.value()),
            [&subroom](const Wall & wall) { subroom->AddWall(wall); });

        _building->ClearTrainWallsRemoved(_trainID);
    }

    // remove added doors
    auto tempDoors = _building->GetTrainDoorsAdded(_trainID);
    if(tempDoors.has_value()) {
        std::for_each(
            std::begin(tempDoors.value()),
            std::end(tempDoors.value()),
            [&subroom, this](Transition & door) {
                subroom->RemoveTransitionByUID(door.GetUniqueID());
                _building->RemoveTransition(&door);
            });

        _building->ClearTrainDoorsAdded(_trainID);
    }

    subroom->Update();
}
