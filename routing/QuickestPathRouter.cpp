/**
 * @file    QuickestPathRouter.cpp
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: Apr 20, 2011
 * Copyright (C) <2009-2011>
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

#include "QuickestPathRouter.h"

QuickestPathRouter::QuickestPathRouter( ):GlobalRouter() {
}

QuickestPathRouter::~QuickestPathRouter() {
}

int QuickestPathRouter::FindExit(Pedestrian* ped){

	int next=FindNextExit(ped);

	// that ped will be deleted
	if(next==-1) return next;

	if(ped->IsFeelingLikeInJam()){
		Redirect(ped);
		ped->ResetTimeInJam();
		//ped->RerouteIn(2.50); // seconds
	}else if(ped->IsReadyForRerouting()){
		Redirect(ped);
		ped->ResetRerouting();
	}else{
		ped->UpdateReroutingTime();
	}

	return next;
}

int QuickestPathRouter::FindNextExit(Pedestrian* ped){

	int nextDestination=ped->GetNextDestination();
	//get the room and find the corresponding node

	SubRoom* sub = _building->GetRoom(ped->GetRoomID())->GetSubRoom(ped->GetSubRoomID());

	// get the opened exits
	int bestAPsID=-1;
	double minDist=FLT_MAX;

	const vector<int>& accessPointsInSubRoom =sub->GetAllGoalIDs();

	for(unsigned int i=0;i<accessPointsInSubRoom.size();i++){

		int apID=accessPointsInSubRoom[i];

		const Point& pt3=ped->GetPos();
		Crossing* cross = _building->GetGoal(apID);
		double distToExit=cross->DistTo(pt3);

		//double distToExit2=(pt3-cross->ShortestPoint(pt3)).NormSquare();
		//if(distToExit2 >EPS_AP_DIST*EPS_AP_DIST) continue;

		double tolerance=J_EPS_AP_DIST;
		if(cross->GetSubRoom1()==cross->GetSubRoom2()){
			tolerance=J_EPS_HL_DIST;
		}
		if(distToExit >tolerance) continue;


		//uncomment these lines to return to the gsp
		//nextDestination=pAccessPoints[apID]->GetNextApTo(ped->GetFinalDestination());;
		//int dum=pAccessPoints[apID]->GetNextApTo(ped->GetFinalDestination());;

		nextDestination = GetQuickestRoute(ped,_accessPoints[apID]);
		//printf("[%d === %d]\n",dum,nextDestination);

		if(nextDestination==-1){ // we are almost at the exit
			nextDestination=apID;
		}

		//check that the next destination is in the actual room of the pedestrian
		if(_accessPoints[nextDestination]->isInRange(ped->GetUniqueRoomID())==false) {
			//return the last destination if defined
			int previousDestination=ped->GetNextDestination();

			//we are still somewhere in the initialization phase
			// and we are aright near a final exit to outside
			if(previousDestination==-1){

				// there is nothing to delete in this case
				//pAccessPoints[nextDestination]->DeleteTransitPed(ped);

				//pAccessPoints[apID]->AddTransitPed(ped);

				ped->SetExitIndex(apID);
				ped->SetExitLine(_building->GetGoal(apID));
				ped->SetSmoothTurning(true);
				return apID;
			}
			else // we are still having a valid destination, don't change
			{
				return previousDestination;
			}
		}
		else  // we have reached the new room
		{
			// only assigned if not already assigned

			ped->SetExitIndex(nextDestination);
			ped->SetExitLine(_building->GetGoal(nextDestination));
			ped->SetSmoothTurning(true);
			return nextDestination;
		}
	}

	//any actual valid destinations?
	// if so keep it
	int next=ped->GetNextDestination();

	if(next!=-1) return next;

	// This usually correspond to the first initialisation step
	// or after resetting the actual destination

	//ich checke alles was ich sehen kann
	//dann entscheide ich  mich fuer die kurzere Variante

	// in the case the previous attempt didnt work
	minDist=FLT_MAX;
	bestAPsID=-1;
	//int randomExit=0;

	for(unsigned int i=0;i<accessPointsInSubRoom.size();i++)
	{
		int apID=accessPointsInSubRoom[i];
		//randomExit=apID;
		int uniqueRoomID= ped->GetUniqueRoomID();
		if(_accessPoints[apID]->isInRange(uniqueRoomID)==false) continue;

		//check if that exit is open.
		if(_building->GetGoal(apID)->IsOpen()==false) continue;

		//FIXME
		//will not longer be needed if the agents take the exits they see and
		// not the neareast aps first hlines in room010 and room030
		if(apID==1352) continue;
		if(apID==1339) continue;
		if(apID==1185) continue;

		//avoid going into rang charakterized by size==2
		// provided there are more choices
		if ((_building->GetGoal(apID)->Length()<0.53)&&
				(accessPointsInSubRoom.size()>10)) continue;

		//check if I can see the exit
		SubRoom* sub = _building->GetRoom(ped->GetRoomID())->GetSubRoom(ped->GetSubRoomID());

		// segment connecting the two APs/goals
		const Point& p1 = (_building->GetGoal(apID)->GetPoint1()+_building->GetGoal(apID)->GetPoint2())*0.5;
		const Point& p2 = ped->GetPos();
		Line segment = Line(p1,p2);

		// check if this in intersected by any other connections/walls/doors/trans/cross in the room
		bool isVisible=true;

		//first walls
		const vector<Wall>& walls= sub->GetAllWalls();

		for(unsigned int b=0;b<walls.size();b++){
			if(segment.IntersectionWith(walls[b])==true) {
				isVisible=false;
				//cout<<"failed: walls "<<b<<" in subroom " << sub->GetSubRoomID()<<endl;
				break;
			}
		}
		if(isVisible==false) continue;

		// then all goals
		for(unsigned int g=0;g<accessPointsInSubRoom.size();g++){
			int gID=accessPointsInSubRoom[g];
			if(gID==apID) continue;
			// skip the concerned exits door and d
			if(segment.IntersectionWith(*_building->GetGoal(gID))==true) {
				isVisible=false;
				//cout<<"failed: goal "<<gID<<endl;
				break;
			}
		}
		if(isVisible==false) continue;
		double x = ped->GetPos().GetX();
		double y = ped->GetPos().GetY();
		double dist=_accessPoints[apID]->GetDistanceTo(ped->GetFinalDestination())+_accessPoints[apID]->distanceTo(x,y);

		//FIXME: should be based on time not on distance
		if(dist<minDist){
			bestAPsID=_accessPoints[apID]->GetID();
			minDist=dist;
			//cout<<" best found: " <<apID<<" " <<bestAPsID<<endl;
		}
	}

	if (bestAPsID==-1){


		char tmp[CLENGTH];
		const char* caption=_building->GetRoom(ped->GetRoomID())->GetCaption().c_str();
		sprintf(tmp,"WARNING: GlobalRouter: best AP could not be identified for pedestrian %d in room/subroom [%s] %d/%d \n",ped->GetPedIndex(), caption, ped->GetSubRoomID(),ped->GetSubRoomID());
		Log->Write(tmp);
		Log->Write("WARNING: GlobalRouter: There are no exit in the sight range");
		bestAPsID=GetBestDefaultRandomExit(ped);
		sprintf(tmp,"WARNING: GlobalRouter: I am choosing a random one [ %d ]",bestAPsID);
		Log->Write(tmp);

		ped->SetExitLine(_building->GetGoal(0));
		exit(EXIT_FAILURE);
		//return -1;
	}


	ped->SetExitIndex(bestAPsID);
	ped->SetSmoothTurning(true);
	ped->SetExitLine(_building->GetGoal(bestAPsID));

	//pAccessPoints[0]->Dump();
	return bestAPsID;
}


double QuickestPathRouter::CBA (double ref_g1, double comp_g2){
	return (comp_g2-ref_g1)/(ref_g1+comp_g2);
}


double QuickestPathRouter::TAP (double alpha){
	alpha=fabs(alpha);
	const double pi = 3.14159265;

	if(alpha<(pi/3.0)){
		return 0.9;
	}
	else if((alpha>=(pi/3.0))&&(alpha<(2*pi/3))){
		return 0.8;
	}else{
		return 0.7;
	}
	//	return ( (alpha < pi/3 )? (0.9):( (alpha<2*pi/3) ? (0.8):(0.9)) );
	return 1;
}


int QuickestPathRouter::GetQuickestRoute(Pedestrian*ped, AccessPoint* nearestAP){

	// uncomment this line to get the standard global shortest path
	//return nearestAP->GetNextApTo(ped->GetFinalDestination());

	int preferredExit=nearestAP->GetNearestTransitAPTO(ped->GetFinalDestination());
	double preferredExitTime=FLT_MAX;
	int quickest=-1;
	double minTime=FLT_MAX;


	// get all AP connected to the nearest
	//const vector<AccessPoint*>& aps = nearestAP->GetConnectingAPs();


	const vector<AccessPoint*>& aps = nearestAP->GetTransitAPsTo(ped->GetFinalDestination());

	//special case where there is only one alternative
	if(aps.size()==1) return preferredExit;

	//special case again going from tribune from rooms 070/090 to rooms 010/030
	//1178 is in the room 030,
	//1189 is in the room 010
	if((nearestAP->GetID()==1178) || (nearestAP->GetID()==1189)) {
		double distMin=FLT_MAX;
		int nearest=-1;
		for(unsigned int ap=0;ap<aps.size();ap++){
			double dist1=(aps[ap]->GetCentre()-ped->GetPos()).NormSquare();
			if(dist1<distMin){
				distMin=dist1;
				nearest=aps[ap]->GetID();
			}
		}
		return nearest;
	}


	//select the optimal time
	for(unsigned int ap=0;ap<aps.size();ap++){

		// select the reference and
		int flag=0;
		int exitid=aps[ap]->GetID();
		Pedestrian* myref=NULL;
		SelectReferencePedestrian(ped,&myref,exitid,&flag);

//		if((ped->GetPedIndex()==11)&&(ped->GetExitIndex()==1186)){
//				printf("\n\n\n\n");
//				printf("AP  %d\n",aps[ap]->GetID());
//				printf("Ref Ped: %d\n",(myref)?myref->GetPedIndex():-1);
//				printf("Flag: %d\n",flag);
//				//nearestAP->Dump();
//				ped->Dump(11);
//			}

		// compute the time
		double time=FLT_MAX;

		// case of free exit
		if((myref==NULL)&& (flag==FREE_EXIT)){
			//			time= (ped->GetPos()- aps[ap]->GetCentre()).Norm()/ped->GetV0Norm();
			// time to reach the AP
			double t1 = (ped->GetPos()- aps[ap]->GetCentre()).Norm()/ped->GetV().Norm();

			//guess time from the Ap to the outside
			double t2 = (aps[ap]->GetDistanceTo(ped->GetFinalDestination()))/ped->GetV().Norm();

			time=t1+t2;
		}

		// case of unreachable exit
		if((myref==NULL)&& (flag==UNREACHEABLE_EXIT)){
			time= FLT_MAX;
		}

		// case of ref ped
		if((myref!=NULL) && (flag==REF_PED_FOUND)){

			//time to reach the reference
			double t1= (ped->GetPos()- myref->GetPos()).Norm()/ped->GetV().Norm();
			//double t1= (ped->GetPos()- myref->GetPos()).Norm()/ped->GetV0Norm();

			//if(myref->GetV().Norm()==0.0){
			//	cout<<"bye"<<endl; exit(0);
			//}
			//time for the reference to get out
			double t2=(myref->GetPos()- aps[ap]->GetCentre()).Norm()/myref->GetV().Norm();

			//guess time from the Ap to the outside
			double t3 = (aps[ap]->GetDistanceTo(ped->GetFinalDestination()))/ped->GetV().Norm();

			time=t1+t2+t3;

		}

		if((myref==NULL) && (flag==REF_PED_FOUND)){
			cout<<" Fatal Error in Quickest Path Router"<<endl;
			cout<<" reference pedestrians is NULL"<<endl;
			exit(EXIT_FAILURE);
		}

		if(time<minTime){
			minTime=time;
			quickest=exitid;
		}

		//printf(" ped [%d] checking [%d] ---> [%f]\n",ped->GetPedIndex(),exitid,time);
		//also save the time for the default destinations for later comparison
		if (exitid==preferredExit){
			preferredExitTime=time;
		}

		//if(ped->GetPedIndex()==27){
		//	printf("reference to exit [%d] is ped [%d]\n",exitid,myref->GetPedIndex());
		//}
	}
	//compare it with my preferred (shortest nearest)
	if(quickest==preferredExit) return quickest;

	double cba = CBA(gain(preferredExitTime),gain(minTime));

	//cout<<"cba:" <<cba<<endl;
	if (cba<0.05) return preferredExit;

	return quickest;
}


double QuickestPathRouter::gain(double time){
	return 1.0/time;
}


// based on jam only
//int QuickestPathRouter::GetQuickestRoute(Pedestrian*ped, AccessPoint* nearestAP){
//
//	// uncomment this line to get the standard global shortest path
//	//return nearestAP->GetNextApTo(ped->GetFinalDestination());
//
//	int quickest=-1;
//	double minJam=FLT_MAX;
//	//first return the result based on congestion
//
//	// get all AP connected to the nearest
//	const vector<AccessPoint*>& aps = nearestAP->GetConnectingAPs();
//
//	//TODO: delete not relevant alternatives
//
//	for(unsigned int ap=0;ap<aps.size();ap++){
//
//		// this wont be necessary if you have directed graph
//		double dist1=aps[ap]->GetDistanceTo(ped->GetFinalDestination());
//		double dist2=nearestAP->GetDistanceTo(ped->GetFinalDestination());
//		if(dist1>=dist2) continue;
//
//		double jam=GetJamSizeAtExit(aps[ap]->GetID());
//		if(jam<=minJam){
//			minJam=jam;
//			quickest=aps[ap]->GetID();
//		}
//	}
//
//	// return the one with the lowest jam size
//	return quickest;
//}


//TODO: exclude myself from the jam computations
double QuickestPathRouter::GetJamSizeAtExit(int exitID){
	double result=0.0;
	double jamThreshold=0.5;
	//lazy version of the implementation

	// get the connecting rooms
	// cant do this with the APs, need subrooms
	SubRoom* sbr1 = _building->GetGoal(exitID)->GetSubRoom1();
	SubRoom* sbr2 = _building->GetGoal(exitID)->GetSubRoom2();

	if (sbr1){
		const vector<Pedestrian*>& peds = sbr1->GetAllPedestrians();
		for (unsigned int p=0;p<peds.size();p++){
			Pedestrian* ped = peds[p];
			if(ped->GetExitIndex()==exitID){
				if(ped->GetV().Norm()<jamThreshold){
					result+=ped->GetEllipse().GetArea();
				}
			}
		}
	}

	if (sbr2){
		const vector<Pedestrian*>& peds = sbr2->GetAllPedestrians();
		for (unsigned int p=0;p<peds.size();p++){
			Pedestrian* ped= peds[p];
			if(ped->GetExitIndex()==exitID){
				if(ped->GetV().Norm()<jamThreshold){
					result+=ped->GetEllipse().GetArea();
				}
			}
		}
	}

	// get the pedestrians with destination exitID

	// check the velocities

	// compute the areas
	return result;
}


void QuickestPathRouter::ReduceGraph(){

	for(unsigned int i=0;i<_accessPoints.size();i++){
		vector<AccessPoint*>toBeDeleted;
		AccessPoint* from_AP=_accessPoints[i];
		int from_door=from_AP->GetID();

		// get all AP connected to the nearest
		const vector<AccessPoint*>& aps = from_AP->GetConnectingAPs();

		//loop over all accesspoint connections and
		//collect the connections to remove

		for(unsigned int j=0;j<aps.size();j++)
		{
			AccessPoint* to_AP=aps[j];

			/* TODO: check all final destinations
			for( map<int, int>::iterator it = pMapIdToFinalDestination.begin();
					it != pMapIdToFinalDestination.end(); it++) {
				int fid=it->first;
			 */


			//remove all AP which point to me
			if(to_AP->GetNearestTransitAPTO(FINAL_DEST_OUT)==from_door){
				toBeDeleted.push_back(to_AP);
			}

			//don't remove if that is the best destination
			//TODO: if there are more suitable final destinations?
			if(GetCommonDestinationCount(from_AP, to_AP)>0){
				if(from_AP->GetNearestTransitAPTO(FINAL_DEST_OUT)!=to_AP->GetID())
					toBeDeleted.push_back(to_AP);
			}

			// remove all APs wich have at least one common destination with me
			//			if (to_AP->GetNextApTo(FINAL_DEST_OUT)==from_AP->GetNextApTo(FINAL_DEST_OUT)) {
			//				toBeDeleted.push_back(to_AP);
			//
			//			}
		}

		// now remove the aps/connections
		std::sort(toBeDeleted.begin(), toBeDeleted.end());
		toBeDeleted.erase(std::unique(toBeDeleted.begin(), toBeDeleted.end()), toBeDeleted.end());
		for(unsigned int k=0;k<toBeDeleted.size();k++){
			from_AP->RemoveConnectingAP(toBeDeleted[k]);
		}

	}


	//clear double links
	CheckAndClearDoubleLinkedNodes();
}

