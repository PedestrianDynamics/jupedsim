/**
 * File:   Pedestrian.cpp
 *
 * Created on 30. September 2010, 15:59
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "Pedestrian.h"
#include "../geometry/Building.h"

/************************************************************
  Konstruktoren
 ************************************************************/

Pedestrian::Pedestrian() {
	pRoomID = -1;
	pSubRoomID = -1;
	pExitIndex = -1;
	pPedIndex = 0;
	pMass = 1;
	pTau = 0.5;
	pNewOrientationFlag = false;
	pNewOrientationDelay = 0; //0 seconds, in steps
	tmpFirstOrientation = true;
	pUpdateRate = 0;
	pTurninAngle=0.0;
	pEllipse = Ellipse();
	pNavLine = NULL;
	pReroutingThreshold=0.0; // new orientation after 10 seconds, value is incremented
	pTimeBeforeRerouting=0.0;
	pReroutingEnabled=false;
	pTimeInJam=0.0;
	pPatienceTime=5.0;// time after which the ped feels to be in jam
	pDesiredFinalDestination=FINAL_DEST_OUT;
	pMentalMap=std::map<int, int>();
	pDestHistory=std::vector<int>();
	pDt=0.01;
	pV0=Point(0,0);
	pLastPosition=Point(0,0);

	pHeight=160;
	pAge=30;
	pGender="male";
	pTrip=vector<int> ();
	pGroup=-1;

}

Pedestrian::Pedestrian(const Pedestrian& orig) {
	pRoomID = orig.GetRoomID();
	pSubRoomID = orig.GetSubRoomID();
	pExitIndex = orig.GetExitIndex();
	pPedIndex = orig.GetPedIndex();
	pMass = orig.GetMass();
	pTau = orig.GetTau();
	pEllipse = orig.GetEllipse();
	pNavLine = orig.GetExitLine();
	pDesiredFinalDestination = orig.GetFinalDestination();
	pTimeInJam=orig.GetTimeInJam();
	pReroutingThreshold=0;
	pPatienceTime=10.0;
	pDt=0.01;
	pMentalMap=std::map<int, int>();
	pDestHistory=std::vector<int>();
	pLastPosition=Point(0,0);
}

