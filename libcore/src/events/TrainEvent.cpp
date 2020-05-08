#include "TrainEvent.h"

#include "geometry/SubRoom.h"
#include "geometry/Wall.h"

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
    SubRoom * subroom;
    int room_id, subroom_id;
    auto mytrack = _building->GetTrackWalls(_trackStart, _trackEnd, room_id, subroom_id);
    Room * room  = _building->GetRoom(room_id);
    subroom      = room->GetSubRoom(subroom_id);
    if(subroom == nullptr) {
        LOG_ERROR(
            "Simulation::correctGeometry got wrong room_id|subroom_id ({}|{}). TrainId {}",
            room_id,
            subroom_id,
            _trainID);
        exit(EXIT_FAILURE);
    }
    static int transition_id = 10000; // randomly high number

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
        LOG_INFO(
            "Train {} {}. Transformed coordinates of doors: {} -- {}",
            _trainType._type,
            _trainID,
            door.GetPoint1().toString(),
            door.GetPoint2().toString());
    }

    auto pws   = _building->GetIntersectionPoints(doors, mytrack);
    auto walls = subroom->GetAllWalls();
    //---
    for(auto pw : pws) {
        auto pw1 = pw.first;
        auto pw2 = pw.second;
        auto p1  = pw1.first;
        auto w1  = pw1.second;
        auto p2  = pw2.first;
        auto w2  = pw2.second;
        // case 1
        Point P;
        if(w1 == w2) {
            Transition * e = new Transition();
            e->SetID(transition_id++);
            e->SetCaption(_trainType._type);
            e->SetPoint1(p1);
            e->SetPoint2(p2);
            std::string transType =
                "Train_" + std::to_string(_trainID) + "_" + std::to_string(_time);
            e->SetType(transType);
            room->AddTransitionID(e->GetUniqueID()); // danger area
            e->SetRoom1(room);
            e->SetSubRoom1(subroom);
            subroom->AddTransition(e);   // danger area
            _building->AddTransition(e); // danger area

            double dist_pt1 = (w1.GetPoint1() - e->GetPoint1()).NormSquare();
            double dist_pt2 = (w1.GetPoint1() - e->GetPoint2()).NormSquare();
            Point A, B;

            if(dist_pt1 < dist_pt2) {
                A = e->GetPoint1();
                B = e->GetPoint2();
            } else {
                A = e->GetPoint2();
                B = e->GetPoint1();
            }

            Wall NewWall(w1.GetPoint1(), A);
            Wall NewWall1(w1.GetPoint2(), B);
            NewWall.SetType(w1.GetType());
            NewWall1.SetType(w1.GetType());

            // add new lines to be controled against overlap with exits
            if(NewWall.GetLength() > J_EPS_DIST) {
                _building->TempAddedWalls[_trainID].push_back(NewWall);
                subroom->AddWall(NewWall);
            }


            if(NewWall1.GetLength() > J_EPS_DIST) {
                _building->TempAddedWalls[_trainID].push_back(NewWall1);
                subroom->AddWall(NewWall1);
            }

            _building->TempAddedDoors[_trainID].push_back(*e);
            _building->TempRemovedWalls[_trainID].push_back(w1);
            subroom->RemoveWall(w1);

        } else if(w1.ShareCommonPointWith(w2, P)) {
            //------------ transition --------
            Transition * e = new Transition();
            e->SetID(transition_id++);
            e->SetCaption(_trainType._type);
            e->SetPoint1(p1);
            e->SetPoint2(p2);
            std::string transType =
                "Train_" + std::to_string(_trainID) + "_" + std::to_string(_time);
            e->SetType(transType);
            room->AddTransitionID(e->GetUniqueID()); // danger area
            e->SetRoom1(room);
            e->SetSubRoom1(subroom);

            subroom->AddTransition(e);   // danger area
            _building->AddTransition(e); // danger area
            //--------------------------------
            Point N, M;
            if(w1.GetPoint1() == P)
                N = w1.GetPoint2();
            else
                N = w1.GetPoint1();

            if(w2.GetPoint1() == P)
                M = w2.GetPoint2();
            else
                M = w2.GetPoint1();

            Wall NewWall(N, p1);
            Wall NewWall1(M, p2);
            NewWall.SetType(w1.GetType());
            NewWall1.SetType(w2.GetType());
            // changes to building
            _building->TempAddedWalls[_trainID].push_back(NewWall);
            _building->TempAddedWalls[_trainID].push_back(NewWall1);
            _building->TempAddedDoors[_trainID].push_back(*e);
            _building->TempRemovedWalls[_trainID].push_back(w1);
            _building->TempRemovedWalls[_trainID].push_back(w2);
            subroom->AddWall(NewWall);
            subroom->AddWall(NewWall1);
            subroom->RemoveWall(w1);
            subroom->RemoveWall(w2);
        } else // disjoint
        {
            //------------ transition --------
            Transition * e = new Transition();
            e->SetID(transition_id++);
            e->SetCaption(_trainType._type);
            e->SetPoint1(p1);
            e->SetPoint2(p2);
            std::string transType =
                "Train_" + std::to_string(_trainID) + "_" + std::to_string(_time);
            e->SetType(transType);
            room->AddTransitionID(e->GetUniqueID()); // danger area
            e->SetRoom1(room);
            e->SetSubRoom1(subroom);

            subroom->AddTransition(e);   // danger area
            _building->AddTransition(e); // danger area
            //--------------------------------
            // find points on w1 and w2 between p1 and p2
            // (A, B)
            Point A, B;
            if(e->isBetween(w1.GetPoint1()))
                A = w1.GetPoint2();
            else
                A = w1.GetPoint1();

            if(e->isBetween(w2.GetPoint1()))
                B = w2.GetPoint2();
            else
                B = w2.GetPoint1();

            Wall NewWall(A, p1);
            Wall NewWall1(B, p2);
            NewWall.SetType(w1.GetType());
            NewWall1.SetType(w2.GetType());
            // remove walls between
            for(auto wall : mytrack) {
                if(e->isBetween(wall.GetPoint1()) || e->isBetween(wall.GetPoint2())) {
                    _building->TempRemovedWalls[_trainID].push_back(wall);
                    subroom->RemoveWall(wall);
                }
            }
            // changes to building
            _building->TempAddedWalls[_trainID].push_back(NewWall);
            _building->TempAddedWalls[_trainID].push_back(NewWall1);
            _building->TempAddedDoors[_trainID].push_back(*e);
            subroom->AddWall(NewWall);
            subroom->AddWall(NewWall1);
        }
    }
    subroom->Update();
}