void QuickestPathRouter::CheckAndClearDoubleLinkedNodes(){

	for(unsigned int i=0;i<_accessPoints.size();i++){
		vector<AccessPoint*>toBeDeleted;
		AccessPoint* from_AP=_accessPoints[i];
		const vector<AccessPoint*>& from_aps = from_AP->GetConnectingAPs();

		for(unsigned int j=0;j<_accessPoints.size();j++){
			AccessPoint* to_AP=_accessPoints[j];
			const vector<AccessPoint*>& to_aps = to_AP->GetConnectingAPs();

			// if one contains the other
			if(IsElementInVector(from_aps,to_AP)&&IsElementInVector(to_aps,from_AP)){
				//check the distances
				double dist1=from_AP->GetDistanceTo(to_AP) +to_AP->GetDistanceTo(FINAL_DEST_OUT);
				double dist2=to_AP->GetDistanceTo(from_AP) +from_AP->GetDistanceTo(FINAL_DEST_OUT);
				if(dist1<dist2){
					to_AP->RemoveConnectingAP(from_AP);
				}else{
					from_AP->RemoveConnectingAP(to_AP);
				}
			}
		}
	}
}

void QuickestPathRouter::ExpandGraph(){

	for(unsigned int i=0;i<_accessPoints.size();i++){
		vector<AccessPoint*>toBeDeleted;
		AccessPoint* tmp=_accessPoints[i];
		//int tmpID=tmp->GetID();

		// get all AP connected to the nearest
		const vector<AccessPoint*>& aps = tmp->GetConnectingAPs();

		//loop over all accesspoint connections and
		//collect the connections to remove

		for(unsigned int j=0;j<aps.size();j++)
		{
			AccessPoint* tmp1=aps[j];

			if(tmp->GetNearestTransitAPTO(FINAL_DEST_OUT)==tmp1->GetNearestTransitAPTO(FINAL_DEST_OUT))
				toBeDeleted.push_back(tmp1);
			if(tmp->GetID()==tmp1->GetNearestTransitAPTO(FINAL_DEST_OUT))
				toBeDeleted.push_back(tmp1);
			if(tmp1->GetDistanceTo(FINAL_DEST_OUT)>tmp->GetDistanceTo(FINAL_DEST_OUT))
				toBeDeleted.push_back(tmp1);

			int bestID=tmp1->GetNearestTransitAPTO(FINAL_DEST_OUT);
			double dist1=_accessPoints[bestID]->GetDistanceTo(tmp)+tmp->GetDistanceTo(tmp1);
			double dist2=tmp->GetDistanceTo(tmp1);
			if(dist1<dist2)
				toBeDeleted.push_back(tmp1);

			//			for(unsigned int l=0;l<aps.size();l++)
			//			{
			//				AccessPoint* tmp2=aps[l];
			//				if(tmp2->GetID()==tmp1->GetID())continue;
			//
			//				const vector<AccessPoint*>& aps1 = tmp1->GetConnectingAPs();
			//				for(unsigned int k=0;k<aps1.size();k++)
			//				{
			//					AccessPoint* tmp3=aps1[k];
			//					if(tmp3->GetID()==tmp2->GetID()){
			//						toBeDeleted.push_back(tmp1);
			//					}
			//				}
			//			}
		}

		// now remove the aps/connections
		for(unsigned int k=0;k<toBeDeleted.size();k++){
			tmp->RemoveConnectingAP(toBeDeleted[k]);
		}

	}
}

