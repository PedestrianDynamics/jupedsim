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
        FMT_STRING("After {} sec: {}, type: {}, platform: {}"),
        _time,
        action,
        _trainType._type,
        _platformID);
}

void TrainEvent::TrainArrival()
{
    int room_id = -1, subroom_id = -1;
    auto mytrack      = _building->GetTrackWalls(_trackStart, _trackEnd, room_id, subroom_id);
    Room * room       = _building->GetRoom(room_id);
    SubRoom * subroom = room->GetSubRoom(subroom_id);

    if(subroom == nullptr) {
        std::string message(fmt::format(
            FMT_STRING(
                "Simulation::correctGeometry got wrong room_id|subroom_id ({}|{}). TrainId {}"),
            room_id,
            subroom_id,
            _trainID));
        throw std::runtime_error(message);
    }

    if(mytrack.empty()) {
        return;
    }

    auto doors = _trainType._doors;
    for(auto && door : doors) {
        auto newX = door.GetPoint1()._x + _trainStart._x + _trackStart._x;
        auto newY = door.GetPoint1()._y + _trainStart._y + _trackStart._y;
        door.SetPoint1(Point(newX, newY));
        newX = door.GetPoint2()._x + _trainStart._x + _trackStart._x;
        newY = door.GetPoint2()._y + _trainStart._y + _trackStart._y;
        door.SetPoint2(Point(newX, newY));
    }

    for(const auto & door : doors) {
        LOG_DEBUG(
            "Train {} {}. Transformed coordinates of doors: {} -- {}",
            _trainType._type,
            _trainID,
            door.GetPoint1().toString(),
            door.GetPoint2().toString());
    }

    geometry::helper::AddTrainDoors(_trainID, *_building, *subroom, mytrack, doors);
}

void TrainEvent::TrainDeparture()
{
    // this function is composed of three copy/pasted blocks.
    int room_id, subroom_id;
    std::set<SubRoom *> subRoomsToUpdate;

    // remove temp added walls
    auto tempWalls = _building->GetTrainWallsAdded(_trainID);

    if(tempWalls.has_value()) {
        for(auto it = tempWalls.value().begin(); it != tempWalls.value().end();) {
            auto wall = *it;
            if(it != tempWalls.value().end()) {
                tempWalls.value().erase(it);
            }
            for(const auto & platform : _building->GetPlatforms()) {
                room_id    = platform.second->rid;
                subroom_id = platform.second->sid;
                SubRoom * subroom =
                    _building->GetAllRooms().at(room_id)->GetAllSubRooms().at(subroom_id).get();
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
        _building->SetTrainWallsAdded(_trainID, tempWalls.value());
    }

    // add removed walls
    auto tempRemovedWalls = _building->GetTrainWallsRemoved(_trainID);

    if(tempRemovedWalls.has_value()) {
        for(auto it = tempRemovedWalls.value().begin(); it != tempRemovedWalls.value().end();) {
            auto wall = *it;
            if(it != tempRemovedWalls.value().end()) {
                tempRemovedWalls.value().erase(it);
            }
            for(const auto & platform : _building->GetPlatforms()) {
                auto tracks = platform.second->tracks;
                room_id     = platform.second->rid;
                subroom_id  = platform.second->sid;
                SubRoom * subroom =
                    _building->GetAllRooms().at(room_id)->GetAllSubRooms().at(subroom_id).get();
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
        _building->SetTrainWallsRemoved(_trainID, tempRemovedWalls.value());
    }

    // remove added doors
    auto tempDoors = _building->GetTrainDoorsAdded(_trainID);

    if(tempDoors.has_value()) {
        for(auto it = tempDoors.value().begin(); it != tempDoors.value().end();) {
            auto door = *it;
            if(it != tempDoors.value().end()) {
                tempDoors.value().erase(it);
            }
            for(const auto & platform : _building->GetPlatforms()) {
                auto tracks = platform.second->tracks;
                room_id     = platform.second->rid;
                subroom_id  = platform.second->sid;
                SubRoom * subroom =
                    _building->GetAllRooms().at(room_id)->GetAllSubRooms().at(subroom_id).get();
                for(auto subTrans : subroom->GetAllTransitions()) {
                    if(*subTrans == door) {
                        // Trnasitions are added to subrooms and building!!
                        subroom->RemoveTransition(subTrans);
                        _building->RemoveTransition(subTrans);
                    }
                }
                subRoomsToUpdate.insert(subroom);
            }
        }
        _building->SetTrainDoorsAdded(_trainID, tempDoors.value());
    }


    std::for_each(std::begin(subRoomsToUpdate), std::end(subRoomsToUpdate), [](SubRoom * subroom) {
        subroom->Update();
    });
}
