/**
 * \file        AccessPoint.cpp
 * \date        Aug 24, 2010
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


#include "AccessPoint.h"

using namespace std;


AccessPoint::AccessPoint(int id, double center[2],double radius)
{
     _id=id;
     _center[0]=center[0];
     _center[1]=center[1];
     _radius=radius;
     _finaExitToOutside=false;
     _finalGoalOutside=false;
     _room1ID=-1;
     _room2ID=-1;
     _connectingAPs.clear();
     _mapDestToDist.clear();
     pCentre=Point(center[0],center[1]);
     _transitPedestrians = vector<Pedestrian*>();
     _connectingAPs = vector<AccessPoint*>();
     _isClosed=0;
     _navLine=nullptr;
}

AccessPoint::~AccessPoint()
{
     if(_navLine) delete _navLine;
}

int AccessPoint::GetID()
{
     return _id;
}

int AccessPoint::IsClosed()
{
     return _isClosed;
}

void AccessPoint::SetClosed(int isClosed)
{
     _isClosed=isClosed;
}
void AccessPoint::SetFinalExitToOutside(bool isFinal)
{
     _finaExitToOutside=isFinal;
}

bool AccessPoint::GetFinalExitToOutside()
{
     return _finaExitToOutside;
}

const Point& AccessPoint::GetCentre() const
{
     return pCentre;
}

void AccessPoint::SetFinalGoalOutside(bool isFinal)
{
     _finalGoalOutside=isFinal;
}

bool AccessPoint::GetFinalGoalOutside()
{
     return _finalGoalOutside;
}

//TODO: possibly remove
void AccessPoint::AddIntermediateDest(int final, int inter)
{
     _mapDestToAp[final]=inter;
}

void AccessPoint::AddFinalDestination(int UID, double distance)
{
     _mapDestToDist[UID]=distance;
}

double AccessPoint::GetDistanceTo(int UID)
{
     //this is probably a final destination
     if(_mapDestToDist.count(UID)==0) {
          Log->Write("ERROR:\tNo route to destination  [ %d ]",UID);
          Log->Write("ERROR:\tCheck your configuration file");
          Dump();
          //return 0;
          exit(EXIT_FAILURE);
     }
     return _mapDestToDist[UID];
}

double AccessPoint::GetDistanceTo(AccessPoint* ap)
{
     return (pCentre-ap->GetCentre()).Norm();
}

void AccessPoint::AddConnectingAP(AccessPoint* ap)
{
     //only add of not already inside
     for(unsigned int p=0; p<_connectingAPs.size(); p++) {
          if(_connectingAPs[p]->GetID()==ap->GetID()) return;
     }
     _connectingAPs.push_back(ap);
}

//TODO: remove this one
int  AccessPoint::GetNextApTo(int UID)
{
     //this is probably a final destination
     if(_mapDestToAp.count(UID)==0) {
          Log->Write("ERROR:\tNo route to destination  [ %d ]",UID);
          Log->Write("ERROR:\t Did you forget to define the goal in the configuration file?");
          Dump();
          exit(EXIT_FAILURE);
     }
     return _mapDestToAp[UID];
}

int AccessPoint::GetNearestTransitAPTO(int UID)
{
     const vector <AccessPoint*>& possibleDest=_navigationGraphTo[UID];

     if(possibleDest.size()==0) {
          return -1;
     } else if (possibleDest.size()==1) {
          return possibleDest[0]->GetID();
     } else {
          AccessPoint* best_ap=possibleDest[0];
          double min_dist=GetDistanceTo(best_ap) + best_ap->GetDistanceTo(UID); // FIXME: add the shortest distance to outside

          for (unsigned int i=0; i<possibleDest.size(); i++) {
               double tmp= GetDistanceTo(possibleDest[i]);
               if(tmp<min_dist) {
                    min_dist=tmp;
                    best_ap=possibleDest[i];
               }
          }
          return best_ap->GetID();
     }
}

void AccessPoint::setConnectingRooms(int r1, int r2)
{
     _room1ID=r1;
     _room2ID=r2;
}

double AccessPoint::DistanceTo(double x, double y)
{

     return sqrt((x-_center[0])*(x-_center[0]) + (y-_center[1])*(y-_center[1]));
     //return _navLine->DistTo(Point(x,y));
}


bool AccessPoint::isInRange(int roomID)
{
     if((roomID!=_room1ID) && (roomID!=_room2ID)) {
          return false;
     }
     return true;
}

bool AccessPoint::IsInRange(double xPed, double yPed, int roomID)
{

     if((roomID!=_room1ID)&& (roomID!=_room2ID)) {
          return false;
     }
     if (((xPed - _center[0]) * (xPed - _center[0]) + (yPed - _center[1]) * (yPed
               - _center[1])) <= _radius * _radius)
          return true;

     return false;
}

void AccessPoint::SetNavLine(NavLine* line)
{
     //todo: check this
     //_navLine= line;
     //_navLine->SetPoint1(line->GetPoint1());
     //_navLine->SetPoint2(line->GetPoint2());
     _navLine= new NavLine(*line);
}

NavLine* AccessPoint::GetNavLine() const
{
     return _navLine;
}

const vector <AccessPoint*>& AccessPoint::GetConnectingAPs()
{
     return _connectingAPs;
}

void AccessPoint::RemoveConnectingAP(AccessPoint* ap)
{
     vector<AccessPoint*>::iterator it;
     it = find (_connectingAPs.begin(), _connectingAPs.end(), ap);
     if(it==_connectingAPs.end()) {
          cout<<" there is no connection to AP: "<< ap->GetID()<<endl;
     } else {
          _connectingAPs.erase(it);
     }
}

const vector <AccessPoint*>& AccessPoint::GetTransitAPsTo(int UID)
{
     return _navigationGraphTo[UID];
}

void AccessPoint::AddTransitAPsTo(int UID,AccessPoint* ap)
{
     _navigationGraphTo[UID].push_back(ap);
}

void AccessPoint::Reset(int UID)
{
     _navigationGraphTo[UID].clear();
}


void AccessPoint::SetFriendlyName(const std::string& name)
{
     _friendlyName=name;
}


const std::string AccessPoint::GetFriendlyName()
{
     return _friendlyName;
}


void AccessPoint::Dump()
{

     cout<<endl<<"--------> Dumping AP <-----------"<<endl<<endl;
     //cout<<" ID: " <<_id<<" centre = [ "<< _center[0] <<", " <<_center[1] <<" ]"<<endl;
     cout<<" Friendly ID: " <<_friendlyName<<" centre = [ "<< _center[0] <<", " <<_center[1] <<" ]"<<endl;
     cout<<" Real ID: " <<_id<<endl;
     cout<<" Length:  "<<_navLine->LengthSquare()<<endl;

     cout <<" Is final exit to outside :"<<GetFinalExitToOutside()<<endl;
     cout <<" Distance to final goals"<<endl;

     for(std::map<int, double>::iterator p = _mapDestToDist.begin(); p != _mapDestToDist.end(); ++p) {
          cout<<"\t [ "<<p->first<<", " << p->second<<" m ]";
     }
     cout<<endl<<endl;

     cout<<" transit to final goals:"<<endl;
     for(std::map<int, std::vector<AccessPoint*> >::iterator p = _navigationGraphTo.begin(); p != _navigationGraphTo.end(); ++p) {
          cout<<endl<<"\t to UID ---> [ "<<p->first <<" ]";

          if(p->second.size()==0) {
               cout<<"\t ---> [ Nothing ]";
          } else {

               for(unsigned int i=0; i<p->second.size(); i++) {
                    cout<<"\t distance ---> [ "<<GetDistanceTo(p->second[i])+p->second[i]->GetDistanceTo(p->first) <<" m via "<<p->second[i]->GetID() <<" ]";
                    //cout<<"\t distance ---> [ "<<p->second[i]->GetID()<<" @ " << GetDistanceTo(p->first)<<" ]";
               }
          }
     }

     cout<<endl<<endl;

     cout<<" connected to aps : " ;
     for(unsigned int p=0; p<_connectingAPs.size(); p++) {
          //cout<<" [ "<<_connectingAPs[p]->GetID()<<" , "<<_connectingAPs[p]->GetDistanceTo(this)<<" m ]";
          cout<<endl<<"\t [ "<<_connectingAPs[p]->GetID()<<"_"<<_connectingAPs[p]->GetFriendlyName()<<" , "<<_connectingAPs[p]->GetDistanceTo(this)<<" m ]";
     }

     cout<<endl<<endl;
     cout <<" queue [ ";
     for(unsigned int p=0; p<_transitPedestrians.size(); p++) {
          cout<<" "<<_transitPedestrians[p]->GetID();
     }
     cout<<" ]"<<endl;

     //cout<<endl<<" connected to rooms: "<<_room1ID<<" and "<<_room2ID<<endl;
     cout<<endl;
     cout<<endl<<"------------------------------"<<endl<<endl;

}
