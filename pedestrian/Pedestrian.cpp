/**
 * \file        Pedestrian.cpp
 * \date        Sep 30, 2010
 * \version     v0.5
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


#include "Pedestrian.h"
#include "../geometry/Building.h"
#include "../geometry/NavLine.h"
#include "../routing/Router.h"
#include "../geometry/SubRoom.h"
#include "../IO/OutputHandler.h"
#include <cassert>

using namespace std;

/// initialize the static variables
double Pedestrian::_globalTime = 0.0;
bool Pedestrian::_enableSpotlight = false;




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
     _newOrientationDelay = 0; //0 seconds, in steps
     _tmpFirstOrientation = true;
     _updateRate = 0;
     _turninAngle = 0.0;
     _ellipse = JEllipse();
     _navLine = new NavLine(); //FIXME this is not released
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
     _recordingTime = 5; //seconds

     _knownDoors = map<int, NavLineState>();

     _height = 160;
     _age = 30;
     _gender = "male";
     _trip = vector<int> ();
     _group = -1;
     _spotlight = false;
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

void Pedestrian::SetPos(const Point& pos)
{
     _ellipse.SetCenter(pos);

     //save the last values for the records
     _lastPositions.push(pos);
     unsigned int max_size = _recordingTime/_deltaT;
     if(_lastPositions.size() > max_size)
          _lastPositions.pop();
}

void Pedestrian::SetCellPos(int cp)
{
     _lastCellPosition = cp;
}

void Pedestrian::SetV(const Point& v)
{
     _ellipse.SetV(v);

     //save the last values for the records
     _lastVelocites.push(v);
     unsigned int max_size = _recordingTime/_deltaT;
     if(_lastVelocites.size()> max_size)
          _lastVelocites.pop();
}

void Pedestrian::SetV0Norm(double v0)
{
     _ellipse.SetV0(v0);
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

void Pedestrian::AddKnownClosedDoor(int door)
{
     if(_knownDoors.find(door) == _knownDoors.end()) {
          _knownDoors[door].close(GetGlobalTime());
     }
     return;
}

int Pedestrian::DoorKnowledgeCount() const
{
     return _knownDoors.size();
}



set<int>  Pedestrian::GetKnownClosedDoors()
{
     map<int, NavLineState>::iterator it;
     set<int> doors_closed;
     for(it = _knownDoors.begin(); it != _knownDoors.end(); it++) {

          if(it->second.closed()) {
               doors_closed.insert(it->first);
          }
     }

     return doors_closed;
}

map<int, NavLineState> *  Pedestrian::GetKnownDoors()
{
     return & _knownDoors;
}

void Pedestrian::MergeKnownClosedDoors( map<int, NavLineState> * input)
{
     map<int, NavLineState>::iterator it;
     for(it = input->begin(); it != input->end(); it++) {
          //it->second.print();
          if(it->second.isShareable(GetGlobalTime())) {
               if(_knownDoors.find(it->first) == _knownDoors.end()) {
                    _knownDoors[it->first] = NavLineState();
                    if(!_knownDoors[it->first].mergeDoor(it->second, GetGlobalTime())) {
                         _knownDoors.erase(it->first);
                    }
               } else {
                    _knownDoors[it->first].mergeDoor(it->second, GetGlobalTime());
               }
          }
     }
     return;
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
     return _ellipse.GetV0()*_building->GetRoom(_roomID)->GetSubRoom(_subRoomID)->GetCosAngleWithHorizontal();
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

const Point& Pedestrian::GetV0(const Point& target)
{

     const Point& pos = GetPos();
     Point delta = target - pos;
     Point new_v0;

     // Molification around the targets makes little sense
     //new_v0 = delta.NormalizedMolified();
     new_v0 = delta.Normalized();
     //printf("BEVOR new_v0=%f %f norm = %f\n", new_v0.GetX(), new_v0.GetY(), new_v0.Norm());
     // printf("AFTER new_v0=%f %f norm = %f\n", new_v0.GetX(), new_v0.GetY(), new_v0.Norm());
     // -------------------------------------- Handover new target
     double t = _newOrientationDelay++ *_deltaT;
     // printf("t=%f, neworientation=%d\n", t, _newOrientationDelay);
     //getc(stdin);
     _V0 = _V0 + (new_v0 - _V0)*( 1 - exp(-t/_tau) );
     //printf("_v0=%f %f norm = %f\n", _V0.GetX(), _V0.GetY(), _V0.Norm());
     //getc(stdin);
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

void Pedestrian::SetSmoothTurning(bool smt)
{
     _newOrientationDelay = 0;
}


bool Pedestrian::IsFeelingLikeInJam()
{
     //return true;
     return (_patienceTime < _timeInJam);
}

void Pedestrian::ResetTimeInJam()
{
     _timeInJam = 0.0;
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
void Pedestrian::WritePath(ofstream& file, Building* building)
{
     map<int, int>::iterator iter;

     if(building) {
          for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
               file<<building->GetAllRooms()[iter->first/1000]->GetCaption()<<" "<<iter->second<<endl;
          }
     } else {
          for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
               file<<iter->first/1000<<" "<<iter->second<<endl;
          }
     }
}

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

     case 0: {
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
          map<int, int>::iterator iter;
          for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
               printf("\t room / destination  [%d, %d]\n", iter->first, iter->second);
          }
     }
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
     case 6: { //Mental Map
          printf(">> mental map");
          map<int, int>::iterator iter;
          for (iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
               printf("\t room / destination  [%d, %d]", iter->first, iter->second);
          }
     }
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
}

Router* Pedestrian::GetRouter() const
{
     return _router;
}

int Pedestrian::FindRoute()
{
     if( ! _router) {
          Log->Write("ERROR:\t one or more routers does not exit! Check your router_ids");
          exit(EXIT_FAILURE);
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


bool Pedestrian::GetSpotlight()
{
     return !_enableSpotlight || _spotlight;
}

void Pedestrian::ActivateSpotlightSystem(bool status)
{
     _enableSpotlight = status;
}
