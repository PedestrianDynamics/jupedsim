/**
 * \file        SafestPathRouter.cpp
 * \date        Nov 29, 2013
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
 * This class computes the safest path for pedestrians based on information
 * received from FDS
 *
 *
 **/


#include "AccessPoint.h"
#include "SafestPathRouter.h"
#include "../geometry/SubRoom.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <map>


#include "../pedestrian/Pedestrian.h"

#define UPDATE_FREQUENCY 1 // in seconds

using namespace std;

SafestPathRouter::SafestPathRouter()
{
    numberOfSubroom=0;
    _lastUpdateTime=0;
    a=1;
    c=1;
    b=0;
    _lastUpdateTime=-1;
    maximalSquare=0;

    flo =NULL;
    rR=NULL;
    peopleAtSection=NULL;
    squareOfSection=NULL;
    dFinalLength=NULL;
    dPeopleDensity=NULL;
    dPreOFP=NULL;
    dFinalLineOFP=NULL;
    dFinalLineEvac=NULL;

    // Output to files
    _phiFile = new FileHandler("Phi_file.csv");
    // _finalLineEvac = new FileHandler("Evac_File.csv");
    // Output to files
    //_finalLineEvac = new double [numberOfSection];
}

SafestPathRouter::~SafestPathRouter()
{
     // Output to files
     delete _phiFile;
     delete flo;
     delete rR;
     delete peopleAtSection;
     delete  squareOfSection;
     delete  dFinalLength;
     delete dPeopleDensity;
     delete dPreOFP;
     delete dFinalLineOFP;
     delete dFinalLineEvac;
}

void SafestPathRouter::Init(Building* building)
{
     //Load the FDS file info
     //handle over to the global router engine
     GlobalRouter::Init(building);


     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* room = _building->GetRoom(i);
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               if(sub->GetType()=="floor")
                    numberOfSubroom=numberOfSubroom+1;
          }
     }

     //cout<<numberOfSubroom<<endl;
     peopleAtSection = new double [numberOfSubroom];
     squareOfSection = new double [numberOfSubroom];
     dFinalLineOFP = new double [numberOfSubroom];
     dFinalLineEvac = new double [numberOfSubroom];
     dPeopleDensity = new double [numberOfSubroom];
     dFinalLength = new double [numberOfSubroom];
     rR = new double [numberOfSubroom];

     for (int i = 0; i < numberOfSubroom; ++i) {
          peopleAtSection[i]=0.0;
          squareOfSection [i]=0.0;
          dFinalLineOFP [i]=0.0;
          dFinalLineEvac[i]=0.0;
          dPeopleDensity [i]=0.0;
          dFinalLength [i]=0.0;
          rR [i]=0.0;
     }

     int n=300;
     dPreOFP=new double* [n];
     flo = new int [numberOfSubroom];

     MappingFloorIDtoIndex();
     // load the matrix from fds
     ReadMatrixFromFDS();


     // lenthOfSection = new double [numberOfSubroom];

     //for (int i = 0; i < numberOfSubroom; ++i) {
     //       rR[i] = new double[numberOfSubroom];
     //for (int i=0; i<numberOfSubroom; i++)
     //                      rR[i]=0;


     // Print out final distance matrix
     //      for(int i = 0; i < numberOfSubroom; i++)
     //              cout << rR[i] << " ";
     //              cout << endl;


     //      for(int i = 0; i < numberOfSubroom; i++){
     //for(int j = 0; j < numberOfSubroom; j++)
     //cout << rR[i] << " ";

     //}
}


int SafestPathRouter::FindExit(Pedestrian* p)
{


     if(ComputeSafestPath(p)==-1) {
          //Log->Write(" sdfds");
     }
     //handle over to the global router engine
     return GlobalRouter::FindExit(p);
}



