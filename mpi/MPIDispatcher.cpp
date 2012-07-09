/*
 * MPIDispatcher.cpp
 *
 *  Created on: Feb 14, 2011
 *      Author: piccolo
 */


#include <mpi.h>

#include "MPIDispatcher.h"



using namespace std;

MPIDispatcher::MPIDispatcher(int rank, int commSize) {
	pRank=rank;
	pCommSize=commSize;
	pFriends.clear();
	pWorkingArea.clear();
	pShutdown=0; // 1 for shutdown=true
	//for(int i=0;i<25;i++)pProcessorMap[i]=-1;
	pBuilding=NULL;
}

MPIDispatcher::~MPIDispatcher() {

}

//TODO: define appropriate Tags
void MPIDispatcher::Initialise(Building* building){

	pBuilding=building;
	int nRooms=pBuilding->GetAnzRooms();

	//FIXME:
	// Why is the following not working???
	// int* pProcessorMap=new int[nRooms];
	// and also
	//	for(int i=0;i<255;i++)pProcessorMap[i]=-1;
	//after int pP[256]
	//rest the processor map
	//pProcessorMap=new int[nRooms];
	for(int i=0;i<nRooms;i++)pProcessorMap[i]=-1;
	//	for(int i=0;i<90;i++)pProcessorMap[i]=-1;


	//special case with only one Processor
	if(pCommSize==1){
		cout <<" I am the only process"<<endl;
		cout <<" I will do the complete work"<<endl;
		cout <<" I am having "<<nRooms<<" rooms to work on"<<endl;

		for (int r  = 0; r  < nRooms; r++) {
			pProcessorMap[r]=pRank;
			pWorkingArea.push_back(r);
		}
		building->SetMPIDispatcher(this);
		return;
	}

	// starting from here mpi is supported
	MPI_Status status;

	//master
	if(pRank==0){

		if (nRooms < pCommSize) {
			printf("MPI::WARNING there are more processors than room \n");
			printf("MPI::WARNING this is actually not (fully) supported \n");

			for (int r = 0; r < nRooms; r++) {
				pProcessorMap[r]=r;
			}

			//reserve the first room for the master
			// process pid will operate on room pid
			pWorkingArea.push_back(pProcessorMap[pRank]);

			for (int pid = 1; pid < pCommSize; pid++) {
				int roomList = pProcessorMap[pid];
				MPI_Send(&roomList, 1, MPI_INT, pid, 99, MPI_COMM_WORLD);
				MPI_Send(&pProcessorMap, nRooms, MPI_INT, pid, 99, MPI_COMM_WORLD);
			}
		}
		else {

			//work portion for each Processor
			int partSize = floor( (((double)nRooms) / pCommSize ) + 0.5);

			// reserve work for the master/ I am not lazy
			{
				int pid=0;
				int start = pid*partSize;
				int end = (pid + 1) * partSize - 1;
				for (int p = start; p <= end; ++p) {
					pWorkingArea.push_back(p);
					pProcessorMap[p]=pid;
				}
			}

			for(int pid=1;pid<pCommSize;pid++){
				//int partSize =  nRooms / pCommSize;
				int start = pid*partSize;
				int end = (pid + 1) * partSize - 1;
				// last process may do more
				if ((pid == pCommSize - 1)) end = pBuilding->GetAnzRooms() - 1;

				// send the room start-->end to the process pid
				const int nSize=end-start+1;
				int roomList[nSize];
				for (int p = start; p <= end; ++p){
					roomList[p-start]=p;
					pProcessorMap[p]=pid;
				}
				//TODO: the list can be automatically retrieved from the processor map
				MPI_Send(&roomList, nSize, MPI_INT, pid, 99, MPI_COMM_WORLD);
			}

			for(int i=0;i<nRooms;i++){
				printf(" sending Room [%d] --> Proc [%d]\n",i,pProcessorMap[i]);
			}

			for(int pid=1;pid<pCommSize;pid++){
				MPI_Send(&pProcessorMap, nRooms, MPI_INT, pid, 99, MPI_COMM_WORLD);
			}
		}
	}
	else //worker
	{
		//receive max 256 rooms
		int nSize=20;
		int roomList[nSize];
		for(int i=0;i<nSize;i++)roomList[i]=-1;
		MPI_Recv(&roomList, 256, MPI_INT,0,99,MPI_COMM_WORLD, &status);

		for(int i=0;i<nSize;i++){
			if(roomList[i]!=-1) pWorkingArea.push_back(roomList[i]);
		}

		if(pWorkingArea.empty()){
			printf("I [%d] am shutting down, I got no work to do\n",pRank);
		}

		//receive the processors map
		MPI_Recv(&pProcessorMap, 256, MPI_INT,0,99,MPI_COMM_WORLD, &status);

		//for(int i=0;i<nRooms;i++){
		//	printf(" Room [%d] --> Proc [%d]\n",i,pProcessorMap[i]);
		//}
	}


	// now I determine my friends: processes with which i am obliged to exchange information
	//MPI_Barrier(MPI_COMM_WORLD);

	//take each room
	for (int i=0;i<(int)pWorkingArea.size();i++){
		//take all transitions in that room
		int rID=pWorkingArea[i];

		Room* room = pBuilding->GetRoom(rID);

		for (int j = 0; j < room->GetAnzSubRooms(); j++) {

			//pRoomscount++; //get the number of rooms/subrooms
			SubRoom* sub = room->GetSubRoom(j);
			const vector<int>& exitsInSubroom =sub->GetAllGoalIDs();

			for(unsigned int l=0;l< exitsInSubroom.size();l++){
				int id=exitsInSubroom[l];
				SubRoom* sb1=pBuilding->GetRouting()->GetGoal(id)->GetSubRoom1();
				SubRoom* sb2=pBuilding->GetRouting()->GetGoal(id)->GetSubRoom2();
				//get the processor responsible for that room

				// skip hlines
				if(sb1==sb2) continue;

				if(sb1 && sb1->GetRoomID()!=rID){
					int pid=pProcessorMap[sb1->GetRoomID()];
					// check if already friend
					if( (std::find(pFriends.begin(), pFriends.end(), pid) == pFriends.end()) && (pid!=pRank))
						pFriends.push_back(pid);
				}
				if(sb2 && sb2->GetRoomID()!=rID){
					int pid=pProcessorMap[sb2->GetRoomID()];
					// check if already friend
					if((std::find(pFriends.begin(), pFriends.end(), pid) == pFriends.end()) && (pid!=pRank))
						pFriends.push_back(pid);
				}
			}
		}

	}
	//MPI_Barrier(MPI_COMM_WORLD);
	//exit(0);
	building->SetMPIDispatcher(this);

	// compute the ghost cells
	//ComputeGhostArea();

	// synchronizing the team
	MPI_Barrier(MPI_COMM_WORLD);
}




