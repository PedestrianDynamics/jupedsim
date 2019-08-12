/**
 * \file        Pedestrian.cpp
 * \date        Sep 30, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/
#include "Pedestrian.h"

#include "Knowledge.h"

#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "geometry/WaitingArea.h"
#include "JPSfire/generic/FDSMeshStorage.h"

#include <cassert>

// initialize the static variables
double Pedestrian::_globalTime = 0.0;
int Pedestrian::_agentsCreated=1;
double Pedestrian::_minPremovementTime = FLT_MAX;
AgentColorMode Pedestrian::_colorMode=BY_VELOCITY;
std::vector<int> colors = {
     0,
     255,
     35,
     127,
     90,
};



Pedestrian::Pedestrian()
{
     _id = _agentsCreated;//default id
     _exitIndex = -1;
     _group = -1;
     _desiredFinalDestination = FINAL_DEST_OUT;
     _height = 170;
     _age = 30;
     _premovement = 0;
     _riskTolerance = 0;
     _gender = "female";
     _mass = 1;
     _tau = 0.5;
     _swayFreqA = 0.44;
     _swayFreqB = 0.35;
     _swayAmpA = -0.14;
     _swayAmpB = 0.21;
     _T = 1.0;
     _deltaT = 0.01;
     _ellipse = JEllipse();
     _V0 = Point(0,0);
     _V0UpStairs=0.6;
     _V0DownStairs=0.6;
     _EscalatorUpStairs=0.8;
     _EscalatorDownStairs=0.8;
     _V0IdleEscalatorUpStairs=0.6;
     _V0IdleEscalatorDownStairs=0.6;
     _roomCaption = "";
     _roomID = -1;
     _subRoomID = -1;
     _subRoomUID = -1;
     _oldRoomID = -1;
     _oldSubRoomID = -1;
     _lastE0 = Point(0,0);
     _navLine = nullptr;
     _mentalMap = std::map<int, int>();
     _destHistory = std::vector<int>();
     _trip = std::vector<int> ();
     _lastPosition = Point(0,0);
     _lastCellPosition = -1;
     _knownDoors.clear();
     _distToBlockade=0.0;
     _reroutingThreshold = 0.0; // new orientation after 10 seconds, value is incremented
     _timeBeforeRerouting = 0.0;
     _timeInJam = 0.0;
     _patienceTime = 5.0;// time after which the ped feels to be in jam
     _recordingTime = 20; //seconds
     //_lastPosition;
     //_lastVelocities
     _routingStrategy=ROUTING_GLOBAL_SHORTEST;
     _newOrientationDelay = 0; //0 seconds, in steps
     _updateRate = _deltaT;
     _turninAngle = 0.0;
     _reroutingEnabled = false;
     _tmpFirstOrientation = true;
     _newOrientationFlag = false;
     _router = nullptr;
     _building = nullptr;

     //_knownDoors = map<int, NavLineState>();

     _spotlight = false;
     _ticksInThisRoom = 0;

     _agentsCreated++;//increase the number of object created
     _FED_In = 0.0;
     _FED_Heat = 0.0;
     _WalkingSpeed = nullptr;
     _ToxicityAnalysis = nullptr;
}

//const shared_ptr<ToxicityAnalysis> &Pedestrian::getToxicityAnalysis() { return _ToxicityAnalysis; }

Pedestrian::Pedestrian(const StartDistribution& agentsParameters, Building& building)
:
     _group(agentsParameters.GetGroupId()),
     _desiredFinalDestination(agentsParameters.GetGoalId()),
     _height(agentsParameters.GetHeight()),
     _age(agentsParameters.GetAge()),
     _premovement(agentsParameters.GetPremovementTime()),
     _gender(agentsParameters.GetGender()),
     _roomCaption(""),
     _roomID(agentsParameters.GetRoomId()),
     _subRoomID(agentsParameters.GetSubroomID()),
     _subRoomUID(building.GetRoom(_roomID)->GetSubRoom(_subRoomID)->GetUID()),
     _lastPosition(),

     _patienceTime(agentsParameters.GetPatience()),
     _router(building.GetRoutingEngine()->GetRouter(agentsParameters.GetRouterId())),
     _building(&building),
     _ticksInThisRoom(0)
{
     _roomID = -1;
     _subRoomID = -1;
     _subRoomUID = -1;
     _oldRoomID = -1;
     _oldSubRoomID = -1;
     _exitIndex = -1;
     _id = _agentsCreated;//default id
     _mass = 1;
     _tau = 0.5;
     _T = 1.0;
     _swayFreqA = 0.44;
     _swayFreqB = 0.35;
     _swayAmpA = -0.14;
     _swayAmpB = 0.21;
     _newOrientationFlag = false;
     _newOrientationDelay = 0; //0 seconds, in steps
     _tmpFirstOrientation = true;
     _turninAngle = 0.0;
     _ellipse = JEllipse();
     _navLine = nullptr;
     _router = nullptr;
     _building = nullptr;
     _reroutingThreshold = 0.0; // new orientation after 10 seconds, value is incremented
     _timeBeforeRerouting = 0.0;
     _reroutingEnabled = false;
     _timeInJam = 0.0;
     _patienceTime = 5.0;// time after which the ped feels to be in jam
     _desiredFinalDestination = FINAL_DEST_OUT;
     _mentalMap = std::map<int, int>();
     _destHistory = std::vector<int>();
     _deltaT = 0.01;
     _updateRate = _deltaT;
     _V0 = Point(0,0);
     _lastPosition = Point(0,0);
     _lastCellPosition = -1;
     _recordingTime = 20; //seconds
     //_knownDoors = map<int, NavLineState>();
     _knownDoors.clear();
     _height = 170;
     _age = 30;
     _gender = "male";
     _trip = std::vector<int> ();
     _group = -1;
     _spotlight = false;
     _V0UpStairs=0.6;
     _V0DownStairs=0.6;
     _EscalatorUpStairs=0.8;
     _EscalatorDownStairs=0.8;
     _V0IdleEscalatorUpStairs=0.6;
     _V0IdleEscalatorDownStairs=0.6;
     _distToBlockade=0.0;
     _routingStrategy=ROUTING_GLOBAL_SHORTEST;
     _lastE0 = Point(0,0);
     _agentsCreated++;//increase the number of object created
     _FED_In = 0.0;
     _FED_Heat = 0.0;
     _ToxicityAnalysis = nullptr;
     _WalkingSpeed = nullptr;
}


Pedestrian::~Pedestrian()
{
     delete _navLine;
}


void Pedestrian::SetID(int i)
{
     _id = i;
     if(i<=0)
     {
          std::cerr<<">> Invalid pedestrians ID " << i<< std::endl;
          std::cerr<<">> Pedestrian ID should be > 0. Exit." << std::endl;
          exit(0);
     }
}

void Pedestrian::SetRoomID(int i, std::string roomCaption)
{
     _roomID = i;
     _roomCaption = roomCaption;
}

void Pedestrian::SetSubRoomID(int i)
{
     _subRoomID = i;
}

void Pedestrian::SetSubRoomUID(int i)
{
     _subRoomUID = i;
}

void Pedestrian::SetMass(double m)
{
     _mass = m;
}

void Pedestrian::SetTau(double tau)
{
     _tau = tau;
}

void Pedestrian::SetT(double T)
{
     _T = T;
}


void Pedestrian::SetSwayParameters(double freqA, double freqB, double ampA, double ampB) {
     _swayFreqA = freqA;
     _swayFreqB = freqB;
     _swayAmpA = ampA;
     _swayAmpB = ampB;
}

void Pedestrian::SetEllipse(const JEllipse& e)
{
     _ellipse = e;
}

void Pedestrian::SetExitIndex(int i)
{
     _exitIndex = i;
     //save that destination for that room
     _mentalMap[GetUniqueRoomID()] = i;
     //_destHistory.push_back(i);
}

void Pedestrian::SetExitLine(const NavLine* l)
{
     if(_navLine)
          delete _navLine;
     if(l) {
          _navLine = new NavLine(*l);
     }
}

void Pedestrian::SetPos(const Point& pos, bool initial)
{
     if((_globalTime>=_premovement) || (initial==true))
     {
          _ellipse.SetCenter(pos);
          //save the last values for the records
          _lastPositions.push(pos);
          unsigned int max_size = _recordingTime/_deltaT;
          if(_lastPositions.size() > max_size)
               _lastPositions.pop();
     }
}

void Pedestrian::SetCellPos(int cp)
{
     _lastCellPosition = cp;
}

void Pedestrian::SetV(const Point& v)
{
     if (_globalTime >= _premovement)
     {
          _ellipse.SetV(v);
          //save the last values for the records
          _lastVelocites.push(v);

          unsigned int max_size = _recordingTime / _deltaT;
          if (_lastVelocites.size() > max_size)
               _lastVelocites.pop();
     }
}

void Pedestrian::SetV0Norm(double v0, double v0UpStairs, double v0DownStairs, double escalatorUp, double escalatorDown, double v0IdleEscalatorUp, double v0IdleEscalatorDown)
{
     _ellipse.SetV0(v0);
     _V0DownStairs=v0DownStairs;
     _V0UpStairs=v0UpStairs;
     _EscalatorUpStairs=escalatorUp;
     _EscalatorDownStairs=escalatorDown;
     _V0IdleEscalatorUpStairs=v0IdleEscalatorUp;
     _V0IdleEscalatorDownStairs=v0IdleEscalatorDown;
}


void Pedestrian::SetFEDIn(double FED_In)
{
     _FED_In = FED_In;
}
double Pedestrian::GetFEDIn()
{
     return _FED_In;
}

void Pedestrian::SetFEDHeat(double FED_Heat)
{
     _FED_Heat = FED_Heat;
}
double Pedestrian::GetFEDHeat()
{
     return _FED_Heat;
}


void Pedestrian::Setdt(double dt)
{
     _deltaT = dt;
}
double Pedestrian::Getdt()
{
     return _deltaT;
}

void Pedestrian::SetTrip(const std::vector<int>& trip)
{
     _trip = trip;
}


int Pedestrian::GetID() const
{
     return _id;
}

double Pedestrian::GetUpdateRate() const
{
      return _updateRate;
}
int Pedestrian::GetRoomID() const
{
     return _roomID;
}

int Pedestrian::GetSubRoomID() const
{
     return _subRoomID;
}

int Pedestrian::GetSubRoomUID() const
{
     return _subRoomUID;
}

double Pedestrian::GetMass() const
{
     return _mass;
}

double Pedestrian::GetTau() const
{
     return _tau;
}


double Pedestrian::GetSwayFreqA() const {
     return _swayFreqA;
}

double Pedestrian::GetSwayFreqB() const {
     return _swayFreqB;
}

double Pedestrian::GetSwayAmpA() const {
     return _swayAmpA;
}

double Pedestrian::GetSwayAmpB() const {
     return _swayAmpB;
}

double Pedestrian::GetT() const
{
     return _T;
}

const JEllipse& Pedestrian::GetEllipse() const
{
     return _ellipse;
}

int Pedestrian::GetExitIndex() const
{
     return _exitIndex;
}

NavLine* Pedestrian::GetExitLine() const
{
     return _navLine;
}

const std::vector<int>& Pedestrian::GetTrip() const
{
     return _trip;
}

// return the unique subroom Identifier

int Pedestrian::GetUniqueRoomID() const
{
     return _roomID * 1000 + _subRoomID;
}

RoutingStrategy Pedestrian::GetRoutingStrategy() const
{
     return _routingStrategy;
}

// returns the exit Id corresponding to the
// unique subroom identifier

int Pedestrian::GetNextDestination()
{
     if (_mentalMap.count(GetUniqueRoomID()) == 0) {
          return -1;
     } else {
          return _mentalMap[GetUniqueRoomID()];
     }
}

Point Pedestrian::GetLastE0() const
{
      return _lastE0;
}
void Pedestrian::SetLastE0(Point E0)
{
      _lastE0 = E0;
}

int Pedestrian::GetLastDestination()
{
     if(_destHistory.size() == 0)
          return -1;
     else
          return _destHistory.back();

}

bool Pedestrian::ChangedSubRoom()
{
     if(_oldRoomID != GetRoomID() || _oldSubRoomID != GetSubRoomID()) {
          _oldRoomID = GetRoomID();
          _oldSubRoomID = GetSubRoomID();
          return true;
     }
     return false;
}

void Pedestrian::ClearMentalMap()
{
     _mentalMap.clear();
     _exitIndex = -1;
}

void Pedestrian::AddKnownClosedDoor(int door, double ttime, bool state, double quality, double latency)
{
     if(ttime==0) ttime=_globalTime;
     _knownDoors[door].SetState(door,state,ttime,quality, latency);
}

void Pedestrian::ClearKnowledge()
{
     _knownDoors.clear();
}

std::map<int, Knowledge>&  Pedestrian::GetKnownledge()
{
     return _knownDoors;
}

const std::vector<int>& Pedestrian::GetLastDestinations() const
{
     return _destHistory;
}

const std::string Pedestrian::GetKnowledgeAsString() const
{
     std::string key="";
     for(auto&& knowledge:_knownDoors)
     {
          //skip low quality information
          if(knowledge.second.GetQuality()<0.2) continue;

          int door=knowledge.first;
          if(key.empty())
               key.append(std::to_string(door));
          else
               key.append(":"+std::to_string(door));
     }
     return key;
}

const Point& Pedestrian::GetPos() const
{
     return _ellipse.GetCenter();
}

int Pedestrian::GetCellPos() const
{
     return _lastCellPosition;
}

const Point& Pedestrian::GetV() const
{
     return _ellipse.GetV();
}

const Point& Pedestrian::GetV0() const
{
     return _V0;
}


double Pedestrian::GetV0Norm() const
{
     // @todo: we need to know the difference of the ped_elevation to the old_nav_elevation, and use this in the function f.
     //detect the walking direction based on the elevation
     SubRoom* sub=_building->GetRoom(_roomID)->GetSubRoom(_subRoomID);
     double ped_elevation = sub->GetElevation(_ellipse.GetCenter());
     if (_navLine ==nullptr)
     {
          //printf("Error: ped %d has no navline\n", _id);
          //exit(EXIT_FAILURE);
          return std::max(0.,_ellipse.GetV0());
     }
     const Point& target = _navLine->GetCentre();
     double nav_elevation = sub->GetElevation(target);
     double delta = nav_elevation - ped_elevation;
     double walking_speed = 0;
//---------------------------------------------------
     //-----------------------------------------


     // const Point& pos = GetPos();
     // double distanceToTarget = (target-pos).Norm();
     // double iniDistanceToTarget = (target-_lastPositions.front()).Norm();

     // printf("delta = %f, nav_elev = %f, ped_elev= %f, ped=[%f %f] targe=[%f, %f]\n", delta, nav_elevation, ped_elevation, pos._x, pos._y, target._x, target._y);
     // fprintf(stderr, "%f  %f front [%f, %f] nav [%f, %f] dist=%f, iniDist=%f\n", delta, ped_elevation, _lastPositions.front()._x, _lastPositions.front()._y, _navLine->GetCentre()._x, _navLine->GetCentre()._y, distanceToTarget, iniDistanceToTarget);



     // we are walking on an even plane
     //TODO: move _ellipse.GetV0() to _V0Plane
     if(fabs(delta)<J_EPS){
         //FIXME std::normal_distribution generated V0's that are very small or even < 0
         //assume absolute v_min according to Weidmann
         walking_speed = std::max(0.,_ellipse.GetV0());

         //fprintf(stderr, "%f  %f  %f  %f\n", pos._x, pos._y, ped_elevation, walking_speed);
     }
      // we are walking downstairs
     else{
           double c = 15.0;
           // c should be chosen so that the func grows fast (but smooth) from 0 to 1
           // However we have to pay attention to tau. The velocity adaptation
           // from v to v0 in the driven force takes tau time.
           double f, g; // f in [0, 1]
           if(delta<0)
           {
                 double maxSubElevation = sub->GetMaxElevation();
                 double stairLength = maxSubElevation - sub->GetMinElevation();
                 double stairInclination = acos(sub->GetCosAngleWithHorizontal());
                 f = 2.0/(1+exp(-c*stairInclination*(maxSubElevation - ped_elevation)*(maxSubElevation - ped_elevation))) - 1;
                 g = 2.0/(1+exp(-c*stairInclination*(maxSubElevation - ped_elevation - stairLength)*(maxSubElevation - ped_elevation - stairLength))) - 1;
                 double speed_down = _V0DownStairs;
                 if(sub->GetType() == "escalator"){
                       speed_down = _EscalatorDownStairs;
                 }
                 else if(sub->GetType() == "idle_escalator"){
                       speed_down = _V0IdleEscalatorDownStairs;
                 }
                 // fprintf(stderr, "%f  %f  %f  %f\n", pos._x, pos._y, ped_elevation, (1-f)*_ellipse.GetV0() + f*speed_down);
                 // fprintf(stderr, "%f  %f   %f  %f %f\n", _globalTime, _ellipse.GetV0(), (1-f*g)*_ellipse.GetV0() + f*g*speed_down, GetV().Norm(), ped_elevation);
                 //                  // getc(stdin);

                 walking_speed =(1-f*g)*_ellipse.GetV0() + f*g*speed_down;

                     // printf("%f  DOWN max_e=%f,  z=%f, f=%f, v0=%f, v0d=%f, ret=%f\n", _globalTime, maxSubElevation, ped_elevation, f, _ellipse.GetV0(), _V0DownStairs, (1-f*g)*_ellipse.GetV0() + f*g*speed_down);

           }
           //we are walking upstairs
           else
           {
                 double minSubElevation = sub->GetMinElevation();
                 double stairHeight = sub->GetMaxElevation() - minSubElevation;
                 double stairInclination = acos(sub->GetCosAngleWithHorizontal());
                 // double stairHorinzontalLength =  stairHeight / sub->GetTanAngleWithHorizontal();
                 f = 2.0/(1+exp(-c*stairInclination*(minSubElevation - ped_elevation)*(minSubElevation - ped_elevation))) - 1;
                 g = 2.0/(1+exp(-c*stairInclination*(ped_elevation - minSubElevation - stairHeight)*(ped_elevation - minSubElevation - stairHeight))) - 1;

                 //FIXME std::normal_distribution generated V0's that are very small or even < 0
                 double speed_up = std::max(0.0, _V0UpStairs);

                 if(sub->GetType() == "escalator"){
                       speed_up = _EscalatorUpStairs;
                 }
                 else if(sub->GetType() == "idle_escalator"){
                       speed_up = _V0IdleEscalatorUpStairs;
                 }
                 // if(_id==209){
                       // printf("%f UP min_e=%f, z=%f, f=%f, g=%f, v0=%f, speed_up=%f, ret=%f, v=%f\n", _globalTime , minSubElevation, ped_elevation, f, g, _ellipse.GetV0(), speed_up, (1-f*g)*_ellipse.GetV0() + f*g*speed_up, GetV().Norm());
                 // printf("minElevation = %f, maxELevation = %f, ped_elevation = %f, stairHeight = %f, stairLength = %f, angle = %.2f pos=(%f, %f)\n", minSubElevation, sub->GetMaxElevation(), ped_elevation ,stairHeight, stairHorinzontalLength, stairInclination, pos._x, pos._y);
                       // getc(stdin);
                 // fprintf(stderr, "%f  %f   %f  %f %f %f\n", _globalTime, _ellipse.GetV0(), (1-f*g)*_ellipse.GetV0() + f*g*speed_up, GetV().Norm(), ped_elevation,  stairInclination*180./3.14159265);
                 // }
                 // getc(stdin);

                 walking_speed = (1-f*g)*_ellipse.GetV0() + f*g*speed_up;
           }
     }

     //IF execution of WalkingInSmoke depending on JPSfire section in INI file
     #ifdef JPSFIRE
     if(_WalkingSpeed && _WalkingSpeed->ReduceWalkingSpeed()) {
         walking_speed = _WalkingSpeed->WalkingInSmoke(this, walking_speed);
     }
     #endif
     //WHERE should the call to that routine be placed properly?
     //only executed every 3 seconds
     // fprintf(stderr, "%f\n", walking_speed);

     return walking_speed;
     // orthogonal projection on the stair
     //return _ellipse.GetV0()*_building->GetRoom(_roomID)->GetSubRoom(_subRoomID)->GetCosAngleWithHorizontal();
}
#ifdef JPSFIRE
void Pedestrian::ConductToxicityAnalysis()
{
    if(_ToxicityAnalysis->ConductToxicityAnalysis()){
             _ToxicityAnalysis->HazardAnalysis(this);
        }
}
#endif
// get axis in the walking direction
double Pedestrian::GetLargerAxis() const
{
     return _ellipse.GetEA();
}
// get axis in the shoulder direction = orthogonal to the walking direction
double Pedestrian::GetSmallerAxis() const
{
     return _ellipse.GetEB();
}

void Pedestrian::SetPhiPed()
{
     double cosPhi, sinPhi;
     double vx = GetV()._x;
     double vy = GetV()._y;

     if (fabs(vx) > J_EPS || fabs(vy) > J_EPS) {
          double normv = sqrt(vx * vx + vy * vy);
          cosPhi = vx / normv;
          sinPhi = vy / normv;
     } else {
          cosPhi = GetEllipse().GetCosPhi();
          sinPhi = GetEllipse().GetSinPhi();
     }
     _ellipse.SetCosPhi(cosPhi);
     _ellipse.SetSinPhi(sinPhi);
}


void Pedestrian::InitV0(const Point& target)
{

#define DEBUG 0
     const Point& pos = GetPos();
     Point delta = target - pos;

     _V0 = delta.Normalized();

#if DEBUG
     printf("Ped=%d : _v0=[%f, %f] delta=[%f, %f], pos=[%f, %f], target=[%f, %f]\n", _id, _V0._x, _V0._y, delta._x, delta._y, pos._x, pos._y, target._x, target._y);
#endif
}


const Point& Pedestrian::GetV0(const Point& target)
{

#define DEBUGV0 1
     const Point& pos = GetPos();
     Point delta = target - pos;
     Point new_v0;
     double t;
     // Molification around the targets makes little sense
     //new_v0 = delta.NormalizedMolified();
     new_v0 = delta.Normalized();
     // -------------------------------------- Handover new target
     t = _newOrientationDelay++ *_deltaT/(1.0+100* _distToBlockade);

     _V0 = _V0 + (new_v0 - _V0)*( 1 - exp(-t/_tau) );

#if DEBUGV0
     if(0){
          printf("=====\nGoal Line=[%f, %f]-[%f, %f]\n", _navLine->GetPoint1()._x, _navLine->GetPoint1()._y, _navLine->GetPoint2()._x, _navLine->GetPoint2()._y);
          printf("Ped=%d, sub=%d, room=%d pos=[%f, %f], target=[%f, %f]\n", _id, _subRoomID, _roomID, pos._x, pos._y, target._x, target._y);
          printf("Ped=%d : BEFORE new_v0=%f %f norm = %f\n", _id, new_v0._x, new_v0._y, new_v0.Norm());
          printf("ped=%d: t=%f, _newOrientationFlag=%d, neworientationDelay=%d, _DistToBlockade=%f\n", _id,t, _newOrientationFlag, _newOrientationDelay, _distToBlockade);
          printf("_v0=[%f, %f] norm = %f\n=====\n", _V0._x, _V0._y, _V0.Norm());
          getc(stdin);
     }

#endif
     // --------------------------------------

     return _V0;
}

double Pedestrian::GetTimeInJam() const
{
     return _timeInJam;
}

// set the new orientation flag
// to delay sharp turn
// TODO: maybe combine this with SetExitLine

// void Pedestrian::SetSmoothTurning(bool smt)
// {
//      //ignoring first turn
//      if (_tmpFirstOrientation) {
//           _tmpFirstOrientation = false;
//      } else {
//           if (_newOrientationDelay <= 0)// in the case the pedestrian is still rotating
//                _newOrientationFlag = smt;
//      }

// }

void Pedestrian::SetSmoothTurning()
{
     _newOrientationDelay = 0;
}


bool Pedestrian::IsFeelingLikeInJam()
{
     //return true;
     return (_patienceTime < _timeInJam);
}

//reduce the felt time in Jam by half
void Pedestrian::ResetTimeInJam()
{
     _timeInJam = 0.0;
     //_timeInJam /= 2.0;
}

void Pedestrian::UpdateTimeInJam()
{
     _timeInJam += _deltaT;
}

//TODO: magic
void Pedestrian::UpdateJamData()
{
     if(GetV().NormSquare() < 0.25*GetV0().NormSquare()) {
          _timeInJam += _deltaT;
     } else {
          _timeInJam /= 2.0;
     }
}

void Pedestrian::UpdateReroutingTime()
{
     _timeBeforeRerouting -= _deltaT;
}

void Pedestrian::RerouteIn(double time)
{
     _reroutingEnabled = true;
     _timeBeforeRerouting = time;
}

bool Pedestrian::IsReadyForRerouting()
{
     return(_reroutingEnabled &&(_timeBeforeRerouting <= 0.0));
}


double Pedestrian::GetReroutingTime()
{
     return _timeBeforeRerouting;
}

bool  Pedestrian::GetNewOrientationFlag()
{
     return _newOrientationFlag;
}

void Pedestrian::SetDistToBlockade(double dist)
{
     _distToBlockade = dist;
}
double Pedestrian::GetDistToBlockade()
{
     return _distToBlockade;
}


void Pedestrian::SetNewOrientationFlag(bool flag)
{
     _newOrientationFlag=flag;
}

double Pedestrian::GetAge() const
{
     return _age;
}

void Pedestrian::SetAge(double age)
{
     _age = age;
}

std::string Pedestrian::GetGender() const
{
     return _gender;
}

void Pedestrian::SetGender(std::string gender)
{
     _gender = gender;
}

double Pedestrian::GetHeight() const
{
     return _height;
}

int Pedestrian::GetGroup() const
{
     return _group;
}

void Pedestrian::SetGroup(int group)
{
     _group = group;
}

void Pedestrian::SetHeight(double height)
{
     _height = height;
}

void Pedestrian::ResetRerouting()
{
     _reroutingEnabled = false;
     _timeBeforeRerouting = -1.00;
}

void Pedestrian::SetRecordingTime(double timeInSec)
{
     _recordingTime = timeInSec;
}

double Pedestrian::GetRecordingTime() const
{
     return _recordingTime;
}

double Pedestrian::GetMeanVelOverRecTime() const {
     //just few position were saved
     if (_lastPositions.size()<2) return _ellipse.GetV().Norm();
     return fabs ( (_lastPositions.back()-_lastPositions.front()).Norm() / _recordingTime );
}

double Pedestrian::GetDistanceToNextTarget() const
{
     return (_navLine->DistTo(GetPos()));
}

void Pedestrian::SetFinalDestination(int finale)
{
     _desiredFinalDestination = finale;
}

int Pedestrian::GetFinalDestination() const
{
     return _desiredFinalDestination;
}

///@deprecated
//void Pedestrian::WritePath(ofstream& file, Building* building)
//{
//     map<int, int>::iterator iter;
//
//     if(building) {
//          for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
//               file<<building->GetAllRooms()[iter->first/1000]->GetCaption()<<" "<<iter->second<<endl;
//          }
//     } else {
//          for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
//               file<<iter->first/1000<<" "<<iter->second<<endl;
//          }
//     }
//}

std::string Pedestrian::GetPath()
{
     std::map<int, int>::iterator iter;
     std::string path;

     for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
          std::stringstream ss;//create a stringstream
          ss << iter->first/1000<<":"<<iter->second<<">"; //@todo:ar.graf: has this to do with roomNr*1000+subroom and is now wrong?
          path.append(ss.str());
     }
     return path;
}

void Pedestrian::Dump(int ID, int pa) const
{

     if (ID != _id) return;

     printf("------> ped %d <-------\n", _id);

     switch (pa) {

     case 0:
          printf(">> Room/Subroom [%d / %d]\n", _roomID, _subRoomID);
          printf(">> Destination [ %d ]\n", _exitIndex);
          printf(">> Final Destination [ %d ]\n", _desiredFinalDestination);
          printf(">> Position [%0.2f, %0.2f]\n", GetPos()._x, GetPos()._y);
          printf(">> V0       [%0.2f, %0.2f]  Norm = [%0.2f]\n", _V0._x, _V0._y, GetV0Norm());
          printf(">> Velocity [%0.2f, %0.2f]  Norm = [%0.2f]\n", GetV()._x, GetV()._y, GetV().Norm());
          if(GetExitLine()) {
               printf(">> ExitLine: (%0.2f, %0.2f) -- (%0.2f, %0.2f)\n", GetExitLine()->GetPoint1()._x, GetExitLine()->GetPoint1()._y,
                         GetExitLine()->GetPoint2()._x, GetExitLine()->GetPoint2()._y);
               printf(">> dist: %f\n", GetExitLine()->DistTo(GetPos()));
          }
          printf(">> smooth rotating: %s \n", (_newOrientationDelay > 0) ? "yes" : "no");
          printf(">> mental map");
          for(auto&& item: _mentalMap)
               printf("\t room / destination  [%d, %d]\n", item.first, item.second);
          for(auto&& item:_knownDoors)
               printf(">> %s \n",item.second.Dump().c_str());
          printf(">> Knowledge: %s \n",GetKnowledgeAsString().c_str());
          printf(">> Color: %d \n",GetColor());
          break;

     case 1:
          printf(">> Position [%f, %f]\n", GetPos()._x, GetPos()._y);
          break;

     case 2:
          printf(">> Velocity [%f, %f]\n", GetV()._x, GetV()._y);
          break;

     case 3:
          printf(">> V0       [%f, %f]  Norm = [%f]\n", _V0._x, _V0._y, GetV0Norm());
          break;

     case 4:
          printf(">> Room/Subroom [%d / %d]\n", _roomID, _subRoomID);
          break;

     case 5:
          printf(">> Destination [ %d ]\n", _exitIndex);
          break;

     case 6:  //Mental Map
          printf(">> mental map");
          for(auto&& item: _mentalMap)
               printf("\t room / destination  [%d, %d]", item.first, item.second);
          break;

     case 7:
          printf(">> knowledge\n");
          for(auto&& item:_knownDoors)
               printf("\t door [%d] closed since [%.2f] sec\n",item.first, _globalTime-item.second.GetTime());
          break;

     default:
          break;

     }
     fflush(stdout);
     getc(stdin);
}

void Pedestrian::RecordActualPosition()
{
     _lastPosition = GetPos();
}

double Pedestrian::GetDistanceSinceLastRecord()
{
     return (_lastPosition-GetPos()).Norm();
}

double Pedestrian::GetGlobalTime()
{
     return _globalTime;
}

void Pedestrian::SetRouter(Router* router)
{
     _router=router;
     _routingStrategy=router->GetStrategy();
}

Router* Pedestrian::GetRouter() const
{
     return _router;
}

int Pedestrian::FindRoute()
{
     if( ! _router) {
          Log->Write("ERROR:\t one or more routers does not exist! Check your router_ids");
          return -1;
     }
     //bool isinsub = (_building->GetAllRooms().at(this->GetRoomID())->GetSubRoom(this->GetSubRoomID())->IsInSubRoom(this));
     return _router->FindExit(this);
}

double Pedestrian::GetElevation() const
{
     return _building->GetRoom(_roomID)->GetSubRoom(_subRoomID)->GetElevation(GetPos());
}

void Pedestrian::SetGlobalTime(double time)
{
     _globalTime = time;
}

double Pedestrian::GetPatienceTime() const
{
     return _patienceTime;
}

void Pedestrian::SetPatienceTime(double patienceTime)
{
     _patienceTime = patienceTime;
}

void Pedestrian::SetPremovementTime(double pretime)
{
      if(pretime < _minPremovementTime)
            _minPremovementTime = pretime;

     _premovement=pretime;
}

double Pedestrian::GetMinPremovementTime()
{
     return _minPremovementTime;
}

double Pedestrian::GetPremovementTime()
{
     return _premovement;
}

void Pedestrian::SetRiskTolerance(double tol)
{
     if (tol>1) tol=1;
     if(tol<0) tol=0;
     _riskTolerance=tol;
}

double Pedestrian::GetRiskTolerance() const
{
     return _riskTolerance;
}

const Building* Pedestrian::GetBuilding()
{
     return _building;
}

void Pedestrian::SetBuilding(Building* building)
{
     _building = building;
}

void Pedestrian::SetWalkingSpeed(std::shared_ptr<WalkingSpeed> walkingSpeed)
{
    _WalkingSpeed = walkingSpeed;
}

void Pedestrian::SetTox(std::shared_ptr<ToxicityAnalysis>toxicityAnalysis)
{
    _ToxicityAnalysis = toxicityAnalysis;
}

void Pedestrian::SetSpotlight(bool spotlight)
{
     _spotlight = spotlight;
}

void Pedestrian::SetColorMode(AgentColorMode mode)
{
     _colorMode=mode;
}

int Pedestrian::GetAgentsCreated()
{
     return _agentsCreated;
}

int Pedestrian::GetColor() const
{
     //default color is by velocity
     std::string key;

     switch (_colorMode)
     {
     case BY_SPOTLIGHT:
     {
          if (_spotlight==false)
               return -1;
          break;
     }

     case BY_VELOCITY:
     {
          int color = -1;
          double v0 = GetV0Norm();
          if (v0 != 0.0) {
               double v = GetV().Norm();
               color = (int) (v / v0 * 255);
          }
          return color;
     }
     break;

     // Hash the knowledge represented as String
     case BY_KNOWLEDGE:
     {
          key=GetKnowledgeAsString();
          if(key.empty()) return -1;
     }
     break;

     case BY_ROUTER:
     case BY_ROUTE:
     {
          key = std::to_string(_routingStrategy);
     }
     break;

     case BY_GROUP:
     {
          key = std::to_string(_group); // @todo find a better solution to get
                                        // colors clearly distinguishable form
                                        // each other
          return(colors[_group%colors.size()]);
     }
     break;

     case BY_FINAL_GOAL:
     {
          key=std::to_string(_desiredFinalDestination);
     }
     break;

     case BY_INTERMEDIATE_GOAL:
     {
          key=std::to_string(_exitIndex);
     }
     break;

     default:
          break;
     }

     std::hash<std::string> hash_fn;
     return  hash_fn(key) % 255;
}


bool Pedestrian::Relocate(std::function<void(const Pedestrian&)> flowupdater) {

     auto allRooms = _building->GetAllRooms();
     bool status = false;
     for (auto&it_room : allRooms)
     {
          auto& room = it_room.second;
          auto subrooms = room->GetAllSubRooms();
          std::map<int, std::shared_ptr<SubRoom> >::iterator sub =
                  std::find_if(subrooms.begin(), subrooms.end(), [&] (std::pair<int, std::shared_ptr<SubRoom>> iterator) {
                      return ((iterator.second->IsDirectlyConnectedWith(allRooms[_roomID]->GetSubRoom(_subRoomID))) && iterator.second->IsInSubRoom(this));
                  });
          if(sub != subrooms.end()) {
               flowupdater(*this); //@todo: ar.graf : this call should move into a critical region? check plz
               ClearMentalMap(); // reset the destination
               const int oldRoomID = _roomID;
               SetRoomID(room->GetID(), room->GetCaption());
               SetSubRoomID(sub->second->GetSubRoomID());
               SetSubRoomUID(sub->second->GetUID());
               _router->FindExit(this);
               if(oldRoomID != room->GetID()){
                      //the agent left the old room
                      //actualize the egress time for that room
#pragma omp critical(SetEgressTime)
                     allRooms.at(oldRoomID)->SetEgressTime(GetGlobalTime()); //set Egresstime to old room //@todo: ar.graf : GetRoomID() yields NEW room
               }
               status = true;
               break;
          }
     }
     return status;
}

int Pedestrian::GetLastGoalID() const
{
     return _lastGoalID;
}

bool Pedestrian::IsInsideGoal() const
{
     return _insideGoal;
}

bool Pedestrian::IsInsideWaitingAreaWaiting() const
{
     if (_insideGoal){
          auto itr = _building->GetAllGoals().find(_desiredFinalDestination);
          if (itr != _building->GetAllGoals().end()){
               Goal* goal = itr->second;
               if (WaitingArea* wa = dynamic_cast<WaitingArea*>(goal)){
                    return wa->IsWaiting(Pedestrian::GetGlobalTime(), _building);
               }
          }
     }
     return false;
}

void Pedestrian::EnterGoal()
{
     _insideGoal = true;
     _lastGoalID = _desiredFinalDestination;
}

void Pedestrian::LeaveGoal()
{
     _insideGoal = false;
}

bool Pedestrian::IsWaiting() const
{
     return _waiting;
}

const Point& Pedestrian::GetWaitingPos() const
{
     return _waitingPos;
}

void Pedestrian::SetWaitingPos(const Point& waitingPos)
{
     _waitingPos = waitingPos;
}


void Pedestrian::StartWaiting()
{
     _waitingPos._x = std::numeric_limits<double>::max();
     _waitingPos._y = std::numeric_limits<double>::max();
     _waiting = true;
}

void Pedestrian::EndWaiting()
{
     _waiting = false;
}

bool Pedestrian::IsOutside()
{
     Room* room = _building->GetRoom(_roomID);

     if (room->GetCaption() == "outside"){
          return true;
     }

     for (auto& itr : room->GetAllSubRooms()){
          auto subRoom =itr.second;

          if (subRoom->IsInSubRoom(this)){
               return false;
          }
     }
     return true;
}