void SafestPathRouter::UpdateMatrices()
{

     int index=0;

     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* room = _building->GetRoom(i);
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               if(sub->GetType()=="floor") {
                    peopleAtSection[index]=sub->GetNumberOfPedestrians();
                    index++;
               }
          }
     }

     int index1=0;
     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* room = _building->GetRoom(i);
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               if(sub->GetType()=="floor") {
                    squareOfSection[index1]=sub->GetArea();
                    index1++;

               }
          }
     }

     // Printing a matrix
     //              for(int j = 0; j < numberOfSubroom; j++)
     //                      cout << peopleAtSection[j]   << " ";
     //              cout << endl;
     /*

             for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
                             Room* room = _building->GetRoom(i);
                             for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
                                     SubRoom* sub = room->GetSubRoom(j);
                                     if(sub->GetType()=="floor")
                                     {
                                             peopleAtSection[sub->GetSubRoomID()]=sub->GetNumberOfPedestrians();
                                     }
                             }
                     }


                     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
                             Room* room = _building->GetRoom(i);
                             for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
                                     SubRoom* sub = room->GetSubRoom(j);
                                     if(sub->GetType()=="floor")
                                     {
                                             squareOfSection[sub->GetSubRoomID()]=sub->GetArea();

                                     }
                             }
                     }
     */
     /*
             int index=0;

             for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
                     Room* room = _building->GetRoom(i);
                     for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
                             SubRoom* sub = room->GetSubRoom(j);
                             if(sub->GetType()=="floor")
                             {
                                     peopleAtSection[index]=sub->GetNumberOfPedestrians();
                                     index++;
                             }
                     }
             }

             int index1=0;
             for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
                     Room* room = _building->GetRoom(i);
                     for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
                             SubRoom* sub = room->GetSubRoom(j);
                             if(sub->GetType()=="floor")
                             {
                                     squareOfSection[index1]=sub->GetArea();
                                     index1++;

                             }
                     }
             }
     */
     /*

     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
             Room* room = _building->GetRoom(i);
             for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
                     SubRoom* sub = room->GetSubRoom(j);
                     if(sub->GetType()=="floor") {
                             lenthOfSection[sub->GetSubRoomID()]=sub->GetArea();
                     }
             }
     }
      */
}





int SafestPathRouter::ComputeSafestPath(Pedestrian* p)
{

//      // adjust the update frequency
//      double diff= fabs((int)p->GetGlobalTime() - p->GetGlobalTime());
//      if (diff>0.015) return -1;
//      if((((int)p->GetGlobalTime())%UPDATE_FREQUENCY)!=0) return -1;
//
     //int next = GetBestDefaultRandomExit(p);
     //double distance = _accessPoints[next]->GetDistanceTo(0)+p->GetDistanceToNextTarget();


     //for (int i=0; i<path.size(); i++)
     //      preSub[i]=0;


     //Print out final distance matrix
     //for(int j = 0; j < numberOfSubroom; j++)
     //      cout << flo[j] << " ";
     //cout << endl;



     //Print out final distance matrix
     //      for(int j = 0; j < numberOfSubroom; j++)
     //              cout << rR[j] << " ";
     //      cout << endl;

     Room* room =  _building->GetRoom(p->GetRoomID());
     SubRoom* sub = room->GetSubRoom(p->GetSubRoomID());
     int best_goal=p->GetFinalDestination();
     double best_phi=14532545;


     if(sub->GetType()=="dA") {

          //eventually write any goal
          for (map<int, Goal*>::const_iterator itr = _building->GetAllGoals().begin();
                    itr != _building->GetAllGoals().end(); ++itr) {
               int goal_current = itr->second->GetId();

               double phi_current=0;

               // get the path
               std::vector<SubRoom*> path;
               path.clear();
               GetPath(p, goal_current,  path);

//                      for(unsigned int j = 0; j <path.size(); j++)
//                              //if(path[j]->GetType()=="floor")
//                              {
//                                      cout << path[j]->GetSubRoomID()<< " ";
//                                      cout << endl;
//                              }



               // compute the cost (adding the phi values)
               for(unsigned int j = 0; j <path.size(); j++) {
                    if(path[j]->GetType()=="floor") {
                         int z=path[j]->GetSubRoomID();

                         for(int j = 0; j <numberOfSubroom; j++) {
                              if(flo[j]==z) {
                                   phi_current=phi_current+rR[j];
                              }
                         }

                    }


               }
               if (phi_current < best_phi) {
                    best_phi = phi_current;
                    best_goal= goal_current;
               }
               //cout <<"value:" <<phi_current<<endl;
               // save the goal id for that path if smaller that the previous
          }
          //cout <<"best phi: "<< best_phi<<endl;
          //cout <<"best goal: "<< best_goal<<endl;
          //exit(0);
          //cout << rR[10]<< endl;
     }

     p->SetFinalDestination(best_goal);
     p->ClearMentalMap();




//      if (p->GetRoomID()==0 && p->GetSubRoomID()==2)
//      {
//              p->SetFinalDestination(0);
//      }
//
//      if (p->GetRoomID()==0 && p->GetSubRoomID()==3)
//      {
//              p->SetFinalDestination(1);
//      }
//for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
//              Room* room = _building->GetRoom(i);
//              for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
//                      SubRoom* sub = room->GetSubRoom(j);
//                      if(sub->GetType()=="floor")
//                      {
//                              std::vector<SubRoom*> path;
//                              GetPath(p, 1,  path);
//                              preSub=new double [path.size()];
//                              for (unsigned int i=0; i<path.size(); i++){
//                                      preSub[i]=path[i]->GetSubRoomID();
//                                      for(int j = 0; j <path.size(); j++)
//                                              cout << preSub[j]<< " ";
//                                      cout << endl;
//
//                              }
//                      }
//              }
//      }
// Printing a matrix
//cout <<"total distance: " <<distance<<endl;




//CalculatePhi();
//main_1(p);



     return -1;
}



