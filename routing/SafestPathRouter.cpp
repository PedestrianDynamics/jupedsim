/**

 * @file    SafestPathRouter.cpp
 * Created on: 29.11.2013
 * Copyright (C) <2009-2013>
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
 * This class computes the safest path for pedestrians based on information
 * received from FDS
 *
 */


#include "SafestPathRouter.h"

#include "../pedestrian/Pedestrian.h"

#define UPDATE_FREQUENCY 10 // in seconds

using namespace std;

SafestPathRouter::SafestPathRouter() {

	//_finalLineEvac = new double [numberOfSection];


}

SafestPathRouter::~SafestPathRouter() {

}

void SafestPathRouter::Init(Building* building) {
	//Load the FDS file info

	//handle over to the global router engine
	GlobalRouter::Init(building);


	int numberOfSubroom=0;

	for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
		Room* room = _building->GetRoom(i);
		for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {

			SubRoom* sub = room->GetSubRoom(j);
			if(sub->GetType()=="floor")
				numberOfSubroom=numberOfSubroom+room->GetNumberOfSubRooms();
		}
	}

	 peopleAtSection = new double [numberOfSubroom];

	 rR = new double*[numberOfSubroom];
	 	for (int i = 0; i < numberOfSubroom; ++i) {
	 		rR[i] = new double[numberOfSubroom];
	 	}
}


int SafestPathRouter::FindExit(Pedestrian* p) {
	if(ComputeSafestPath(p)==-1) {
		//Log->Write(" sdfds");
	}
	//handle over to the global router engine
	return GlobalRouter::FindExit(p);
}


int SafestPathRouter::ComputeSafestPath(Pedestrian* p)
{

	// adjust the update frequency
	double diff= fabs((int)p->GetGlobalTime() - p->GetGlobalTime());
	if (diff>0.015) return -1;
	if((((int)p->GetGlobalTime())%UPDATE_FREQUENCY)!=0) return -1;


	/*
	if(p->GetGlobalTime()>0)
	{
		if (p->GetRoomID()==0 && p->GetSubRoomID()==2)
		{
			// p->SetFinalDestination(0);
			int TotalNumberofPedInsection2=_building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllPedestrians().size();
			TotalNumberofPedInsection=TotalNumberofPedInsection2;
		}
	}


	if (p->GetGlobalTime()>64)
	{
		if(p->GetFinalDestination()==1)
			if (p->GetID()==9)
			{
				//Point position = p->GetPos();
				p->SetFinalDest<group group_id="3" room_id="6" subroom_id="6" number="10" goal_id="1" router_id="1" route_id=""/> destination of "<< p->GetID()<<"from "<<p->GetFinalDestination()<< " to 1 at time ";
				p->ClearMentalMap(); // reset the destination
				cout<<p->GetGlobalTime()<<endl;
				//getc(stdin);
			}
	}

	//p->GetPos()==1
	 */
	main_3(p);
	main_2();
	main_1(p);



	return -1;
}



int SafestPathRouter::GetAgentsCountInSubroom( int roomID, int subroomID)
{
	return _building->GetRoom(roomID)->GetSubRoom(subroomID)->GetAllPedestrians().size();
}






void SafestPathRouter::Initialize(){
	ReadMatrixFromFDS();
}


void SafestPathRouter::ReadMatrixFromFDS()
{
}



