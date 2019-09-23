/**
 * \file        WaitingArea.cpp
 * \date        Oct 15, 2018
 * \version     v0.8
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * Class providing inside goals which can be used to model intermediate goals
 * within the geometry. Additionally the pedestrians may wait for a certain
 * time or till a transition opens inside the waiting area.
 */
#include "WaitingArea.h"

#include "Building.h"

#include <sstream>

int WaitingArea::GetMaxNumPed() const
{
     return _maxNumPed;
}

void WaitingArea::SetMaxNumPed(int maxNumPed)
{
     WaitingArea::_maxNumPed = maxNumPed;
}

int WaitingArea::GetMinNumPed() const
{
     return _minNumPed;
}

void WaitingArea::SetMinNumPed(int minNumPed)
{
     WaitingArea::_minNumPed = minNumPed;
}

bool WaitingArea::IsOpen() const
{
     return _open;
}

void WaitingArea::SetOpen(bool open)
{
     WaitingArea::_open = open;
}

bool WaitingArea::IsGlobalTimer() const
{
     return _globalTimer;
}

void WaitingArea::SetGlobalTimer(bool timer)
{
     WaitingArea::_globalTimer = timer;
}

const std::map<int, double>& WaitingArea::GetNextGoals() const
{
     return _nextGoals;
}

bool WaitingArea::SetNextGoals(const std::map<int, double>& nextGoals)
{
     WaitingArea::_nextGoals = nextGoals;

     // Check for open waiting areas
     _nextGoalsOpen.clear();
     for (auto& goalItr : nextGoals){
          _nextGoalsOpen[goalItr.first] = true;
     }

     return CheckProbabilities();
}

bool WaitingArea::CheckProbabilities()
{
     double p =0.;
     for(auto[_, value] : _nextGoals) {
          p += value;
     }

     return (p>0.9999) && (p<1.000001);
}

void WaitingArea::UpdateProbabilities(bool isOpen, int id)
{
     if ( _nextGoalsOpen.find(id) != _nextGoalsOpen.end() ) {
          _nextGoalsOpen[id] = isOpen;
     }
}

std::string WaitingArea::toString()
{
     std::stringstream buffer;
     buffer << "WaitingArea[\n\tid=" << _id
            << "\n\tcaption=" << _caption
            << "\n\tmin_peds=" << _minNumPed
            << "\n\tmax_peds=" << _maxNumPed
            << "\n\tis_open=" << std::boolalpha << _open
            << "\n\twaiting_time=" << _waitingTime
            << "\n\tglobal_timer=" << std::boolalpha << _caption
            << "\n\ttransition_id=" << _transitionID;

     for(auto[id, p] : _nextGoals) {
          buffer << "\n\tnext id="<< id << "\tp="<<p;
     }
     buffer << "\n]";
     return buffer.str();
}

int WaitingArea::GetWaitingTime() const
{
     return _waitingTime;
}

void WaitingArea::SetWaitingTime(int waitingTime)
{
     WaitingArea::_waitingTime = waitingTime;
}

int WaitingArea::GetNextGoal()
{
     //TODO create global util for random numbers
     std::mt19937_64 gen(_rd());
     // probability of the next goals
     std::vector<double> weights;
     // states if at least one of the succeeding goals is open
     bool open = false;

     // get weights of open goals
     for (auto& nextGoal : _nextGoals) {
          if (_nextGoalsOpen[nextGoal.first]){
               weights.push_back(nextGoal.second);
               open = true;
          }else{
               weights.push_back(0.);
          }
     }

     if (open){
          // if at least one open goal, get random number regarding the
          // weights
          std::discrete_distribution<> distribution(weights.begin(), weights.end());
          int index = distribution(gen);

          auto iter = _nextGoals.begin();
          std::advance(iter, index);

          return iter->first;
     } else {
          // if no open goals found, return own id
          return _id;
     }
}

void WaitingArea::AddPed(int ped)
{
     _pedInside.insert(ped);
     if (_pedInside.size() >= _maxNumPed){
          _open = false;
     }
}

void WaitingArea::RemovePed(int ped)
{
     _pedInside.erase(ped);

     if (_pedInside.size() < _maxNumPed){
          _open = true;
     }
     if (_pedInside.size() < _minNumPed){
          _startTime = -1.;
     }
}

void WaitingArea::StartTimer(double time)
{
     _startTime = time;
}

bool WaitingArea::IsWaiting(double time, const Building* building)
{
     Transition* trans = nullptr;

     // check if transition id is set
     if (_transitionID >= 0){
          trans = building->GetTransition(_transitionID);
     }

     // if numPed > minNumPed and timer not started: start timer
     if ((_pedInside.size() >= _minNumPed) && (_startTime < 0. )){
          StartTimer(time);
     }

     // if no transition is used as waiting reference
     if (trans == nullptr){
          // if global timer is used
          if (_globalTimer){
               // if time since simulation is started is larger than waiting time: no more waiting
               if (time > _waitingTime){
                    return false;
               }
          }else{
               // if start time is set and waitingTime is exceeded: no more waitng
               if ((time > _startTime + _waitingTime) && (_startTime >= 0. )){
                    return false;
               }
          }
     }else{
          // if transition is open: no more waiting
          if (trans->IsOpen()){
               return false;
          }
     }

     // if none of the case above: waiting
     return true;
}

int WaitingArea::GetNumPed()
{
     return _pedInside.size();
}

int WaitingArea::GetTransitionID() const
{
     return _transitionID;
}

void WaitingArea::SetTransitionID(int transitionID)
{
     WaitingArea::_transitionID = transitionID;
}

const std::set<int>& WaitingArea::GetPedInside() const
{
     return _pedInside;
}