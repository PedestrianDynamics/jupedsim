//
// Created by Tobias Schrödter on 2019-05-13.
//

#ifndef JPSCORE_DIRECTIONMANAGER_H
#define JPSCORE_DIRECTIONMANAGER_H

#include "general/Macros.h"
#include <memory>

class WaitingStrategy;
class DirectionStrategy;
class Point;
class Pedestrian;
class Building;
class Room;

class DirectionManager {

private:
     std::shared_ptr<WaitingStrategy> waitingStrategy;
     std::shared_ptr<DirectionStrategy> directionStrategy;

private:
    Building* building;

public:
    void Init(Building*);
    Point GetTarget(Room*, Pedestrian* ped);

    const std::shared_ptr<WaitingStrategy>& GetWaitingStrategy() const;

    void SetWaitingStrategy(const std::shared_ptr<WaitingStrategy>& waitingStrategy);

    const std::shared_ptr<DirectionStrategy>& GetDirectionStrategy() const;

    void SetDirectionStrategy(const std::shared_ptr<DirectionStrategy>& directionStrategy);


};

#endif //JPSCORE_DIRECTIONMANAGER_H
