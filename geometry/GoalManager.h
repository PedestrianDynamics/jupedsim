//
// Created by Tobias Schrödter on 2019-02-17.
//

#ifndef JPSCORE_GOALMANAGER_H
#define JPSCORE_GOALMANAGER_H

#include <map>

class Goal;
class Pedestrian;

class GoalManager {

private:
    std::map<int, Goal*> _allGoals;

public:
    void SetGoals(std::map<int, Goal*> goals);
    void ProcessPedPosition(Pedestrian* ped);

private:
    bool CheckInside(Pedestrian* ped, int goalID);
    bool CheckInsideWaitingArea(Pedestrian* ped, int goalID);
    void SetState(int goalID, bool state);

};

#endif //JPSCORE_GOALMANAGER_H