void SafestPathRouter::main_3(Pedestrian* p)
{



	if (p->GetRoomID()==0 && p->GetSubRoomID()==0)
	{
		peopleAtSection[0]=GetAgentsCountInSubroom(0,0)-1;
	}
	else
	{
		peopleAtSection[0]=GetAgentsCountInSubroom(0,0);
	}

	//////


	if (p->GetRoomID()==0 && p->GetSubRoomID()==2)
	{
		peopleAtSection[1]=GetAgentsCountInSubroom(0,1)-1;
	}
	else
	{
		peopleAtSection[1]=GetAgentsCountInSubroom(0,1);
	}

	//////


	if (p->GetRoomID()==0 && p->GetSubRoomID()==3)
	{
		peopleAtSection[2]=GetAgentsCountInSubroom(0,2)-1;
	}
	else
	{
		peopleAtSection[2]=GetAgentsCountInSubroom(0,2);
	}





	double dMatrixPreEvac[1][3]={0.0125,0.0125,0.00833};








	//double dMatrixPreEvac[1][11]={0.000574,0.000328,0.000589,0.000651,0.000486,0.003576,0.003576,0.001171,0.000976,0.000221,0.000385};
	//double dMatrixPreEvac[1][11]={0.003575,0.000328,0.003575,0.000651,0.000486,0.003575,0.003575,0.001171,0.000976,0.000221,0.000385};


	//double dMatrixPreEvac[1][11]={0.000573,0.000328,0.000589,0.000651,0.000486,0.003574,0.003575,0.001171,0.000976,0.000221,0.000385};
	//double dMatrixPreEvac[1][11]={0.000573,0.000328,0.000589,0.000651,0.000486,0.003574,0.003575,0.001171,0.000976,0.000221,0.000385};

	//double dMatrixPreEvac[1][11]={0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573};





	int i=0;

	for(int j = 0; j < 3; j ++) //
	{
		dFinalLineEvac[i][j]= peopleAtSection[j] * dMatrixPreEvac[i][j];
	}

	// Print out final distance matrix

	//	int vertices = 3;
	//	for(int j = 0; j < vertices; j++)
	//		cout << dFinalLineEvac[i][j] << " ";
	//		cout<<p->GetGlobalTime()<<endl;
	//		cout << endl;
}








void SafestPathRouter::main_2(){

	//double dFinalLength[1][11]={0.328,0.569,0.328,0.414,0.586,0.328,0.328,1.000,0.276,0.759,0.741};
	//double dFinalLength[1][11]={0.638,0.569,0.534,0.414,0.586,0.328,0.328,1.000,0.276,0.759,0.741};


	double dFinalLength[1][11]={0.666,0.666,1.0};


	//double dFinalLength[1][11]={0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638};
	//double dFinalLength[1][11]={37.0,33.0,31.0,24.0,34.0,19.0,19.0,58.0,16.0,44.0,43.0};
	double dFinalLineOFP[1][11]={0,0,0};
	double iNt1[1][11]={0,0,0,0,0,0,0,0,0,0,0};
	double iNt2[1][11]={0,0,0,0,0,0,0,0,0,0,0};
	double iNt3[1][11]={0,0,0,0,0,0,0,0,0,0,0};
	double iNt4[1][11]={0,0,0,0,0,0,0,0,0,0,0};
	double iNt5[1][11]={0,0,0,0,0,0,0,0,0,0,0};
	double iNt6[1][11]={0,0,0,0,0,0,0,0,0,0,0};
	double iNt7[1][11]={0,0,0,0,0,0,0,0,0,0,0};
	int i=0;


	for (int j=0; j<3; j++)
	{
		iNt1[i][j]= dFinalLineEvac[i][j] * 1;
		iNt2[i][j]= dFinalLineOFP[i][j] * 1;
		iNt3[i][j]= dFinalLength[i][j] * 1;
	}



	for (int j=0; j<3; j++)
	{
		iNt4[i][j]= iNt1[i][j] * iNt1[i][j];
		iNt5[i][j]= iNt2[i][j] * iNt2[i][j];
		iNt6[i][j]= iNt3[i][j] * iNt3[i][j];
	}


	for (int j=0; j<3; j++)
	{
		iNt7[i][j]=iNt4[i][j] + iNt5[i][j] + iNt6[i][j];
	}


	for (int j=0; j<11; j++)
	{
		rR[i][j]=sqrt(iNt7[i][j]);
	}




	// Print out final distance matrix

	//	for(int j = 0; j < 11; j++)
	//		cout << rR[i][j] << " ";
	//	cout << endl;
}