// synchronize ghost areas with neighborhood processes
// send and receive Neighboourhood
void MPIDispatcher::Update(){

#ifdef MANUAL
	VT_TRACER("mpi_update");
#endif

	//nothing to do for a single processor
	if (pCommSize==1) {
		pBuilding->UpdateGrid();
		return;
	}

	//clear should be combined with update
	ClearGhostPedestrians();

	//FIXME:TODO get rid of that
	//Grid::update(pBuilding);
	pBuilding->UpdateGrid();

	// number of peds info to be transfered
	// the data is [ID roomID subroomID V0Norm xPos yPos xVel yVel cosPhi sinPhi]
	int nValues=14;

	//mapping each processor to a data block
	map <int, vector <Pedestrian*> > dataToSend = map <int, vector <Pedestrian*> >();

	//reserve place for the data
	// now send to all friends
	for(unsigned int t=0;t<pFriends.size();t++){
		int procID=pFriends[t];
		dataToSend[procID].reserve(200);
	}

	// loop over all rooms in the working area and collect the data to send
	for (unsigned int wa=0; wa< pWorkingArea.size(); wa++){

		int roomID=pWorkingArea[wa];
		Room* room = pBuilding->GetRoom(roomID);

		//take all transitions in that room
		const vector<int>& trans=room->GetAllTransitionsIDs();

		for(unsigned int t=0;t<trans.size();t++){

			//get the transition
			int transID=trans[t];
			//get the room at the other side of the transition
			Transition* tr=((Transition*)pBuilding->GetRouting()->GetGoal(transID));
			// no data exchange with the outside
			if(tr->IsExit()) continue;

			// get the corresponding processor from the communicator
			int otherRoom=tr->GetOtherRoom(roomID)->GetRoomID();
			int procID=pProcessorMap[otherRoom];


			//provide the old neighborhood
			getNeighbourhood(transID,dataToSend[procID]);

			//vector<Pedestrian*> neighbourhood=vector<Pedestrian*>();


			vector<Pedestrian*> pedToTransfer = vector<Pedestrian*>();
			pBuilding->GetPedestriansTransferringToRoom(otherRoom,pedToTransfer);

			//avoid adding a ped twice. This is the case if
			// the pedestrian just changed the room
			for(unsigned int i=0;i<pedToTransfer.size();i++){
				// check if already added
				Pedestrian* ped=pedToTransfer[i];
				std::vector<Pedestrian*>::iterator it=
						std::find(dataToSend[procID].begin(), dataToSend[procID].end(),ped);
				if(it==dataToSend[procID].end() )
					dataToSend[procID].push_back(ped);
			}

			//neighbourhood.insert(neighbourhood.begin(),pedToTransfer.begin(),pedToTransfer.end());

			//dataToSend[procID].insert(dataToSend[procID].end(),neighbourhood.begin(),neighbourhood.end());
			// pack the data
		}
	}// end all rooms

	//new appraoch that will make the use of Isend possible
	//double **newdata= new double*[pFriends.size()];

	// now send to all friends
	for(unsigned int t=0;t<pFriends.size();t++){

		int procID=pFriends[t];

		int msgLength=nValues*dataToSend[procID].size();

		// no neighborhood
		// at least one dummy ped has to be sent
		if(msgLength==0) msgLength=1;
		double* data=new double[msgLength]();

		//part of the new appraoch
		//newdata[t]=new double[msgLength]();
		//double*data=data[t];

		if(dataToSend[procID].size()==0){ // there is no data to send, create dummy one
			for(int i=0;i<msgLength;i++) data[i]=-1;
		}
		else
		{
			for(unsigned int p=0;p<dataToSend[procID].size();p++){
				int offset=p*nValues;
				Pedestrian* ped=dataToSend[procID][p];
				data[offset+0]=ped->GetPedIndex();
				data[offset+1]=ped->GetRoomID();
				data[offset+2]=ped->GetSubRoomID();
				data[offset+3]=ped->GetV0Norm();
				data[offset+4]=ped->GetPos().GetX();
				data[offset+5]=ped->GetPos().GetY();
				data[offset+6]=ped->GetV().GetX();
				data[offset+7]=ped->GetV().GetY();
				data[offset+8]=ped->GetEllipse().GetCosPhi();
				data[offset+9]=ped->GetEllipse().GetSinPhi();
				data[offset+10]=ped->GetEllipse().GetAv();
				data[offset+11]=ped->GetEllipse().GetAmin();
				data[offset+12]=ped->GetEllipse().GetBmin();
				data[offset+13]=ped->GetEllipse().GetBmax();
			}
		}

		//send the packed message to the corresponding processor
		SendMessage(procID,&data,msgLength);
		delete[]data;

	}



	// now receive from all friends
	for(unsigned int t=0;t<pFriends.size();t++)	{

		double *data=NULL;
		int msgLength=0;
		int source=pFriends[t];
		RecvMessage(source, &data, &msgLength);

		// parse the receive message
		int nPeds=msgLength/nValues;

		for(int p=0;p<nPeds;p++){
			int offset=p*nValues;

			int id =	data[offset+0];
			if(id==-1) continue; // no neighborhood
			int roomID=data[offset+1];
			int subroomID=data[offset+2];
			double V0Norm=data[offset+3];
			Point pos (data[offset+4],data[offset+5]);
			Point vel (data[offset+6],data[offset+7]);
			double cosPhi=data[offset+8];
			double sinPhi=data[offset+9];
			double atau = data[offset+10];
			double amin = data[offset+11];
			double bmax = data[offset+12];
			double bmin = data[offset+13];

			// create the incoming pedestrian
			Pedestrian* ped = new Pedestrian();
			// PedIndex
			ped->SetPedIndex(id);
			// a und b setzen muss vor v0 gesetzt werden, da sonst v0 mit Null überschrieben wird
			Ellipse E = Ellipse();
			E.SetAv(atau);
			E.SetAmin(amin);
			E.SetBmax(bmax);
			E.SetBmin(bmin);
			E.SetCosPhi(cosPhi);
			E.SetSinPhi(sinPhi);
			ped->SetEllipse(E);
			ped->SetV0Norm(V0Norm);
			ped->SetPos(pos);
			ped->SetV(vel);
			ped->SetRoomID(roomID,pBuilding->GetRoom(roomID)->GetCaption());
			ped->SetSubRoomID(subroomID);
			ped->SetExitIndex(-1);

			SubRoom* sub = pBuilding->GetRoom(roomID)->GetSubRoom(subroomID);

			//important
			//pBuilding->GetRouting()->FindExit(ped);

//			Pedestrian* p1 =GetPedWithID(id);
//			if(p1){
//				printf("ped [%d]  coming from Proc [%d] is already in room/subroom [%s][%d/%d] on processor %d \n",id,source,
//						pBuilding->GetRoom(roomID)->GetCaption().c_str(),roomID,subroomID,pRank);
//				printf("FIXME!\n");
//				exit(0);
//				continue;
//			}

			// the ped should be added to my working set because
			// they are  centrally collected in the building  object
			if(IsMyWorkingArea(roomID)){
				printf("incoming [%d] from Proc [%d]\n",id,source);
				pBuilding->GetRouting()->FindExit(ped);
				pBuilding->AddPedestrian(ped);
			}

			// setzen
			sub->AddPedestrian(ped);
			pBuilding->GetGrid()->Update(ped);
		}
		delete[]data;
	}

	MPI_Status status;
	//MPI_Request request;

	if(pRank==0){
		//initialisierung
		int totalPedsCount=pBuilding->GetAnzPedestrians();
		//printf(" total of [%d] peds on [%d]\n",totalPedsCount,pRank);
		//first my area
		for(int pid=1;pid<pCommSize;pid++){
			int pedsOnProci=0;
			MPI_Recv(&pedsOnProci, 1, MPI_INT,pid,TAG_PEDSCOUNT,MPI_COMM_WORLD, &status);
			totalPedsCount+=pedsOnProci;
			//printf("receiving [ %d ] from [%d] Total is [%d]\n",pedsOnProci,pid,totalPedsCount);
		}
		pShutdown=(totalPedsCount>0)?0:1;

		for(int pid=1;pid<pCommSize;pid++){
			MPI_Send(&pShutdown, 1, MPI_INT, pid, TAG_SHUTDOWN, MPI_COMM_WORLD);
		}
	}
	else
	{
		// send number of remaining pedestrians to the master
		int pedsCount=pBuilding->GetAnzPedestrians();
		//printf(" total of [%d] peds on [%d]\n",pedsCount,pRank);
		MPI_Send(&pedsCount, 1, MPI_INT, 0, TAG_PEDSCOUNT, MPI_COMM_WORLD);
		// receive the signal to continue or not
		MPI_Recv(&pShutdown, 1, MPI_INT,0,TAG_SHUTDOWN,MPI_COMM_WORLD, &status);
	}

	//delete the newdata
	//for(unsigned int t=0;t<pFriends.size();t++)	{
	//	delete newdata[t];
	//}
	//delete [] newdata;

	// TODO: do I really need/want that?
	//MPI_Barrier(MPI_COMM_WORLD);
}