int SafestPathRouter::GetAgentsCountInSubroom( int roomID, int subroomID)
{
     return _building->GetRoom(roomID)->GetSubRoom(subroomID)->GetAllPedestrians().size();
}





//todo: use ?
void SafestPathRouter::Initialize()
{
     //ReadMatrixFromFDS();
}





void SafestPathRouter::ReadMatrixFromFDS()
{
     //double** a;
     int m=numberOfSubroom;
     int n=300;

     for (int i=0; i<n; i++)
          dPreOFP[i]=new double [m];


     for (int i=0; i<n; i++)
          for (int j=0; j<m; j++)
               dPreOFP[i][j]=0;



     fstream F;
     F.open("OFP2.csv");


     if (F) {
          for (int i=0; i<n; i++)
               for (int j=0; j<m; j++)
                    F>>dPreOFP[i][j];
          F.close();
     }


     else {
          cout<<"File does not existed"<<endl;
     }


     //cout<<a[0][299]<<endl;


     for (int i=0; i<n; i++)
          for (int j=0; j<m; j++)
               dPreOFP[i][j]=1-(dPreOFP[i][j]/30);



     //for (int i=0; i<n; i++){
     //      for (int j=0; j<m; j++)
     //              cout<<dPreOFP[i][j]<<" ";
     //      cout<<endl;}

     int tCurrentStepSize=280;


     for (int j=0; j<m; j++)
          dFinalLineOFP[j]=dPreOFP[tCurrentStepSize][j];


     //for (int j=0; j<m; j++)
     //      cout<<dFinalLineOFP[j]<<" ";
     //cout<<endl;


}
//CalculatePhi();

void SafestPathRouter::GetHline(Building* building)
{

     /*

     //              cout << dFinalLineEvac[j]<< " ";
     //              cout << endl;
     //      lenthOfSection

     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
                     Room* room = _building->GetRoom(i);
                     for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
                             SubRoom* sub = room->GetSubRoom(j);
                             if(sub->GetType()=="floor") {

                                     for (map<int, Hline*>::const_iterator itr = _building->GetAllHlines().begin();
                                                             itr != _building->GetAllHlines().end(); ++itr) {
                                                     //int door=itr->first;
                                                     int door = itr->second->GetUniqueID();
                                                     Hline* cross = itr->second;
                                                     Point centre = cross->GetCentre();
                                                     double center[2] = { centre.GetX(), centre.GetY() };

                                                     AccessPoint* ap = new AccessPoint(door, center);
                                                     ap->SetNavLine(cross);
                                                     char friendlyName[CLENGTH];
                                                     sprintf(friendlyName, "hline_%d_room_%d_subroom_%d", cross->GetID(),
                                                                     cross->GetRoom()->GetID(),
                                                                     cross->GetSubRoom()->GetSubRoomID());
                                                     ap->SetFriendlyName(friendlyName);
                                             }





                             }
                     }
             }
      */
}






