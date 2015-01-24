/**
 * \file        LCGrid.cpp
 * \date        Nov 16, 2010
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J?lich GmbH. All rights reserved.
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
 * This class implements the Linked-Cells algorithm
 * \ref{cacs.usc.edu/education/cs596/01-1LinkedListCell.pdf}
 * A grid is laid on the complete geometry and the pedestrians are assigned the cells
 * at each simulation step. Only pedestrians in the neighbouring cells are involved
 * in the force computations.
 *
 * The class is static as only one instance is needed per simulation round.
 * This solution is fine for parallelisation as well, at least for OpenMP.
 *
 *
 **/
 

#include"LCGrid.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Building.h"

using namespace std;


LCGrid::LCGrid(double boundaries[4], double cellsize, int nPeds)
{
     pGrid_xmin=boundaries[0];
     pGrid_xmax=boundaries[1];
     pGrid_ymin=boundaries[2];
     pGrid_ymax=boundaries[3];
     pCellSize=cellsize;
     pNpeds=nPeds;

     // add 1 to ensure that the whole area is covered by cells if not divisible without remainder
     pGridSizeX = (int) ((pGrid_xmax - pGrid_xmin) / pCellSize) + 1 + 2; // 1 dummy cell on each side
     pGridSizeY = (int) ((pGrid_ymax - pGrid_ymin) / pCellSize) + 1 + 2; // 1 dummy cell on each side

     // allocate memory for cells (2D-array) and initialize
     pCellHead= new int *[pGridSizeY];

     for (int i = 0; i < pGridSizeY; ++i) {
          pCellHead[i]  = new int[pGridSizeX]; // nx columns

          for (int j = 0; j < pGridSizeX; ++j) {
               pCellHead[i][j] = LIST_EMPTY;
          }
     }

     // creating and resetting the pedestrians list
     pList = new int[nPeds];
     for(int i=0; i<nPeds; i++) pList[i]=0;

     //allocating the place for the peds copy
     pLocalPedsCopy=new Pedestrian*[nPeds];
     for(int i=0; i<nPeds; i++) pLocalPedsCopy[i]=NULL;

}

LCGrid::~LCGrid()
{
     for(int i=0; i<pNpeds; i++) {
          if(!pLocalPedsCopy[i])
               delete pLocalPedsCopy[i];
     }
     delete [] pList;
     delete [] pLocalPedsCopy;
     for (int i = 0; i < pGridSizeY; ++i)  delete[] pCellHead[i];
     delete[] pCellHead;
}

void LCGrid::ShallowCopy(const vector<Pedestrian*>& peds)
{

     for(unsigned int p=0; p<peds.size(); p++) {
          int id= peds[p]->GetID()-1;
          pLocalPedsCopy[id]=peds[p];
     }
}

void LCGrid::Update(const vector<Pedestrian*>& peds)
{
     int nSize=peds.size();

     ClearGrid();

     for (int p = 0; p < nSize; p++) {
          Pedestrian* ped = peds[p];
          int id=ped->GetID()-1;
          // determine the cell coordinates of pedestrian i
          int ix = (int) ((ped->GetPos().GetX() - pGrid_xmin) / pCellSize) + 1; // +1 because of dummy cells
          int iy = (int) ((ped->GetPos().GetY() - pGrid_ymin) / pCellSize) + 1;
          //printf("[%f, %f]  ",ped->GetPos().GetX(), ped->GetPos().GetY());
          // create lists
          //printf("[%d=%d] ",ped->GetPedIndex(),id);
          pList[id] = pCellHead[iy][ix];
          pCellHead[iy][ix] = id;

          pLocalPedsCopy[id]=ped;
     }
}

// I hope you had called Clear() first
void LCGrid::Update(Pedestrian* ped)
{
     int id=ped->GetID()-1;
     // determine the cell coordinates of pedestrian i
     int ix = (int) ((ped->GetPos().GetX() - pGrid_xmin) / pCellSize) + 1; // +1 because of dummy cells
     int iy = (int) ((ped->GetPos().GetY() - pGrid_ymin) / pCellSize) + 1;

     // update the list previously created
     pList[id] = pCellHead[iy][ix];
     pCellHead[iy][ix] = id;

     // this is probably a pedestrian coming from the mpi routine, so made a copy
     pLocalPedsCopy[id]=ped;
}

void LCGrid::ClearGrid()
{
     // start by resetting the current list
     for (int i = 0; i < pGridSizeY; ++i) {
          for (int j = 0; j < pGridSizeX; ++j) {
               pCellHead[i][j] = LIST_EMPTY;
          }
     }

     for(int i=0; i<pNpeds; i++) {
          pList[i]=LIST_EMPTY;
          pLocalPedsCopy[i]=NULL;
     }
}

