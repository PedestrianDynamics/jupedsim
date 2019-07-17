//
// Created by Tobias Schrödter on 15.10.18.
//

#include "WaitingArea.h"
#include "Building.h"

int WaitingArea::getMaxNumPed() const
{
     return maxNumPed;
}

void WaitingArea::setMaxNumPed(int newMaxNumPed)
{
     maxNumPed = newMaxNumPed;
}

int WaitingArea::getMinNumPed() const
{
     return minNumPed;
}

void WaitingArea::setMinNumPed(int newMinNumPed)
{
     minNumPed = newMinNumPed;
}

bool WaitingArea::isOpen() const
{
     return open;
}

void WaitingArea::setOpen(bool newOpenState)
{
     open = newOpenState;
}

bool WaitingArea::isGlobalTimer() const
{
     return globalTimer;
}

void WaitingArea::setGlobalTimer(bool timer)
{
     WaitingArea::globalTimer = timer;
}


const std::map<int, double>& WaitingArea::getNextGoals() const
{
     return nextGoals;
}

bool WaitingArea::setNextGoals(const std::map<int, double>& newNextGoals)
{
     nextGoals = newNextGoals;

     nextGoalsOpen.clear();

     for (auto& goalItr : nextGoals){
          nextGoalsOpen[goalItr.first] = true;
     }

     return checkProbabilities();
}

bool WaitingArea::checkProbabilities()
{
     double p =0.;
     for ( std::map<int, double>::iterator it = nextGoals.begin(); it != nextGoals.end(); it++ ){
          p += it->second;
     }

     return (p>0.9999) && (p<1.000001);
}

void WaitingArea::updateProbabilities()
{

}

// Open or closes goal with id
void WaitingArea::updateProbabilities(bool isOpen, int id)
{
     if ( nextGoalsOpen.find(id) != nextGoalsOpen.end() ) {
          nextGoalsOpen[id] = isOpen;
     }
}


std::string WaitingArea::toString()
{
     std::string out;
     char buffer [50];


     out.append("WaitingArea[\n");
     sprintf(buffer, "\tid=%d\n", _id);
     out.append(buffer);
     sprintf(buffer, "\tcaption=%s\n", _caption.c_str());
     out.append(buffer);
     sprintf(buffer, "\tmin_peds=%d\n", minNumPed);
     out.append(buffer);
     sprintf(buffer, "\tmax_peds=%d\n", maxNumPed);
     out.append(buffer);
     sprintf(buffer, "\tis_open=%d\n", open);
     out.append(buffer);
     sprintf(buffer, "\twaiting_time=%f\n", waitingTime);
     out.append(buffer);

     for (auto const& nextWa : nextGoals){
          sprintf(buffer, "\tnext id=%d\tp=%f\n", nextWa.first, nextWa.second);
          out.append(buffer);
     }
     out.append("]");
     return out;

}

double WaitingArea::getWaitingTime() const
{
     return waitingTime;
}

void WaitingArea::setWaitingTime(double newWaitingTime)
{
     waitingTime = newWaitingTime;
}

int WaitingArea::GetNextGoal()
{
     double random = ((double) std::rand() / (RAND_MAX));
     double cumProb = 0.;

     for (auto& nextGoal : nextGoals){
          cumProb += nextGoal.second;
          if ((nextGoalsOpen[nextGoal.first]) && (random <= cumProb)){
               return nextGoal.first;
          }
     }
     return this->_id;
}

void WaitingArea::addPed(int ped)
{
     pedInside.insert(ped);
     if (pedInside.size() >= maxNumPed){
          open = false;
     }
}

void WaitingArea::removePed(int ped)
{
     pedInside.erase(ped);
     if (pedInside.size() < maxNumPed){
          open = true;
     }
     if (pedInside.size() < minNumPed){
          startTime = -1.;
     }
}

void WaitingArea::startTimer(double time)
{
     startTime = time;
//     std::cout << "Timer started at " << startTime << std::endl;
}

bool WaitingArea::isWaiting(double time, const Building* building)
{
     Transition* trans = nullptr;
     if (transitionID >= 0){
          trans = building->GetTransition(transitionID);
     }

     if ((pedInside.size() >= minNumPed) && (startTime < 0. )){
          startTimer(time);
     }

     if (trans == nullptr){
          if (globalTimer){
               if (time > waitingTime){
                    return false;
               }
          }else{
               if ((time > startTime + waitingTime) && (startTime >= 0. )){
                    return false;
               }
          }
     }else{
          if (trans->IsOpen()){
               return false;
          }
     }

//     std::cout << "Waiting ..." << std::endl;
     return true;
}

int WaitingArea::getNumPed()
{
     return pedInside.size();
}

int WaitingArea::getTransitionID() const
{
     return transitionID;
}

void WaitingArea::setTransitionID(int newTransitionID)
{
     transitionID = newTransitionID;
}
