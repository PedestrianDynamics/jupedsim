/**
 * @file    AccessPoint.cpp
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: 24 Aug 2010
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "AccessPoint.h"

using namespace std;


AccessPoint::AccessPoint(int id, double center[2],double radius) {

	pID=id;
	this->center[0]=center[0];
	this->center[1]=center[1];
	pRadius=radius;
	pFinalDestination=false;
	pRoom1ID=-1;
	pRoom2ID=-1;
	pConnectingAPs.clear();
	pMapDestToDist.clear();
	pCentre=Point(center[0],center[1]);
	pTransitPedestrians = vector<Pedestrian*>();
	pConnectingAPs = vector<AccessPoint*>();
}

AccessPoint::~AccessPoint() {

}

int AccessPoint::GetID()
{
	return pID;
}

void AccessPoint::setFinalDestination(bool isFinal)
{
	pFinalDestination=isFinal;
}

bool AccessPoint::isFinalDestination()
{
	return pFinalDestination;
}

const Point& AccessPoint::GetCentre() const{
	return pCentre;
}


void AccessPoint::AddIntermediateDest(int final, int inter){
	pMapDestToAp[final]=inter;
}

void AccessPoint::AddFinalDestination(int UID, double distance){
	pMapDestToDist[UID]=distance;
}

double AccessPoint::GetDistanceTo(int UID){
	//this is probably a final destination
	if(pMapDestToDist.count(UID)==0){
		cout<<"No route to host ["<< UID<<" ]"<<endl;
		Dump();
		//TODO:FIXME: this should not happened
		exit(EXIT_FAILURE);
		//return -1;
	}
	return pMapDestToDist[UID];
}

double AccessPoint::GetDistanceTo(AccessPoint* ap){
	return (pCentre-ap->GetCentre()).Norm();
}

void AccessPoint::AddConnectingAP(AccessPoint* ap){
	//only add of not already inside
	for(unsigned int p=0;p<pConnectingAPs.size();p++){
		if(pConnectingAPs[p]->GetID()==ap->GetID()) return;
	}
	pConnectingAPs.push_back(ap);
}

int  AccessPoint::GetNextApTo(int UID){
	//this is probably a final destination
	if(pMapDestToAp.count(UID)==0){
		//cout<<"No route to host ["<< UID<<" ]"<<endl;
		return -1;
	}
	return pMapDestToAp[UID];
}

int AccessPoint::GetNearestTransitAPTO(int UID){
	const vector <AccessPoint*>& possibleDest=  pNavigationGraphTo[UID];

	if(possibleDest.size()==0){
		return -1;
	}else if (possibleDest.size()==1){
		return possibleDest[0]->GetID();
	}else {
		AccessPoint* best_ap=possibleDest[0];
		double min_dist=GetDistanceTo(best_ap);

		for (int i=0;i<possibleDest.size();i++){
			double tmp= GetDistanceTo(possibleDest[i]);
			if(tmp<min_dist){
				min_dist=tmp;
				best_ap=possibleDest[i];
			}
		}
		return best_ap->GetID();
	}
}

void AccessPoint::setConnectingRooms(int r1, int r2){
	pRoom1ID=r1;
	pRoom2ID=r2;
}

double AccessPoint::distanceTo(double x, double y){

	return sqrt((x-center[0])*(x-center[0]) + (y-center[1])*(y-center[1]));
}


bool AccessPoint::isInRange(int roomID){
	if((roomID!=pRoom1ID) && (roomID!=pRoom2ID)){
		return false;
	}
	return true;
}

bool AccessPoint::isInRange(double xPed, double yPed, int roomID){

	if((roomID!=pRoom1ID)&& (roomID!=pRoom2ID)){
		return false;
	}
	if (((xPed - center[0]) * (xPed - center[0]) + (yPed - center[1]) * (yPed
			- center[1])) <= pRadius * pRadius)
		return true;

	return false;
}

void AccessPoint::DeleteTransitPed(Pedestrian* ped){
	vector<Pedestrian*>::iterator it;
	it = find (pTransitPedestrians.begin(), pTransitPedestrians.end(), ped);
	if(it==pTransitPedestrians.end()){
		cout<<" Ped not found"<<endl;
	}else{
		pTransitPedestrians.erase(it);
	}
}


void AccessPoint::AddTransitPed(Pedestrian* ped){
	pTransitPedestrians.push_back(ped);
}

const vector<Pedestrian*>& AccessPoint::GetAllTransitPed() const{
	return pTransitPedestrians;
}

const vector <AccessPoint*>& AccessPoint::GetConnectingAPs(){
	return pConnectingAPs;
}


void AccessPoint::RemoveConnectingAP(AccessPoint* ap){
	vector<AccessPoint*>::iterator it;
	it = find (pConnectingAPs.begin(), pConnectingAPs.end(), ap);
	if(it==pConnectingAPs.end()){
		cout<<" there is no connection to AP: "<< ap->GetID()<<endl;
	}else{
		pConnectingAPs.erase(it);
	}
}

const vector <AccessPoint*>& AccessPoint::GetTransitAPsTo(int UID){
	return pNavigationGraphTo[UID];
}

void AccessPoint::AddTransitAPsTo(int UID,AccessPoint* ap){
	pNavigationGraphTo[UID].push_back(ap);
}

void AccessPoint::Reset(int UID){
	pNavigationGraphTo[UID].clear();
}


void AccessPoint::Dump(){

	cout<<" -----------------------"<<endl;
	cout<<" ID: " <<pID<<" centre = [ "<< center[0] <<", " <<center[1] <<" ]"<<endl;

	cout <<" Distance to final destination"<<endl;

	for(std::map<int, double>::iterator p = pMapDestToDist.begin(); p != pMapDestToDist.end(); p++) {
		cout<<" [ "<<p->first<<", " << p->second<<" m ]";
	}
	cout<<endl<<endl;

	cout<<" transit to final destination:"<<endl;
	for(std::map<int, int>::iterator p = pMapDestToAp.begin(); p != pMapDestToAp.end(); p++) {
		cout<<" ---> [ "<<p->first<<" via " << p->second<<" ]";
	}
	cout<<endl<<endl;

	cout<<" connected to aps : " ;
	for(unsigned int p=0;p<pConnectingAPs.size();p++){
		cout<<" [ "<<pConnectingAPs[p]->GetID()<<" , "<<pConnectingAPs[p]->GetDistanceTo(this)<<" m ]";
	}

	cout<<endl<<endl;
	cout <<" queue [ ";
	for(unsigned int p=0;p<pTransitPedestrians.size();p++){
		cout<<" "<<pTransitPedestrians[p]->GetPedIndex();
	}
	cout<<" ]"<<endl;

	cout<<endl<<" connected to rooms: "<<pRoom1ID<<" and "<<pRoom2ID<<endl;
	cout<<endl;

}