void QuickestPathRouter::Init(Building* building){

	Log->Write("INFO:\tInit Quickest Path Router Engine");
	GlobalRouter::Init(building);
	//	pBuilding=building;
	ReduceGraph();
	//ExpandGraph();
	//	DumpAccessPoints(981);
	//	exit(0);
	vector<string> rooms;
	//rooms.push_back("050");
	rooms.push_back("010");
	//rooms.push_back("020");
	//rooms.push_back("030");
	//rooms.push_back("040");
	//rooms.push_back("050");
	//rooms.push_back("060");
	//rooms.push_back("070");
	//rooms.push_back("080");
	//rooms.push_back("090");
	//rooms.push_back("100");
	//rooms.push_back("110");
	//rooms.push_back("120");
	//rooms.push_back("130");
	//rooms.push_back("140");
	//rooms.push_back("150");
	//rooms.push_back("outside");
	//WriteGraphGV("routing_graph.gv",FINAL_DEST_ROOM_040,rooms);
	WriteGraphGV("routing_graph.gv",FINAL_DEST_OUT,rooms);
	//DumpAccessPoints(1185);
	//DumpAccessPoints(1186);
	//DumpAccessPoints(1188);
	//DumpAccessPoints(1360);

	//	for(int i=0;i<pBuilding->GetAnzRooms();i++){
	//		cout <<i<<" : "<<pBuilding->GetRoom(i)->GetCaption()<<endl;
	//	}
	//	DumpAccessPoints(1352);
	//	DumpAccessPoints(1356);
	//	exit(0);
	Log->Write("INFO:\tDone with Quickest Path Router Engine!");
}