void MPIDispatcher::getNeighbourhood(int crossID,
		vector<Pedestrian*>& neighbourhood){

	double stepSize=3.0;//metres
	//double stepSize=0.5;//metres

	Line * trans=pBuilding->GetRouting()->GetAllGoals()[crossID];
	int npSeg=trans->Length()/stepSize+1; // in step of 2 metres
	const Point& A=trans->GetPoint1();
	const Point& B=trans->GetPoint2();



	// get all point on the transition in step npSeg.
	for(int i=0;i<npSeg;i++){
		Point pt;
		//special case for small transition
		if(npSeg==1){
			pt = (A+B)*0.5;
		}else{
			pt = A*(1-(double)i/(npSeg-1)) + B*((double)i/(npSeg-1));
		}
		vector<Pedestrian*> peds = vector<Pedestrian*>();
		peds.reserve(200);
		pBuilding->GetGrid()->getNeighbourhood(pt,peds);

		// check the grid output and verify that you haven't added one ped too much
		// and also only return peds that are in my area
		for(unsigned int p=0;p<peds.size();p++){
			// check if already added
			std::vector<Pedestrian*>::iterator it;
			it = std::find(neighbourhood.begin(), neighbourhood.end(),peds[p]);
			if(it==neighbourhood.end() && (IsMyWorkingArea(peds[p]->GetRoomID())))
				neighbourhood.push_back(peds[p]);
		}
	}

	//	//Dump the neighbourhood
	//	if(pRank==-1){
	//		if(neighbourhood.size()>0)	cout<<"neighborhood [ ";
	//
	//		for (unsigned int p=0;p<neighbourhood.size();p++){
	//			Pedestrian* ped =neighbourhood[p];
	//			//		ped->Dump(ped->GetPedIndex());
	//			cout<<ped->GetPedIndex()<<" ";
	//		}
	//
	//		if(neighbourhood.size()>0)  cout<<" ]"<<endl;
	//	}
}