Pedestrian::~Pedestrian() {
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/

void Pedestrian::SetPedIndex(int i) {
	pPedIndex = i;
}

void Pedestrian::SetRoomID(int i, string roomCaption) {
	pRoomID = i;
	pRoomCaption=roomCaption;
}

void Pedestrian::SetSubRoomID(int i) {
	pSubRoomID = i;
}

void Pedestrian::SetMass(double m) {
	pMass = m;
}

void Pedestrian::SetTau(double tau) {
	pTau = tau;
}

void Pedestrian::SetEllipse(const Ellipse& e) {
	pEllipse = e;
}

void Pedestrian::SetExitIndex(int i) {
	pExitIndex = i;
	//save that destination for that room
	pMentalMap[GetUniqueRoomID()] = i;
	pDestHistory.push_back(i);
}

void Pedestrian::SetExitLine(NavLine* l) {
	pNavLine = l;
}
// ruft entsprechende Ellipsenfunktionen auf

void Pedestrian::SetPos(const Point& pos) {
	pEllipse.SetCenter(pos);

}

void Pedestrian::SetV(const Point& v) {
	pEllipse.SetV(v);
}

void Pedestrian::SetV0Norm(double v0) {
	pEllipse.SetV0(v0);
}

void Pedestrian::Setdt(double dt) {
	pDt = dt;
}

void Pedestrian::SetTrip(vector<int> trip){
	pTrip=trip;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

int Pedestrian::GetPedIndex() const {
	return pPedIndex;
}

int Pedestrian::GetRoomID() const {
	return pRoomID;
}

int Pedestrian::GetSubRoomID() const {
	return pSubRoomID;
}

double Pedestrian::GetMass() const {
	return pMass;
}

double Pedestrian::GetTau() const {
	return pTau;
}

const Ellipse& Pedestrian::GetEllipse() const {
	return pEllipse;
}

int Pedestrian::GetExitIndex() const {
	return pExitIndex;
}

NavLine* Pedestrian::GetExitLine() const {
	return pNavLine;
}

const vector<int> Pedestrian::GetTrip() const{
	return pTrip;
}

// return the unique subroom Identifier

int Pedestrian::GetUniqueRoomID() const {
	return pRoomID * 1000 + pSubRoomID;
}

// returns the exit Id corresponding to the
// unique subroom identifier

int Pedestrian::GetNextDestination() {
	if (pMentalMap.count(GetUniqueRoomID()) == 0) {
		return -1;
	} else {
		return pMentalMap[GetUniqueRoomID()];
	}
}


int Pedestrian::GetLastDestination() {
  if(pDestHistory.size() == 0)
    return -1;
  else
    return pDestHistory.back();

}
// erase the peds memory
void Pedestrian::ClearMentalMap(){
	//	pMentalMapArray[pRoomID][pSubRoomID]=-1;
	pMentalMap.clear();
	pExitIndex=-1;
}



const Point& Pedestrian::GetPos() const {
	return pEllipse.GetCenter();
}

const Point& Pedestrian::GetV() const {
	return pEllipse.GetV();
}

const Point& Pedestrian::GetV0() const {
	return pV0;
}

double Pedestrian::GetV0Norm() const {
	return pEllipse.GetV0();
}

double Pedestrian::GetLargerAxis() const {
	return pEllipse.GetLargerAxis();
}

double Pedestrian::GetSmallerAxis() const {
	return pEllipse.GetSmallerAxis();
}

void Pedestrian::SetPhiPed() {
	double cosPhi, sinPhi;
	double vx = GetV().GetX();
	double vy = GetV().GetY();
	double normv = sqrt(vx * vx + vy * vy);


	cosPhi = GetEllipse().GetCosPhi();
	sinPhi = GetEllipse().GetSinPhi();
	if (fabs(vx) > EPS || fabs(vy) > EPS) {
		cosPhi = vx / normv;
		sinPhi = vy / normv;
	}
	pEllipse.SetCosPhi(cosPhi);
	pEllipse.SetSinPhi(sinPhi);
}

const Point& Pedestrian::GetV0(const Point& target) {

	const Point& pos = GetPos();
	Point delta = target - pos;
	Point new_v0;


	new_v0 = delta.Normalized();
	// aktivieren, wenn Rotation aus sein soll
	//pV0 = new_v0;
	//return pV0;

	// Rotation
	double smoothingGrad = 15;
	if (pNewOrientationFlag) {
		double pi = 3.14159265;
		pTurninAngle = atan2(new_v0.GetY(), new_v0.GetX()) - atan2(pV0.GetY(), pV0.GetX());

		// prefer turning of -30° instead of 330°
		if (pTurninAngle <= -pi)pTurninAngle += 2 * pi;
		if (pTurninAngle >= pi)pTurninAngle -= 2 * pi;

		pNewOrientationFlag = false; //disable and set the delay
		if (fabs(pTurninAngle) > 1.22) {// only for turn greater than +/-70 degrees
			pNewOrientationDelay = 2.0 / pDt; //2 seconds/dt, in steps
			pUpdateRate = pNewOrientationDelay / smoothingGrad;
		}
	}
	if (pNewOrientationDelay > 0) {
		double smoothingAngle_k = pTurninAngle / smoothingGrad;
		if (pNewOrientationDelay % pUpdateRate == 0) {
			pV0 = pV0.Rotate(cos(smoothingAngle_k), sin(smoothingAngle_k));
		}
		pNewOrientationDelay--;

		//stop the rotation if the velocity is too high,  0.9m/s
		// this avoid  drifting
		if (GetV().Norm() > 0.90) {
			pNewOrientationDelay = 0;
		}
	}
	if (pNewOrientationDelay <= 0) {
		pV0 = new_v0;
	}

	return pV0;
}

double Pedestrian::GetTimeInJam() const {
	return pTimeInJam;
}

// set the new orientation flag
// to delay sharp turn
// TODO: maybe combine this with SetExitLine

void Pedestrian::SetSmoothTurning(bool smt) {
	//ignoring first turn
	if (tmpFirstOrientation) {
		tmpFirstOrientation = false;
	} else {
		if (pNewOrientationDelay <= 0)// in the case the pedestrian is still rotating
			pNewOrientationFlag = smt;
	}

}


bool Pedestrian::IsFeelingLikeInJam(){
	//return true;
	return (pPatienceTime<pTimeInJam);
}

void Pedestrian::ResetTimeInJam(){
	pTimeInJam=0.0;
}

void Pedestrian::UpdateTimeInJam(){
	pTimeInJam+=pDt;
}

void Pedestrian::UpdateJamData(){
	if(GetV().NormSquare()<0.25*GetV0().NormSquare()){
		pTimeInJam+=pDt;
	}else{
		pTimeInJam/=2.0;
	}
}

void Pedestrian::UpdateReroutingTime(){
	pTimeBeforeRerouting-=pDt;
}

void Pedestrian::RerouteIn(double time){
	pReroutingEnabled=true;
	pTimeBeforeRerouting=time;
}

bool Pedestrian::IsReadyForRerouting(){
	return(pReroutingEnabled &&(pTimeBeforeRerouting<=0.0));
}

double Pedestrian::GetAge() const {
	return pAge;
}

void Pedestrian::SetAge(double age) {
	pAge = age;
}

string Pedestrian::GetGender() const {
	return pGender;
}

void Pedestrian::SetGender(string gender) {
	pGender = gender;
}

double Pedestrian::GetHeight() const {
	return pHeight;
}

int Pedestrian::GetGroup() const {
	return pGroup;
}

void Pedestrian::SetGroup(int group) {
	pGroup = group;
}

void Pedestrian::SetHeight(double height) {
	pHeight = height;
}

void Pedestrian::ResetRerouting(){
	pReroutingEnabled=false;
	pTimeBeforeRerouting=-1.00;
}

double Pedestrian::GetDistanceToNextTarget() const {
	return (pNavLine->DistTo(GetPos()));
}

void Pedestrian::SetFinalDestination(int final) {
	pDesiredFinalDestination = final;
}

int Pedestrian::GetFinalDestination() const {
	return pDesiredFinalDestination;
}


///@deprecated
void Pedestrian::WritePath(ofstream& file, Building* building){
	map<int, int>::iterator iter;

	if(building){
		for (iter = pMentalMap.begin(); iter != pMentalMap.end(); iter++) {
			file<<building->GetAllRooms()[iter->first/1000]->GetCaption()<<" "<<iter->second<<endl;
		}
	}else{
		for (iter = pMentalMap.begin(); iter != pMentalMap.end(); iter++) {
			file<<iter->first/1000<<" "<<iter->second<<endl;
		}
	}
}

string Pedestrian::GetPath(){
	map<int, int>::iterator iter;
	string path;

	for (iter = pMentalMap.begin(); iter != pMentalMap.end(); iter++) {
		stringstream ss;//create a stringstream
		ss << iter->first/1000<<":"<<iter->second<<">";
		path.append(ss.str());
	}
	return path;
}

void Pedestrian::Dump(int ID, int pa) {

	if (ID != pPedIndex) return;

	printf("------> ped %d <-------\n", pPedIndex);

	switch (pa) {

		case 0:
		{
			printf(">> Room/Subroom [%d / %d]\n", pRoomID, pSubRoomID);
			printf(">> Destination [ %d ]\n", pExitIndex);
			printf(">> Final Destination [ %d ]\n", pDesiredFinalDestination);
			printf(">> Position [%f, %f]\n", GetPos().GetX(), GetPos().GetY());
			printf(">> V0       [%f, %f]  Norm = [%f]\n", pV0.GetX(), pV0.GetY(), GetV0Norm());
			printf(">> Velocity [%f, %f]  Norm = [%f]\n", GetV().GetX(), GetV().GetY(), GetV().Norm());
			if(GetExitLine()){
				printf(">> ExitLine: (%f, %f) -- (%f, %f)\n", GetExitLine()->GetPoint1().GetX(), GetExitLine()->GetPoint1().GetY(),
					GetExitLine()->GetPoint2().GetX(), GetExitLine()->GetPoint2().GetY());
				printf(">> dist: %f\n", GetExitLine()->DistTo(GetPos()));
			}
			printf(">> smooth rotating: %s \n", (pNewOrientationDelay > 0) ? "yes" : "no");
			printf(">> mental map");
			map<int, int>::iterator iter;
			for (iter = pMentalMap.begin(); iter != pMentalMap.end(); iter++) {
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
			printf(">> V0       [%f, %f]  Norm = [%f]\n", pV0.GetX(), pV0.GetY(), GetV0Norm());
			break;

		case 4:
			printf(">> Room/Subroom [%d / %d]\n", pRoomID, pSubRoomID);
			break;

		case 5:
			printf(">> Destination [ %d ]\n", pExitIndex);
			break;
		case 6: //Mental Map
		{
			printf(">> mental map");
			map<int, int>::iterator iter;
			for (iter = pMentalMap.begin(); iter != pMentalMap.end(); iter++) {
				printf("\t room / destination  [%d, %d]", iter->first, iter->second);
			}
		}
		break;

	}
}

void Pedestrian::RecordActualPosition(){
	pLastPosition=GetPos();
}

double Pedestrian::GetDistanceSinceLastRecord(){
	return (pLastPosition-GetPos()).Norm();
}