void QuickestPathRouter::SelectReferencePedestrian(Pedestrian* me, Pedestrian** myref, int exitID, int* flag){

	double jamThreshold=0.5;
	*flag=FREE_EXIT; // assume free exit

	Crossing* crossing=_building->GetGoal(exitID);

	bool done=false;
	double radius=3.0;//start radius in metres

	do{
		vector<Pedestrian*> queue;
		queue.reserve(250);
		GetQueueAtExit(crossing,jamThreshold,radius,queue);
		if(queue.size()==0){
			//check if I can see/reach the exit without much effort
			if(IsDirectVisibilityBetween(me,crossing)){
				*myref=NULL;
				*flag=FREE_EXIT;
			}else{
				*myref=NULL;
				*flag=UNREACHEABLE_EXIT;
			}
			// we should return here as there is no queue
			done=true;

		}else{

			double closestDistance=FLT_MAX;
			//select a reference pedestrian I can see
			for(unsigned int p=0;p<queue.size();p++){
				Pedestrian* ped = queue[p];
				if(IsDirectVisibilityBetween(me,ped)==false) continue;
				double dist= (ped->GetPos()-me->GetPos()).NormSquare();
				//cout<<"suspect found 1 @ "<< dist<< " { "<< closestDistance<<" }"<<endl;
				if(dist<closestDistance){
					closestDistance=dist;
					*myref=ped;
					*flag=REF_PED_FOUND; // at least one reference was found
					done=true;
				}
			}
		}

		// we could not find any visible reference so far, we widen our sight range
		// and to the same again
		radius=radius*2;
		if(radius>100){
			if(queue.size()>0){ // there were some ref pedes only not visibles
				*myref=NULL;
				*flag=UNREACHEABLE_EXIT;
				done=true;

			}else{
				*myref=NULL;
				*flag=UNREACHEABLE_EXIT;
				done=true;

				//FIXME: this should be remove if you reach a stable version.
				Log->Write("ERROR: QuickestPathRouter: reference ped cannot be found");
				printf("ERROR: reference ped cannot be found for ped %d within [%f] m  around the exit [%d]\n",me->GetPedIndex(),radius,crossing->GetIndex());
				//exit(EXIT_FAILURE);
			}
		}
	}while (done==false);

}

