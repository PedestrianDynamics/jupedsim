/*
 * File:   Pedestrian.cpp
 * Author: andrea
 *
 * Created on 30. September 2010, 15:59
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
	pExitLine = NULL;
	pReroutingThreshold=0.0; // new orientation after 10 seconds, value is incremented
	pTimeBeforeRerouting=0.0;
	pReroutingEnabled=false;
	pTimeInJam=0.0;
	pPatienceTime=5.0;// time after which the ped feels to be in jam
	pDesiredFinalDestination=FINAL_DEST_OUT;
	pMentalMap=std::map<int, int>();
	pDt=0.01;
	pV0=Point(0,0);
	pLastPosition=Point(0,0);

	//	for(int i=0;i<15;i++)
	//		for(int j=0;j<130;j++)
	//			pMentalMapArray[i][j]=-1;

}

Pedestrian::Pedestrian(const Pedestrian& orig) {
	pRoomID = orig.GetRoomID();
	pSubRoomID = orig.GetSubRoomID();
	pExitIndex = orig.GetExitIndex();
	pPedIndex = orig.GetPedIndex();
	pMass = orig.GetMass();
	pTau = orig.GetTau();
	pEllipse = orig.GetEllipse();
	pExitLine = orig.GetExitLine();
	pDesiredFinalDestination = orig.GetFinalDestination();
	pTimeInJam=orig.GetTimeInJam();
	pReroutingThreshold=0;
	pPatienceTime=10.0;
	pDt=0.01;
	pMentalMap=std::map<int, int>();
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
	//pMentalMapArray[pRoomID][pSubRoomID]=i;
	pExitIndex = i;
	//save that destination for that room
	pMentalMap[GetUniqueRoomID()] = i;
}

void Pedestrian::SetExitLine(Line* l) {
	pExitLine = l;
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

Line* Pedestrian::GetExitLine() const {
	return pExitLine;
}

// return the unique subroom Identifier

int Pedestrian::GetUniqueRoomID() const {
	return pRoomID * 1000 + pSubRoomID;
}

// returns the exit Id corresponding to the
// unique subroom identifier

int Pedestrian::GetNextDestination() {
	//return pMentalMapArray[pRoomID][pSubRoomID];
	//return pExitIndex;
	//reset the actual target after 10 seconds
	//    if(pReroutingThreshold>10){
	//            pReroutingThreshold=0;
	//            pMentalMap.erase(GetUniqueRoomID());
	//            cout<<"resetting"<<endl;
	//            return -1;
	//    }

	if (pMentalMap.count(GetUniqueRoomID()) == 0) {
		return -1;
	} else {
		return pMentalMap[GetUniqueRoomID()];
	}
}

// erase the peds memory
void Pedestrian::ClearMentalMap(){
	//	pMentalMapArray[pRoomID][pSubRoomID]=-1;
	pMentalMap.clear();
	pExitIndex=-1;
}


// Eigenschaften der Ellipse

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

bool Pedestrian::IsInThePromenade() const {
	int label = atoi(pRoomCaption.c_str());

	if (label<60 ){

		switch (label){
			case 10:
				switch (pSubRoomID){
					case 0: case 2: case 3:
						return false;
						break;
				}
				break;

			case 30:
				switch (pSubRoomID){
					case 1: case 2: case 3:
						return false;
						break;
				}
				break;
		}
		return true;
	}

	else{
		return false;
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
	//if(GetV().NormSquare()<0.5*GetV0().NormSquare()){
	//	pTimeInJam+=pDt;
	//}else{
	//	pTimeInJam=0.0;
	//}

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

void Pedestrian::ResetRerouting(){
	pReroutingEnabled=false;
	pTimeBeforeRerouting=-1.00;
}

double Pedestrian::GetDistanceToNextTarget() const {
	return (pExitLine->DistTo(GetPos()));
}

void Pedestrian::SetFinalDestination(int final) {
	pDesiredFinalDestination = final;
}

int Pedestrian::GetFinalDestination() const {
	//the first aim is to get out the tribune/rang
	if((IsInThePromenade()==false) /*&& (GetDistanceToNextTarget()>.5)*/)
		return FINAL_DEST_OUT;

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

	//	for(int i=0;i<15;i++){
	//		for(int j=0;j<130;j++){
	//			if(pMentalMapArray[i][j]!=-1)
	//				file<<i<<" "<<pMentalMapArray[i][j]<<endl;
	//		}
	//	}
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
	//printf("-----------(key?)------------\n\n");
	//getc(stdin);
}

void Pedestrian::RecordActualPosition(){
	pLastPosition=GetPos();
}

double Pedestrian::GetDistanceSinceLastRecord(){
	return (pLastPosition-GetPos()).Norm();
}
