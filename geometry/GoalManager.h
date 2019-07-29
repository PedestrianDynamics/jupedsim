//
// Created by Tobias Schrödter on 2019-02-17.
//

#ifndef JPSCORE_GOALMANAGER_H
#define JPSCORE_GOALMANAGER_H

#include <map>

class Goal;
class Pedestrian;
class Building;


class GoalManager {

private:
    std::map<int, Goal*> _allGoals;
    Building* _building;

public:
    void SetGoals(std::map<int, Goal*> goals);
    void SetBuilding(Building* building);
    void ProcessPedPosition(Pedestrian* ped);
    void ProcessWaitingAreas(double time);

private:
    bool CheckInside(Pedestrian* ped, int goalID);
    bool CheckInsideWaitingArea(Pedestrian* ped, int goalID);
    void SetState(int goalID, bool state);

};
#endif //JPSCORE_GOALMANAGER_H