int QuickestPathRouter::GetCommonDestinationCount(AccessPoint* ap1, AccessPoint* ap2){
	const vector<AccessPoint*>& aps1 = ap1->GetConnectingAPs();
	const vector<AccessPoint*>& aps2 = ap2->GetConnectingAPs();

	vector<AccessPoint*> common;

	for(unsigned int i=0;i<aps1.size();i++){
		AccessPoint* from_AP=aps1[i];
		if(from_AP->GetID()==ap2->GetID()) continue;
		for(unsigned int j=0;j<aps2.size();j++){
			AccessPoint* to_AP=aps2[j];
			if(to_AP->GetID()==ap1->GetID()) continue;
			if(from_AP->GetID()==to_AP->GetID()){
				//only add if the destination is shorter than mine
				//if(ap2->GetDistanceTo(FINAL_DEST_OUT)<from_AP->GetDistanceTo(FINAL_DEST_OUT))
				//if(ap1->GetDistanceTo(FINAL_DEST_OUT)<from_AP->GetDistanceTo(FINAL_DEST_OUT))
				common.push_back(from_AP);
			}
		}
	}

	std::sort(common.begin(), common.end());
	common.erase(std::unique(common.begin(), common.end()), common.end());

	return common.size();
}

void QuickestPathRouter::GetQueueAtExit(Crossing* crossing, double minVel,
		double radius, vector<Pedestrian*>& queue){

	SubRoom* sbr1 = crossing->GetSubRoom1();
	SubRoom* sbr2 = crossing->GetSubRoom2();
	int exitID=crossing->GetIndex();
	double radius2=radius*radius;
	double minVel2=minVel*minVel;

	//if this is a hline
	if(sbr1==sbr2){
		sbr2=NULL;
	}

	if (sbr1){
		//double closestDistance=FLT_MAX;
		const vector<Pedestrian*>& peds = sbr1->GetAllPedestrians();
		for (unsigned int p=0;p<peds.size();p++){
			Pedestrian* ped = peds[p];
			if(ped->GetExitIndex()==exitID){
				if(ped->GetV().NormSquare()<minVel2){
					double dist= (ped->GetPos()-crossing->GetCentre()).NormSquare();
					//cout<<"suspect found 1 @ "<< dist<< " { "<< closestDistance<<" }"<<endl;
					if(dist<radius2){
						queue.push_back(ped);
					}
				}

			}
		}
	}

	if (sbr2){
		//double closestDistance=FLT_MAX;
		const vector<Pedestrian*>& peds = sbr2->GetAllPedestrians();
		for (unsigned int p=0;p<peds.size();p++){
			Pedestrian* ped = peds[p];
			if(ped->GetExitIndex()==exitID){
				if(ped->GetV().NormSquare()<minVel2){
					double dist= (ped->GetPos()-crossing->GetCentre()).NormSquare();
					//cout<<"suspect found 1 @ "<< dist<< " { "<< closestDistance<<" }"<<endl;
					if(dist<radius2){
						queue.push_back(ped);
					}
				}

			}
		}
	}
}