void TrainEvent::TrainDeparture()
{
    // this function is composed of three copy/pasted blocks.
    int room_id, subroom_id;
    std::set<SubRoom *> subRoomsToUpdate;
    // remove temp added walls
    auto tempWalls = _building->TempAddedWalls[_trainID];
    for(auto it = tempWalls.begin(); it != tempWalls.end();) {
        auto wall = *it;
        if(it != tempWalls.end()) {
            tempWalls.erase(it);
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
    _building->TempAddedWalls[_trainID] = tempWalls;

    /*       // add remove walls */
    auto tempRemovedWalls = _building->TempRemovedWalls[_trainID];
    for(auto it = tempRemovedWalls.begin(); it != tempRemovedWalls.end();) {
        auto wall = *it;
        if(it != tempRemovedWalls.end()) {
            tempRemovedWalls.erase(it);
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
    _building->TempRemovedWalls[_trainID] = tempRemovedWalls;

    /*       // remove added doors */
    auto tempDoors = _building->TempAddedDoors[_trainID];
    for(auto it = tempDoors.begin(); it != tempDoors.end();) {
        auto door = *it;
        if(it != tempDoors.end()) {
            tempDoors.erase(it);
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
    _building->TempAddedDoors[_trainID] = tempDoors;

    std::for_each(std::begin(subRoomsToUpdate), std::end(subRoomsToUpdate), [](SubRoom * subroom) {
        subroom->Update();
    });
}