// dump each process (or)
void MPIDispatcher::Dump(){
	cout <<" Processor ID: "<<pRank<<endl;
	cout <<" Working Area: ";
	for (unsigned int i=0;i<pWorkingArea.size();i++) cout<<pBuilding->GetRoom(pWorkingArea[i])->GetCaption() <<" ";
	cout<<endl;
	cout<<" Friends: ";
	for (unsigned int i=0;i<pFriends.size();i++) cout<<pFriends[i]<<" ";
	cout<<endl<<endl;;
}



// look if the pedestrians with id is present
// return NULL otherwise
// so check before processing

Pedestrian* MPIDispatcher::GetPedWithID(int id){
	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* room = pBuilding->GetRoom(i);
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
				Pedestrian* ped = sub->GetPedestrian(k);
				if(id==ped->GetPedIndex()){
					//ped->Dump(id);
					printf("[%d] is in the room/subroom [%s][%d,%d]\n",ped->GetPedIndex(),room->GetCaption().c_str(),i,j);
					return ped;
				}
			}
		}
	}
	return NULL;
}

// clear all pedestrians in the ghost area from other rooms
void MPIDispatcher::ClearGhostPedestrians(){
	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		if(IsMyWorkingArea(i)==true) continue;
		Room* room = pBuilding->GetRoom(i);
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			room->GetSubRoom(j)->ClearAllPedestrians();
		}
	}
}