bool QuickestPathRouter::IsDirectVisibilityBetween(Pedestrian* ped, Pedestrian* ref){

	int ignore_ped1=ped->GetPedIndex();
	int ignore_ped2=ref->GetPedIndex();
	Crossing* ignore_crossing=_building->GetGoal(ref->GetExitIndex());

	int obstacles=GetObstaclesCountBetween(ped->GetPos(),ref->GetPos(),ignore_crossing,ignore_ped1,ignore_ped2);

	if(obstacles>4) return false;
	return true;
}

bool QuickestPathRouter::IsDirectVisibilityBetween(Pedestrian* myself, Crossing* crossing){

	int ignore_ped1=myself->GetPedIndex();
	int ignore_ped2=-1;//there is no second ped to ignore

	int obstacles=GetObstaclesCountBetween(myself->GetPos(),crossing->GetCentre(),crossing,ignore_ped1,ignore_ped2);

	if(obstacles>4) return false;
	return true;

}

int QuickestPathRouter::GetObstaclesCountBetween(const Point& p1, const Point& p2, Crossing* crossing,
		int ignore_ped1, int ignore_ped2){
	//first collect all pedestrians in that room

	SubRoom* sbr1 = crossing->GetSubRoom1();
	SubRoom* sbr2 = crossing->GetSubRoom2();
	Line visibilityLine = Line(p1,p2);

	int exitID=crossing->GetIndex();
	int obstacles=0;

	//if this is a hline
	if(sbr1==sbr2){
		sbr2=NULL;
	}

	if (sbr1){
		const vector<Pedestrian*>& peds = sbr1->GetAllPedestrians();
		for (unsigned int p=0;p<peds.size();p++){
			Pedestrian* ped = peds[p];

			//avoiding myself
			if(ped->GetPedIndex()==ignore_ped1) continue;
			if(ped->GetPedIndex()==ignore_ped2) continue;
			// pedestrian going in that direction are not obstacles to me
			if(ped->GetExitIndex()==exitID) continue;

			if(visibilityLine.IntersectionWithCircle(ped->GetPos())){
				obstacles++;
				if(obstacles>4) return obstacles;
			}

		}
	}

	if (sbr2){
		const vector<Pedestrian*>& peds = sbr2->GetAllPedestrians();
		for (unsigned int p=0;p<peds.size();p++){
			Pedestrian* ped = peds[p];

			//avoiging myself
			if(ped->GetPedIndex()==ignore_ped1) continue;
			if(ped->GetPedIndex()==ignore_ped2) continue;
			// pedestrian going in that direction are not obstacles to me
			if(ped->GetExitIndex()==exitID) continue;

			if(visibilityLine.IntersectionWithCircle(ped->GetPos())){
				obstacles++;
				if(obstacles>4) return obstacles;
			}

		}
	}


	return obstacles;
}


