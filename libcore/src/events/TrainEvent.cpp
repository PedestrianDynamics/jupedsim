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
    int room_id    = -1;
    int subroom_id = -1;
    auto mytrack =
        _info.building->GetTrackWalls(_info.trackStart, _info.trackEnd, room_id, subroom_id);
    Room * room       = _info.building->GetRoom(room_id);
    SubRoom * subroom = room->GetSubRoom(subroom_id);

    if(subroom == nullptr) {
        std::string message(fmt::format(
            FMT_STRING(
                "Simulation::correctGeometry got wrong room_id|subroom_id ({}|{}). TrainId {}"),
            room_id,
            subroom_id,
            _info.trainID));
        throw std::runtime_error(message);
    }

    if(mytrack.empty()) {
        return;
    }

    auto doors = _info.trainType._doors;
    for(auto && door : doors) {
        auto newX = door.GetPoint1()._x + _info.trainStart._x + _info.trackStart._x;
        auto newY = door.GetPoint1()._y + _info.trainStart._y + _info.trackStart._y;
        door.SetPoint1(Point(newX, newY));
        newX = door.GetPoint2()._x + _info.trainStart._x + _info.trackStart._x;
        newY = door.GetPoint2()._y + _info.trainStart._y + _info.trackStart._y;
        door.SetPoint2(Point(newX, newY));
    }

    for(const auto & door : doors) {
        LOG_DEBUG(
            "Train {} {}. Transformed coordinates of doors: {} -- {}",
            _info.trainType._type,
            _info.trainID,
            door.GetPoint1().toString(),
            door.GetPoint2().toString());
    }

    geometry::helper::AddTrainDoors(_info.trainID, *_info.building, *subroom, mytrack, doors);
}

std::string TrainArrivalEvent::ToString() const
{
    return fmt::format(
        FMT_STRING("After {} sec: {}, type: {}, platform: {}"),
        _time,
        "Train arrives",
        _info.trainType._type,
        _info.platformID);
}

TrainDepartureEvent::TrainDepartureEvent(double time, TrainEventInfo info) :
    Event(time), _info(std::move(info))
{
}

void TrainDepartureEvent::Process()
{
    // this function is composed of three copy/pasted blocks.
    int room_id;
    int subroom_id;
    std::set<SubRoom *> subRoomsToUpdate;

    // remove temp added walls
    auto tempWalls = _info.building->GetTrainWallsAdded(_info.trainID);

    if(tempWalls.has_value()) {
        for(auto it = tempWalls.value().begin(); it != tempWalls.value().end();) {
            auto wall = *it;
            if(it != tempWalls.value().end()) {
                tempWalls.value().erase(it);
            }
            for(const auto & platform : _info.building->GetPlatforms()) {
                room_id           = platform.second->rid;
                subroom_id        = platform.second->sid;
                SubRoom * subroom = _info.building->GetAllRooms()
                                        .at(room_id)
                                        ->GetAllSubRooms()
                                        .at(subroom_id)
                                        .get();
                for(const auto & subWall : subroom->GetAllWalls()) {
                    if(subWall == wall) {
                        // if everything goes right, then we should enter this
                        // if. We already erased from tempWalls!
                        subroom->RemoveWall(wall);
                    } //if
                }     //subroom
                subRoomsToUpdate.insert(subroom);
            } //platforms
        }
        _info.building->SetTrainWallsAdded(_info.trainID, tempWalls.value());
    }

    // add removed walls
    auto tempRemovedWalls = _info.building->GetTrainWallsRemoved(_info.trainID);

    if(tempRemovedWalls.has_value()) {
        for(auto it = tempRemovedWalls.value().begin(); it != tempRemovedWalls.value().end();) {
            auto wall = *it;
            if(it != tempRemovedWalls.value().end()) {
                tempRemovedWalls.value().erase(it);
            }
            for(const auto & platform : _info.building->GetPlatforms()) {
                auto tracks       = platform.second->tracks;
                room_id           = platform.second->rid;
                subroom_id        = platform.second->sid;
                SubRoom * subroom = _info.building->GetAllRooms()
                                        .at(room_id)
                                        ->GetAllSubRooms()
                                        .at(subroom_id)
                                        .get();
                for(const auto & track : tracks) {
                    auto walls = track.second;
                    for(const auto & trackWall : walls) {
                        if(trackWall == wall) {
                            subroom->AddWall(wall);
                        }
                    }
                }
                subRoomsToUpdate.insert(subroom);
            }
        }
        _info.building->SetTrainWallsRemoved(_info.trainID, tempRemovedWalls.value());
    }

    // remove added doors
    auto tempDoors = _info.building->GetTrainDoorsAdded(_info.trainID);

    if(tempDoors.has_value()) {
        for(auto it = tempDoors.value().begin(); it != tempDoors.value().end();) {
            auto door = *it;
            if(it != tempDoors.value().end()) {
                tempDoors.value().erase(it);
            }
            for(const auto & platform : _info.building->GetPlatforms()) {
                auto tracks       = platform.second->tracks;
                room_id           = platform.second->rid;
                subroom_id        = platform.second->sid;
                SubRoom * subroom = _info.building->GetAllRooms()
                                        .at(room_id)
                                        ->GetAllSubRooms()
                                        .at(subroom_id)
                                        .get();
                for(auto subTrans : subroom->GetAllTransitions()) {
                    if(*subTrans == door) {
                        // Trnasitions are added to subrooms and building!!
                        subroom->RemoveTransition(subTrans);
                        _info.building->RemoveTransition(subTrans);
                    }
                }
                subRoomsToUpdate.insert(subroom);
            }
        }
        _info.building->SetTrainDoorsAdded(_info.trainID, tempDoors.value());
    }


    std::for_each(std::begin(subRoomsToUpdate), std::end(subRoomsToUpdate), [](SubRoom * subroom) {
        subroom->Update();
    });
}

std::string TrainDepartureEvent::ToString() const
{
    return fmt::format(
        FMT_STRING("After {} sec: {}, type: {}, platform: {}"),
        _time,
        "Train departs",
        _info.trainType._type,
        _info.platformID);
}