void SafestPathRouter::main_1(Pedestrian* p)
{
	// Initialize
	int vertices = 4;
	vector<vector<double> > a(vertices, vector<double>(vertices,999));
	// initialize diagonal
	for(int i=0; i < vertices; i++)
		a[i][i]=0;


	// initialize distances
	a[0][1]=rR[0][0];
	a[1][2]=rR[0][1];
	a[1][3]=rR[0][2];
	//
	a[1][0]=rR[0][0];
	a[2][1]=rR[0][1];
	a[3][1]=rR[0][2];



	// Floyd-Warshal
	// Add nodes between (first 1 then 2, 3 till n) and look if
	// distance is shorter
	for(int k = 0; k < vertices; k++)
		for(int i = 0; i < vertices; i++)
			for(int j = 0; j < vertices; j++)
				if(a[i][j]>a[i][k]+a[k][j])
					a[i][j]=a[i][k]+a[k][j];

	//Print out final distance matrix

	//for(int i = 0; i < vertices; i++){
	//	for(int j = 0; j < vertices; j++)
	//		cout << a[i][j] << " ";

	//		cout << endl;
	//		cout<<p->GetGlobalTime()<<endl;
	//	}



	//double g3_0=a[0][2];// From node 3 to 0 (0 is the goal O)
	//double g3_1=a[1][2];// From node 3 to 1 (1 is the goal 1)
	double g1_3=a[0][2];
	double g1_4=a[0][3];




	//cout<<"The g5_0: "<<g5_0<<"\n";
	//cout<<"The g5_1: "<<g5_1<<"\n";
	//cout<<p->GetGlobalTime()<<endl;

	/*

		bool a1 = true;

		while (a1 == true)
		{

			if (g5_0>=g5_1)
			{
				p->SetFinalDestination(1);
				p->ClearMentalMap();

			}
			else
			{
				p->SetFinalDestination(0);
				p->ClearMentalMap();
				a1=false;
			}
		}
		}
	 */






	for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
			Room* room = _building->GetRoom(i);
			for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {

				SubRoom* sub = room->GetSubRoom(j);
				int room;
			}
		}


	int roomID=p->GetRoomID();
	int subroomID=p->GetSubRoomID();
	int nGoals =_building->GetNumberOfGoals();

	//rR[subroomID][subroomID]=

	//if (p->GetRoomID()==0 && p->GetSubRoomID()==0)
	{
		if (g1_3>=g1_4)
		{
			p->SetFinalDestination(1);
			p->ClearMentalMap();
		}
		else
		{
			p->SetFinalDestination(0);
			p->ClearMentalMap();
		}
	}


}










//	if (p->GetRoomID()==6 && p->GetSubRoomID()==6)
//	{
//	p->SetFinalDestination(0);
//	}




/*
		if (p->GetID()==1)
		{
			if (g5_0>=g5_1)
			{
				p->SetFinalDestination(1);
				p->ClearMentalMap(); // reset the destination
				//cout<<"The g5_0 is the best: "<< g5_0 <<"\n";
			}
			else{
				p->SetFinalDestination(0);
				p->ClearMentalMap(); // reset the destination
				//cout<<"The g5_1 is the best: "<< g5_1 <<"\n";
			}
		}




		//if (p->GetRoomID()==0 && p->GetSubRoomID()==2)
		//cout<<p->GetGlobalTime()<<endl;


			if (p->GetRoomID()==6 && p->GetSubRoomID()==6)
			{
				int TotalNumberofPedInsection;
				// p->SetFinalDestination(0);
				int TotalNumberofPedInSection2=_building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllPedestrians().size();
				TotalNumberofPedInSection=TotalNumberofPedInsection2;
				cout<<TotalNumberofPedInSection<<endl;
				cout<<p->GetGlobalTime()<<endl;
			}

 */