int QuickestPathRouter::isCongested(Pedestrian* ped){


	//collect the pedestrians within 1 metre radius
	vector<Pedestrian*> neighbourhood;
	double range=1.0;//1m

	_building->GetGrid()->GetNeighbourhood(ped,neighbourhood);


	std::vector<int> conflictings;
	std::vector<int>::iterator per;

	//int congested=1;
	int pedCrossing=0;

	//Debug::Messages("congested ( %hd ): [ ",myID);

	Point start=ped->GetPos();
	//looping on a circle
	for(double phi=0.0; phi<2*M_PI; phi+=0.1){

		Point end= start+Point(range*cos(phi), range*sin(phi));//translation

		Line line= Line(start, end);

		for(unsigned int p=0;p<neighbourhood.size();p++){
			Pedestrian* ref = neighbourhood[p];

			//skipping those in other rooms
			if(ped->GetUniqueRoomID()!=ref->GetUniqueRoomID()) continue;

			if((ped->GetPos()-ref->GetPos()).NormSquare()>1.0) continue;
			// do not add a pedestrian twice
			vector<int>::iterator per = find(conflictings.begin(), conflictings.end(), ref->GetPedIndex());
			if (per != conflictings.end()) continue;

			if(line.IntersectionWithCircle(ref->GetPos())==false) continue;

			const Point& pos1=ref->GetPos();
			Point pos2= start-pos1;
			Point vel1=ped->GetV();

			// only those  behind me
			if(pos2.ScalarP(vel1)) pedCrossing++;

			conflictings.push_back(ref->GetPedIndex());

		}

	}

	if(pedCrossing<4) return false;
	return true;
}