void SafestPathRouter::CalculatePhi()
{

     // Here:
     //              0.125 is a square of horizontal people's projection (m2)
     //              0.92 is a maximal people's density which do not hurt themselves (m2/m2) (per/m2)
     // All of those values can be changed regarding to some conditions

     for(int j = 0; j < numberOfSubroom; j ++) { //
          dPeopleDensity[j]=peopleAtSection[j]/ squareOfSection[j];
          dFinalLineEvac[j]= (peopleAtSection[j] * 0.125) / (squareOfSection[j] * 0.92);
     }




     // Printing a matrix
     //      for(int j = 0; j < numberOfSubroom; j++)
     //              cout << dPeopleDensity[j]<< " ";
     //              cout << endl;




     for(int j = 0; j < numberOfSubroom; j ++) { //
          double max=0;
          if (squareOfSection[j] > max) {
               maximalSquare=squareOfSection[j];

          }
          //cout << maximalSquare<<endl;
     }

     for(int j = 0; j < numberOfSubroom; j ++) { //
          dFinalLength[j]= squareOfSection[j]/maximalSquare;
     }


     double iNt1[numberOfSubroom];
     double iNt2[numberOfSubroom];
     double iNt3[numberOfSubroom];
     double iNt4[numberOfSubroom];
     double iNt5[numberOfSubroom];
     double iNt6[numberOfSubroom];
     double iNt7[numberOfSubroom];

     for (int j=0; j<numberOfSubroom; j++) {
          iNt1[j]= dFinalLineEvac[j] * a;
          iNt2[j]= dFinalLineOFP[j] * b;
          iNt3[j]= dFinalLength[j] * c;
     }

     // Printing a matrix
     //      for(int j = 0; j < numberOfSubroom; j++)
     //              cout << iNt1[j]<< " ";
     //              cout << endl;


     for (int j=0; j<numberOfSubroom; j++) {
          iNt4[j]= iNt1[j] * iNt1[j];
          iNt5[j]= iNt2[j] * iNt2[j];
          iNt6[j]= iNt3[j] * iNt3[j];
     }


     for (int j=0; j<numberOfSubroom; j++) {
          iNt7[j]=iNt4[j] + iNt5[j] + iNt6[j];
     }

     //double xX[numberOfSubroom];

     for (int j=0; j<numberOfSubroom; j++) {
          rR[j]=sqrt(iNt7[j]);
     }


     // Printing a matrix
     //for(int j = 0; j < numberOfSubroom; j++)
     //      cout << rR[j]<< " ";
     //cout << endl;



     //double dMatrixPreEvac[3]={0.0125,0.0125,0.00833};
     //double dMatrixPreEvac[11]={0.000574,0.000328,0.000589,0.000651,0.000486,0.003576,0.003576,0.001171,0.000976,0.000221,0.000385};
     //double dMatrixPreEvac[1][11]={0.003575,0.000328,0.003575,0.000651,0.000486,0.003575,0.003575,0.001171,0.000976,0.000221,0.000385};
     //double dMatrixPreEvac[1][11]={0.000573,0.000328,0.000589,0.000651,0.000486,0.003574,0.003575,0.001171,0.000976,0.000221,0.000385};
     //double dMatrixPreEvac[1][11]={0.000573,0.000328,0.000589,0.000651,0.000486,0.003574,0.003575,0.001171,0.000976,0.000221,0.000385};
     //double dMatrixPreEvac[1][11]={0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573,0.000573};


}

void SafestPathRouter::PrintInfoToFile()
{

     string content;

     for (int j=0; j<numberOfSubroom; j++) {
          char tmp[20];
          sprintf(tmp,"%lf",rR[j]);
          //cout<<"tmp: "<<tmp<<endl;
          content.append(",");
          content.append(tmp);
     }

     //      cout<<content<<endl;
     _phiFile->Write(content.c_str());

     /*
     for (int j=0; j<numberOfSubroom; j++)
     {
             char tmp[20];
             sprintf(tmp,"%lf",dPeopleDensity[j]);
             //cout<<"tmp: "<<tmp<<endl;
             content.append(",");
             content.append(tmp);
     }

     //      cout<<content<<endl;
     _finalLineEvac->Write(content.c_str());
     */
}

void SafestPathRouter::MappingFloorIDtoIndex()
{

     //map <int, int> flo;

     int index=0;
     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* room = _building->GetRoom(i);
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               if(sub->GetType()=="floor") {
                    flo[index]=sub->GetSubRoomID();
                    index++;

               }
          }
     }

     //Print out final distance matrix
     for(int j = 0; j < numberOfSubroom; j++)
          cout << flo[j] << " ";
     cout << endl;




}

void SafestPathRouter::UpdateRRmatrix(Pedestrian* p)
{

     //double dFinalLength[1][11]={0.328,0.569,0.328,0.414,0.586,0.328,0.328,1.000,0.276,0.759,0.741};
     //double dFinalLength[11]={0.638,0.569,0.534,0.414,0.586,0.328,0.328,1.000,0.276,0.759,0.741};
     //double dFinalLength[11]={0.666,0.666,1.0};
     //double dFinalLength[1][11]={0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638,0.638};
     //double dFinalLength[1][11]={37.0,33.0,31.0,24.0,34.0,19.0,19.0,58.0,16.0,44.0,43.0};
     //double dFinalLineOFP[numberOfSubroom];
     // Print out final distance matrix
     //      for(int j = 0; j < numberOfSubroom; j++)
     //              cout << rR[j] << " ";
     //              cout << endl;


     if(p->GetGlobalTime()>=5) {
          rR[10]=1000000;

     }

}


