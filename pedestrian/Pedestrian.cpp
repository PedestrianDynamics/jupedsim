/**
 * \file        Pedestrian.cpp
 * \date        Sep 30, 2010
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#include <cassert>
#include "../geometry/Building.h"
#include "../geometry/NavLine.h"
#include "../routing/Router.h"
#include "../geometry/SubRoom.h"
#include "../IO/OutputHandler.h"
#include "Knowledge.h"
#include "Pedestrian.h"

using namespace std;

// initialize the static variables
double Pedestrian::_globalTime = 0.0;
AgentColorMode Pedestrian::_colorMode=BY_VELOCITY;

Pedestrian::Pedestrian()
{
     _roomID = -1;
     _subRoomID = -1;
     _oldRoomID = -1;
     _oldSubRoomID = -1;
     _exitIndex = -1;
     _id = 0;
     _mass = 1;
     _tau = 0.5;
     _newOrientationFlag = false;
     _newEventFlag = false;
     _newOrientationDelay = 0; //0 seconds, in steps
     _tmpFirstOrientation = true;
     _updateRate = 0;
     _turninAngle = 0.0;
     _ellipse = JEllipse();
     _navLine = new NavLine();
     _router = NULL;
     _building = NULL;
     _reroutingThreshold = 0.0; // new orientation after 10 seconds, value is incremented
     _timeBeforeRerouting = 0.0;
     _reroutingEnabled = false;
     _timeInJam = 0.0;
     _patienceTime = 5.0;// time after which the ped feels to be in jam
     _desiredFinalDestination = FINAL_DEST_OUT;
     _mentalMap = map<int, int>();
     _destHistory = vector<int>();
     _deltaT = 0.01;
     _V0 = Point(0,0);
     _lastPosition = Point(0,0);
     _lastCellPosition = -1;
     _recordingTime = 20; //seconds
     //     _knownDoors = map<int, NavLineState>();
     _knownDoors.clear();
     _height = 170;
     _age = 30;
     _gender = "male";
     _trip = vector<int> ();
     _group = -1;
     _spotlight = false;
     _V0UpStairs=0.0;
     _V0DownStairs=0.0;
     _distToBlockade=0.0;
     _routingStrategy=ROUTING_GLOBAL_SHORTEST;
}


Pedestrian::~Pedestrian()
{
     if(_navLine) delete _navLine;
}


void Pedestrian::SetID(int i)
{
     _id = i;
}

void Pedestrian::SetRoomID(int i, string roomCaption)
{
     _roomID = i;
     _roomCaption = roomCaption;
}

void Pedestrian::SetSubRoomID(int i)
{
     _subRoomID = i;
}

void Pedestrian::SetMass(double m)
{
     _mass = m;
}

void Pedestrian::SetTau(double tau)
{
     _tau = tau;
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
     _destHistory.push_back(i);
}

void Pedestrian::SetExitLine(const NavLine* l)
{
     //_navLine = l;
     _navLine->SetPoint1(l->GetPoint1());
     _navLine->SetPoint2(l->GetPoint2());
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

void Pedestrian::SetV0Norm(double v0,double v0UpStairs, double v0DownStairs)
{
     _ellipse.SetV0(v0);
     _V0DownStairs=v0DownStairs;
     _V0UpStairs=v0UpStairs;
}

void Pedestrian::Setdt(double dt)
{
     _deltaT = dt;
}
double Pedestrian::Getdt()
{
     return _deltaT;
}

void Pedestrian::SetTrip(const vector<int>& trip)
{
     _trip = trip;
}


int Pedestrian::GetID() const
{
     return _id;
}

int Pedestrian::GetRoomID() const
{
     return _roomID;
}

int Pedestrian::GetSubRoomID() const
{
     return _subRoomID;
}

double Pedestrian::GetMass() const
{
     return _mass;
}

double Pedestrian::GetTau() const
{
     return _tau;
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

const vector<int>& Pedestrian::GetTrip() const
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

int Pedestrian::GetDestinationCount()
{
     return _destHistory.size();
}

void Pedestrian::ClearMentalMap()
{
     _mentalMap.clear();
     _exitIndex = -1;
}

void Pedestrian::AddKnownClosedDoor(int door, double time)
{
     if(time==0) time=_globalTime;
     _knownDoors[door].SetState(door,true,time);
}

void Pedestrian::ClearKnowledge()
{
     _knownDoors.clear();
}

const map<int, Knowledge>&  Pedestrian::GetKnownledge() const
{
     return _knownDoors;
}

const std::string Pedestrian::GetKnowledgeAsString() const
{
     string key="";
     for(auto&& knowledge:_knownDoors)
     {
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
     //detect the walking direction based on the elevation
     SubRoom* sub=_building->GetRoom(_roomID)->GetSubRoom(_subRoomID);
     double delta = sub->GetElevation(_navLine->GetCentre())-
               sub->GetElevation(_ellipse.GetCenter());

     //TODO: The stairs should be detect before (1m in front)
     //and the velocity reduced accordingly

     // we are walking on an even plane
     //TODO: move _ellipse.GetV0() to _V0Plane
     if(fabs(delta)<J_EPS)
          return _ellipse.GetV0();

     // we are walking downstairs
     if(delta<0)
     {
          return _V0DownStairs;
     }
     //we are walking upstairs
     else
     {
          return _V0UpStairs;
     }
     // orthogonal projection on the stair
     //return _ellipse.GetV0()*_building->GetRoom(_roomID)->GetSubRoom(_subRoomID)->GetCosAngleWithHorizontal();
}
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
     double vx = GetV().GetX();
     double vy = GetV().GetY();

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
     printf("Ped=%d : _v0=[%f, %f] \n", _id, _v0.GetX(), _V0.GetY());
#endif
}


const Point& Pedestrian::GetV0(const Point& target)
{

#define DEBUG 0
     const Point& pos = GetPos();
     Point delta = target - pos;
     Point new_v0;
     double t;
     // Molification around the targets makes little sense
     //new_v0 = delta.NormalizedMolified();
     new_v0 = delta.Normalized();
     // -------------------------------------- Handover new target
     t = _newOrientationDelay++ *_deltaT/(1.0+1000* _distToBlockade); 

     _V0 = _V0 + (new_v0 - _V0)*( 1 - exp(-t/_tau) );
#if DEBUG
     if(_id==24){
          printf("Goal Line=[%f, %f]-[%f, %f]\n", _navLine->GetPoint1().GetX(), _navLine->GetPoint1().GetY(), _navLine->GetPoint2().GetX(), _navLine->GetPoint2().GetY());
          printf("Ped=%d, sub=%d, room=%d pos=[%f, %f], target=[%f, %f]\n", _id, _subRoomID, _roomID, pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
          printf("Ped=%d : BEFORE new_v0=%f %f norm = %f\n", _id, new_v0.GetX(), new_v0.GetY(), new_v0.Norm());
          printf("ped=%d: t=%f, _newOrientationFlag=%d, neworientationDelay=%d, _DistToBlockade=%f\n", _id,t, _newOrientationFlag, _newOrientationDelay, _distToBlockade);
          printf("_v0=[%f, %f] norm = %f\n", _V0.GetX(), _V0.GetY(), _V0.Norm());
     }
     // getc(stdin);
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

bool Pedestrian::GetNewEventFlag()
{
     return _newEventFlag;
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

void Pedestrian::SetNewEventFlag(bool flag)
{
     _newEventFlag=flag;
}

double Pedestrian::GetAge() const
{
     return _age;
}

void Pedestrian::SetAge(double age)
{
     _age = age;
}

string Pedestrian::GetGender() const
{
     return _gender;
}

void Pedestrian::SetGender(string gender)
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

void Pedestrian::SetFinalDestination(int final)
{
     _desiredFinalDestination = final;
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

string Pedestrian::GetPath()
{
     map<int, int>::iterator iter;
     string path;

     for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
          stringstream ss;//create a stringstream
          ss << iter->first/1000<<":"<<iter->second<<">";
          path.append(ss.str());
     }
     return path;
}

void Pedestrian::Dump(int ID, int pa)
{

     if (ID != _id) return;

     printf("------> ped %d <-------\n", _id);

     switch (pa) {

     case 0:
          printf(">> Room/Subroom [%d / %d]\n", _roomID, _subRoomID);
          printf(">> Destination [ %d ]\n", _exitIndex);
          printf(">> Final Destination [ %d ]\n", _desiredFinalDestination);
          printf(">> Position [%0.2f, %0.2f]\n", GetPos().GetX(), GetPos().GetY());
          printf(">> V0       [%0.2f, %0.2f]  Norm = [%0.2f]\n", _V0.GetX(), _V0.GetY(), GetV0Norm());
          printf(">> Velocity [%0.2f, %0.2f]  Norm = [%0.2f]\n", GetV().GetX(), GetV().GetY(), GetV().Norm());
          if(GetExitLine()) {
               printf(">> ExitLine: (%0.2f, %0.2f) -- (%0.2f, %0.2f)\n", GetExitLine()->GetPoint1().GetX(), GetExitLine()->GetPoint1().GetY(),
                         GetExitLine()->GetPoint2().GetX(), GetExitLine()->GetPoint2().GetY());
               printf(">> dist: %f\n", GetExitLine()->DistTo(GetPos()));
          }
          printf(">> smooth rotating: %s \n", (_newOrientationDelay > 0) ? "yes" : "no");
          printf(">> mental map");
          for(auto&& item: _mentalMap)
               printf("\t room / destination  [%d, %d]\n", item.first, item.second);
          break;

     case 1:
          printf(">> Position [%f, %f]\n", GetPos().GetX(), GetPos().GetY());
          break;

     case 2:
          printf(">> Velocity [%f, %f]\n", GetV().GetX(), GetV().GetY());
          break;

     case 3:
          printf(">> V0       [%f, %f]  Norm = [%f]\n", _V0.GetX(), _V0.GetY(), GetV0Norm());
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
     //fflush(stdout);
     // getc(stdin);
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
          Log->Write("ERROR:\t one or more routers does not exit! Check your router_ids");
          return -1;
     }
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
     _premovement=pretime;
}

double Pedestrian::GetPremovementTime()
{
     return _premovement;
}

const Building* Pedestrian::GetBuilding()
{
     return _building;
}

void Pedestrian::SetBuilding(Building* building)
{
     _building = building;
}

void Pedestrian::SetSpotlight(bool spotlight)
{
     _spotlight = spotlight;
}

void Pedestrian::SetColorMode(AgentColorMode mode)
{
     _colorMode=mode;
}

int Pedestrian::GetColor()
{
     //default color is by velocity
     int color = -1;
     double v0 = GetV0Norm();
     if (v0 != 0.0) {
          double v = GetV().Norm();
          color = (int) (v / v0 * 255);
     }

     switch (_colorMode)
     {
     case BY_SPOTLIGHT:
          if (_spotlight==false)
               color=-1;
          break;

     case BY_VELOCITY:
          break;

          // Hash the knowledge represented as String
     case BY_KNOWLEDGE:
     {
          string key=GetKnowledgeAsString();
          std::hash<std::string> hash_fn;
          color = hash_fn(key) % 255;
          //cout<<"color: "<<hash_fn(key)<<endl;
          //cout<<" key : "<<key<<endl;
     }
     break;

     case BY_ROUTE:
     {
          string key = std::to_string(_routingStrategy);
          std::hash<std::string> hash_fn;
          color = hash_fn(key) % 255;
     }
     break;

     default:
          break;
     }

     return color;
}