// CAUTION:
// all send are non blocking
void MPIDispatcher::SendMessage(int to_procID, double** msg, int msgLength){
	//MPI_Request request; //??
	//MPI_Isend(*msg, msgLength, MPI_DOUBLE, to_procID, TAG_DEFAULT, MPI_COMM_WORLD,&request);

	MPI_Send(*msg, msgLength, MPI_DOUBLE, to_procID, TAG_DEFAULT, MPI_COMM_WORLD);
}

// CAUTION
// all recv are blocking
void MPIDispatcher::RecvMessage(int from_procID, double** msg, int* msgLength){
	MPI_Status status;

	//size of the message
	MPI_Probe(from_procID, TAG_DEFAULT, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_DOUBLE, msgLength);

	//message
	//	*msg = malloc((*msgLength)*sizeof(double));
	*msg = new double[*msgLength]();
	MPI_Recv(*msg, *msgLength, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

}

bool MPIDispatcher::Shutdown(){
	if(pCommSize==1){
		pShutdown=(pBuilding->GetAnzPedestrians()>0)?0:1;
	}
	return pShutdown;
}


bool MPIDispatcher::IsMyWorkingArea(int roomID){

	if (pProcessorMap[roomID]==pRank){
		return true;
	}
	return false;

}

const vector<int>& MPIDispatcher::GetMyWorkingArea() const {
	return pWorkingArea;
}

int MPIDispatcher::GetMyRank(){
	return pRank;
}

// return the toal number of pedestrian in the simulation
// across all processors
int MPIDispatcher::GetGlobalPedestriansCount(){

	//only one processor
	if(pCommSize==1){
		return pBuilding->GetAnzPedestrians();
	}


	int totalPedsCount=0;
	MPI_Status status;

	// the master collects all info and send them back
	if(pRank==0){
		//initialisierung
		totalPedsCount=pBuilding->GetAnzPedestrians();
		//printf(" total of [%d] peds on [%d]\n",totalPedsCount,pRank);
		//first my area
		for(int pid=1;pid<pCommSize;pid++){
			int pedsOnProci=0;
			MPI_Recv(&pedsOnProci, 1, MPI_INT,pid,TAG_PEDSCOUNT,MPI_COMM_WORLD, &status);
			totalPedsCount+=pedsOnProci;
			//printf("receiving [ %d ] from [%d] Total is [%d]\n",pedsOnProci,pid,totalPedsCount);
		}

		for(int pid=1;pid<pCommSize;pid++){
			MPI_Send(&totalPedsCount, 1, MPI_INT, pid, TAG_PEDSCOUNT, MPI_COMM_WORLD);
		}
	}
	else
	{
		// send number of remaining pedestrians to the master
		int pedsCount=pBuilding->GetAnzPedestrians();
		//printf(" total of [%d] peds on [%d]\n",pedsCount,pRank);
		MPI_Send(&pedsCount, 1, MPI_INT, 0, TAG_PEDSCOUNT, MPI_COMM_WORLD);
		// receive the signal to continue or not
		MPI_Recv(&totalPedsCount, 1, MPI_INT,0,TAG_PEDSCOUNT,MPI_COMM_WORLD, &status);
	}

	return totalPedsCount;
}