void SafestPathRouter::ComputeAndUpdateDestinations(
     std::vector<Pedestrian*>& pedestrians)
{

     int currentTime = pedestrians[0]->GetGlobalTime();

     if(currentTime!=_lastUpdateTime)
          if((currentTime%UPDATE_FREQUENCY)==0) {

               UpdateMatrices();
               CalculatePhi();
               _lastUpdateTime=currentTime;
               PrintInfoToFile();
               //cout <<" Updating at : " <<currentTime<<endl;
          }

     // Update

     for (unsigned int p = 0; p < pedestrians.size(); ++p) {

          if(ComputeSafestPath(pedestrians[p])==-1) {
               //Log->Write(" sdfds");
          }

          //handle over to the global router engine
          if (GlobalRouter::FindExit(pedestrians[p]) == -1) {
               //Log->Write("\tINFO: \tCould not found a route for pedestrian %d",_allPedestians[p]->GetID());
               //Log->Write("\tINFO: \tHe has reached the target cell");
               _building->DeletePedFromSim(pedestrians[p]);
               //exit(EXIT_FAILURE);
          }
     }

//      for (unsigned int p = 0; p < pedestrians.size(); ++p) {
//
//              if (pedestrians[p]->FindRoute() == -1) {
//                      //Log->Write("\tINFO: \tCould not found a route for pedestrian %d",_allPedestians[p]->GetID());
//                      //Log->Write("\tINFO: \tHe has reached the target cell");
//                      _building->DeletePedFromSim(pedestrians[p]);
//                      //exit(EXIT_FAILURE);
//              }
//      }

}


/*
        std::vector<SubRoom*> path;
        GetPath(p, 1,  path);

        double dF[path.size()];
        double fF=0;

        for(unsigned i=0; i<path.size(); i++)
                dF[i]=0;

        for(unsigned j = 0; j <path.size(); j++)
        {
                int i=preSub[j];
                dF[j]=rR[i];

                //      for(int j = 0; j < numberOfSubroom; j++)
                //              cout <<i<<endl;

        }

        //for(int j = 0; j < path.size(); j++)
        //      cout << dF[j] << " ";
        //cout << endl;


        //for(int j=0; j<path.size(); j++)
        //      fF=fF+dF[j];

        fF=fF-dF[1];

        //cout << fF<<endl;


        // Print out final distance matrix
        //      for(int j = 0; j < numberOfSubroom; j++)
        //              cout << i<<endl;


}



        // Floyd-Warchal algorithm is going here
        // Initialize
        int vertices = numberOfSubroom;
        vector<vector<double> > a(vertices, vector<double>(vertices,999));
        // initialize diagonal
        for(int i=0; i < vertices; i++)
                a[i][i]=0;


        // initialize distances
        a[0][1]=rR[0];
        a[1][2]=rR[1];
        a[1][3]=rR[2];
        //
        a[1][0]=rR[0];
        a[2][1]=rR[1];
        a[3][1]=rR[2];



        // Floyd-Warshal
        // Add nodes between (first 1 then 2, 3 till n) and look if
        // distance is shorter
        for(int k = 0; k < vertices; k++)
                for(int i = 0; i < vertices; i++)
                        for(int j = 0; j < vertices; j++)
                                if(a[i][j]>a[i][k]+a[k][j])
                                        a[i][j]=a[i][k]+a[k][j];


 */

/*





        //Print out final distance matrix
        //for(int i = 0; i < vertices; i++){
        //      for(int j = 0; j < vertices; j++)
        //              cout << a[i][j] << " ";
        //              cout << endl;
        //              cout<<p->GetGlobalTime()<<endl;
        //      }
        //double g3_0=a[0][2];// From node 3 to 0 (0 is the goal O)
        //double g3_1=a[1][2];// From node 3 to 1 (1 is the goal 1)
        //double g1_3=a[0][2];
        //double g1_4=a[0][3];
        //cout<<"The g5_0: "<<g5_0<<"\n";
        //cout<<"The g5_1: "<<g5_1<<"\n";
        //cout<<p->GetGlobalTime()<<endl;

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

        rR[subroomID][subroomID]=

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


                                                int roomID=p->GetRoomID();
                                                int subroomID=p->GetSubRoomID();
                                                int nGoals =_building->GetNumberOfGoals();


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

}
 */