void LCGrid::HighlightNeighborhood(int pedID, Building* building)
{
     // force spotlight activation
     Pedestrian::SetColorMode(BY_SPOTLIGHT);
     //darken all
     const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     for(unsigned int p=0;p<allPeds.size();p++){
          allPeds[p]->SetSpotlight(false);
     }

     Pedestrian* ped=building->GetPedestrian(pedID);
     //get and highlight the neighborhood
     if(ped){
          vector<Pedestrian*> neighbours;
          GetNeighbourhood(ped,neighbours);
          for(unsigned int p=0;p<neighbours.size();p++){
               neighbours[p]->SetSpotlight(true);
          }
     }
}

void LCGrid::GetNeighbourhood(const Pedestrian* ped, vector<Pedestrian*>& neighbourhood)
{

     double xPed=ped->GetPos().GetX();
     double yPed=ped->GetPos().GetY();

     int l = (int) ((xPed - pGrid_xmin) / pCellSize) + 1; // +1 because of dummy cells
     int k = (int) ((yPed - pGrid_ymin) / pCellSize) + 1;

     //-1 to get  correct mapping in the array local
     int myID=ped->GetID()-1;

     // all neighbor cells
     for (int i = l - 1; i <= l + 1; ++i) {
          for (int j = k - 1; j <= k + 1; ++j) {
               //printf(" i=%d j=%d k=%d l=%d\n",i,j,nx,ny);
               int p = pCellHead[j][i];
               // all peds in one cell
               while (p != LIST_EMPTY) {
                    // double x=pLocalPedsCopy[p]->GetPos().GetX();
                    // double y=pLocalPedsCopy[p]->GetPos().GetY();
                    // double dist=((x-xPed)*(x-xPed) + (y-yPed)*(y-yPed));
                    if(p!=myID) {
                         // if((dist<pCellSize*pCellSize) && (p!=myID)) {
                         neighbourhood.push_back(pLocalPedsCopy[p]);
                    }
                    // next ped
                    p = pList[p];
               }
          }
     }
}

double LCGrid::GetCellSize()
{
    return pCellSize;
}


void LCGrid::Dump()
{

     for(int l =1; l<pGridSizeY-1; l++) {
          for(int k=1; k<pGridSizeX-1; k++) {

               int     ped = pCellHead[l][k];

               if(ped==LIST_EMPTY) continue;

               printf("Cell[%d][%d] = { ",l,k);
               // all neighbor cells
               for (int i = l - 1; i <= l + 1; ++i) {
                    for (int j = k - 1; j <= k + 1; ++j) {
                         // dummy cells will be empty
                         int p =  pCellHead[i][j];
                         // all peds in one cell
                         while (p != LIST_EMPTY) {
                              printf("%d, ",p+1);
                              // next ped
                              p = pList[p];
                         }
                    }
               }
               printf("}\n");
          }
     }
}

void LCGrid::dumpCellsOnly()
{
     for(int l =1; l<pGridSizeY-1; l++) {
          for(int k=1; k<pGridSizeX-1; k++) {

               int     ped = pCellHead[l][k];

               if(ped==LIST_EMPTY) continue;

               printf("Cell[%d][%d] = { ",l,k);

               // all neighbor cells
               // dummy cells will be empty
               int p =  pCellHead[l][k];
               // all peds in one cell
               while (p != LIST_EMPTY) {
                    printf("%d, ",p+1);
                    // next ped
                    p = pList[p];
               }
               printf("}\n");
          }
     }
}

std::string LCGrid::ToXML()
{
     string grid;
     for (double x=pGrid_xmin;x<=pGrid_xmax;x+=pCellSize)
     {
          char wall[500] = "";
          grid.append("\t\t<wall>\n");
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",x*FAKTOR, pGrid_ymin * FAKTOR);
          grid.append(wall);
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",x*FAKTOR, pGrid_ymax * FAKTOR);
          grid.append(wall);
          grid.append("\t\t</wall>\n");
     }
     for (double y=pGrid_ymin;y<=pGrid_ymax;y+=pCellSize)
     {
          char wall[500] = "";
          grid.append("\t\t<wall>\n");
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",pGrid_xmin*FAKTOR, y * FAKTOR);
          grid.append(wall);
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",pGrid_xmax*FAKTOR, y * FAKTOR);
          grid.append(wall);
          grid.append("\t\t</wall>\n");
     }


     //     for (int a=0;a<pGridSizeX;a++)
     //     {
     //          double x= pGrid_xmin +a*pCellSize;
     //          char wall[500] = "";
     //          grid.append("\t\t<wall>\n");
     //          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",x*FAKTOR, pGrid_ymin * FAKTOR);
     //          grid.append(wall);
     //          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",x*FAKTOR, pGrid_ymax * FAKTOR);
     //          grid.append(wall);
     //          grid.append("\t\t</wall>\n");
     //     }
     //
     //     for (int a=0;a<pGridSizeY;a++)
     //     {
     //          double y=pGrid_ymin +a*pCellSize;
     //          char wall[500] = "";
     //          grid.append("\t\t<wall>\n");
     //          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",pGrid_xmin*FAKTOR, y * FAKTOR);
     //          grid.append(wall);
     //          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",pGrid_xmax*FAKTOR, y * FAKTOR);
     //          grid.append(wall);
     //          grid.append("\t\t</wall>\n");
     //     }
     return grid;
}