double QuickestPathRouter::GetEstimatedTravelTimeVia(Pedestrian* ped, int exitid){

	//select a reference pedestrian
	Pedestrian* myref=NULL;
	int flag=FREE_EXIT; //assume free exit
	SelectReferencePedestrian(ped,&myref,exitid,&flag);

	AccessPoint* ap=_accessPoints[exitid];

	// compute the time
	double time=FLT_MAX;

	// case of free exit
	if((myref==NULL)&& (flag==FREE_EXIT)){
		double t1 = (ped->GetPos()- ap->GetCentre()).Norm()/ped->GetV0Norm();
		// time to reach the AP
		//double t1 = (ped->GetPos()- ap->GetCentre()).Norm()/ped->GetV().Norm();

		//guess time from the Ap to the outside
		double t2 = (ap->GetDistanceTo(ped->GetFinalDestination()))/ped->GetV0Norm();

		time=t1+t2;
	}

	// case of unreachable exit
	if((myref==NULL)&& (flag==UNREACHEABLE_EXIT)){
		time= FLT_MAX;
	}

	// case of ref ped
	if((myref!=NULL) && (flag==REF_PED_FOUND)){

		//time to reach the reference
		//double t1= (ped->GetPos()- myref->GetPos()).Norm()/ped->GetV().Norm();
		double t1= (ped->GetPos()- myref->GetPos()).Norm()/ped->GetV0Norm();

		//if(myref->GetV().Norm()==0.0){
		//	cout<<"bye"<<endl; exit(0);
		//}
		//time for the reference to get out
		double t2=(myref->GetPos() -  ap->GetCentre()).Norm()/myref->GetV().Norm();

		//guess time from the Ap to the outside
		double t3 = (ap->GetDistanceTo(ped->GetFinalDestination()))/ped->GetV().Norm();

		time=t1+t2+t3;

	}

	if((myref==NULL) && (flag==REF_PED_FOUND)){
		cout<<" Fatal Error in Quickest Path Router"<<endl;
		cout<<" reference pedestrians is NULL"<<endl;
		exit(EXIT_FAILURE);

	}

	return time;
}

void QuickestPathRouter::Redirect(Pedestrian* ped){

	int preferredExit=ped->GetExitIndex();

	double preferredExitTime=FLT_MAX;
	int quickest=-1;
	double minTime=FLT_MAX;

	//only redirect to other final exits in the actual room.
	// if there is no final exit in the sight range,
	// then no redirection is possible

	// collect the possible alternatives
	Room* room=_building->GetRoom(ped->GetRoomID());
	SubRoom* sub=room->GetSubRoom(ped->GetSubRoomID());

	const vector<int>& goals=room->GetAllTransitionsIDs();

	for(unsigned int g=0;g<goals.size();g++){
		int exitid=goals[g];
		AccessPoint* ap=_accessPoints[exitid];

		//only final are accounted
		if(ap->isFinalDestination()==false) continue;

		//check if I can reach that exit, there should exits a direct line
		// segment connecting the two APs/goals
		const Point& p1 = _building->GetGoal(exitid)->GetCentre();
		const Point& p2 = ped->GetPos();
		Line segment = Line(p1,p2);

		bool isVisible=true;
		//first walls
		const vector<Wall>& walls= sub->GetAllWalls();

		for(unsigned int b=0;b<walls.size();b++){
			if(segment.IntersectionWith(walls[b])==true) {
				isVisible=false;
				//cout<<"failed: walls "<<b<<" in subroom " << sub->GetSubRoomID()<<endl;
				break;
			}
		}
		if(isVisible==false) continue;

		double time=GetEstimatedTravelTimeVia(ped, exitid);

		if(time<minTime){
			minTime=time;
			quickest=exitid;
		}

		//printf(" ped [%d] checking [%d] ---> [%f]\n",ped->GetPedIndex(),exitid,time);
		//also save the time for the default destinations for later comparison
		if (exitid==preferredExit){
			preferredExitTime=time;
		}

	}
	//compare it with my preferred/current (shortest nearest)
	if(quickest!=preferredExit){
		double cba = CBA(gain(preferredExitTime),gain(minTime));
		//cout<<"cba:" <<cba<<endl;
		if (cba>0.010){
			ped->SetExitIndex(quickest);
			ped->SetExitLine(_building->GetGoal(quickest));
		}

	}
}
