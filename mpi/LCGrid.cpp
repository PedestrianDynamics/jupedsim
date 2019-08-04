/**
 * \file        LCGrid.cpp
 * \date        Nov 16, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J?lich GmbH. All rights reserved.
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

#include "pedestrian/Pedestrian.h"

#include <mutex>


std::mutex grid_mutex;

//FIXME:
#define MAX_AGENT_COUNT  10000 // 1000000

LCGrid::LCGrid(double boundaries[4], double cellsize, int nPeds)
{
     _gridXmin=boundaries[0];
     _gridXmax=boundaries[1];
     _gridYmin=boundaries[2];
     _gridYmax=boundaries[3];
     _cellSize=cellsize;
     _nPeds = nPeds + MAX_AGENT_COUNT;

     // add 1 to ensure that the whole area is covered by cells if not divisible without remainder
     _gridSizeX = (int) ((_gridXmax - _gridXmin) / _cellSize) + 1 + 2; // 1 dummy cell on each side
     _gridSizeY = (int) ((_gridYmax - _gridYmin) / _cellSize) + 1 + 2; // 1 dummy cell on each side

     // allocate memory for cells (2D-array) and initialize
     _cellHead= new int *[_gridSizeY];

     for (int i = 0; i < _gridSizeY; ++i) {
          _cellHead[i]  = new int[_gridSizeX]; // nx columns

          for (int j = 0; j < _gridSizeX; ++j) {
               _cellHead[i][j] = LIST_EMPTY;
          }
     }

     // creating and resetting the pedestrians list
     _list = new int[_nPeds];
     for(int i=0; i<_nPeds; i++) _list[i]=0;

     //allocating the place for the peds copy
     _localPedsCopy=new Pedestrian*[_nPeds];
     for(int i=0; i<_nPeds; i++) _localPedsCopy[i]=nullptr;

}

LCGrid::~LCGrid()
{
//     for(int i=0; i<_nPeds; i++) {
//          if(_localPedsCopy[i]!=nullptr)
//               delete _localPedsCopy[i];
//     }
     delete [] _list;
     delete [] _localPedsCopy;
     for (int i = 0; i < _gridSizeY; ++i)  delete[] _cellHead[i];
     delete[] _cellHead;
}

void LCGrid::ShallowCopy(const std::vector<Pedestrian*>& peds)
{
     for(unsigned int p=0; p<peds.size(); p++)
     {
          int id= peds[p]->GetID()-1;
          _localPedsCopy[id]=peds[p];
     }
}

void LCGrid::Update(const std::vector<Pedestrian*>& peds)
{
     grid_mutex.lock();
     ClearGrid();

     for (auto& ped: peds)
     {
               //Pedestrian* ped = peds[p];
          int id=ped->GetID()-1;
          // determine the cell coordinates of pedestrian i
          int ix = (int) ((ped->GetPos()._x - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
          int iy = (int) ((ped->GetPos()._y - _gridYmin) / _cellSize) + 1;
          //printf("[%f, %f]  ",ped->GetPos()._x, ped->GetPos()._y);
          // create lists
          //printf("[%d=%d] ",ped->GetPedIndex(),id);
          _list[id] = _cellHead[iy][ix];
          _cellHead[iy][ix] = id;

          _localPedsCopy[id]=ped;

//          if (ped->GetID() == 71) {
//               std::cout << "pos: " << ped->GetPos().toString() << " ix:"  << ix << " iy:" << iy << std::endl;
//          }

     }
     grid_mutex.unlock();

}

// I hope you had called Clear() first
// todo: can be used to solve the issue with MAX_AGENT_COUNT
void LCGrid::Update(Pedestrian* ped)
{
     grid_mutex.lock();

     int id=ped->GetID()-1;
     // determine the cell coordinates of pedestrian i
     int ix = (int) ((ped->GetPos()._x - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
     int iy = (int) ((ped->GetPos()._y - _gridYmin) / _cellSize) + 1;

     // update the list previously created
     _list[id] = _cellHead[iy][ix];
     _cellHead[iy][ix] = id;

     // this is probably a pedestrian coming from the mpi routine, so made a copy
     _localPedsCopy[id]=ped;
     grid_mutex.unlock();

}

void LCGrid::ClearGrid()
{
     // start by resetting the current list
     for (int i = 0; i < _gridSizeY; ++i) {
          for (int j = 0; j < _gridSizeX; ++j) {
               _cellHead[i][j] = LIST_EMPTY;
          }
     }

     for(int i=0; i<_nPeds; i++) {
          _list[i]=LIST_EMPTY;
          _localPedsCopy[i]=nullptr;
     }
}

void LCGrid::HighlightNeighborhood(int pedID, Building* building)
{
     // force spotlight activation
     Pedestrian::SetColorMode(BY_SPOTLIGHT);
     //darken all
     for(auto&& ped: building->GetAllPedestrians())
     {
          ped->SetSpotlight(false);
     }

     Pedestrian* ped=building->GetPedestrian(pedID);
     //get and highlight the neighborhood
     if(ped){
          std::vector<Pedestrian*> neighbours;
          GetNeighbourhood(ped,neighbours);

          for(auto&& p: neighbours)
               p->SetSpotlight(true);
     }
}
/*
void LCGrid::HighlightNeighborhood(int pedID, Building* building)
{
     // force spotlight activation
     Pedestrian::SetColorMode(BY_SPOTLIGHT);
     //darken all
     //const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     //for(unsigned int p=0;p<allPeds.size();p++){
     //     allPeds[p]->SetSpotlight(false);
     //}

     for(auto&& ped: building->GetAllPedestrians())
     {
          ped->SetSpotlight(false);
     }

     Pedestrian* ped=building->GetPedestrian(pedID);
     //get and highlight the neighborhood
     if(ped){
          vector<Pedestrian*> neighbours;
          GetNeighbourhood(ped,neighbours);

          for(auto&& p: neighbours)
               p->SetSpotlight(true);
          //for(unsigned int p=0;p<neighbours.size();p++){
          //     neighbours[p]->SetSpotlight(true);
          //}
     }
}
*/
void LCGrid::GetNeighbourhood(const Pedestrian* ped, std::vector<Pedestrian*>& neighbourhood)
{
     grid_mutex.lock();

     double xPed=ped->GetPos()._x;
     double yPed=ped->GetPos()._y;

     int l = (int) ((xPed - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
     int k = (int) ((yPed - _gridYmin) / _cellSize) + 1;

     //-1 to get  correct mapping in the array local
     int myID=ped->GetID()-1;

     // all neighbor cells
     for (int i = l - 1; i <= l + 1; ++i) {
          for (int j = k - 1; j <= k + 1; ++j) {
               //printf(" i=%d j=%d k=%d l=%d\n",i,j,nx,ny);
               int p = _cellHead[j][i];
               // all peds in one cell
               while (p != LIST_EMPTY) {
                    // double x=pLocalPedsCopy[p]->GetPos()._x;
                    // double y=pLocalPedsCopy[p]->GetPos()._y;
                    // double dist=((x-xPed)*(x-xPed) + (y-yPed)*(y-yPed));
                    if(p!=myID) {
                         // if((dist<pCellSize*pCellSize) && (p!=myID)) {
                         neighbourhood.push_back(_localPedsCopy[p]);
                    }
                    // next ped
                    p = _list[p];
               }
          }
     }

     grid_mutex.unlock();

}

void LCGrid::GetNeighbourhood(const Point& pos, std::vector<Pedestrian*>& neighbourhood)
{
     grid_mutex.lock();

     double xPed=pos._x;
     double yPed=pos._y;

     int l = (int) ((xPed - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
     int k = (int) ((yPed - _gridYmin) / _cellSize) + 1;

     // all neighbor cells
     for (int i = l - 1; i <= l + 1; ++i) {
          for (int j = k - 1; j <= k + 1; ++j) {
               //printf(" i=%d j=%d k=%d l=%d\n",i,j,nx,ny);
               int p = _cellHead[j][i];
               // all peds in one cell
               while (p != LIST_EMPTY)
               {
                    neighbourhood.push_back(_localPedsCopy[p]);
                    p = _list[p];  // next ped
               }
          }
     }
     grid_mutex.unlock();

}


double LCGrid::GetCellSize()
{
    return _cellSize;
}


void LCGrid::Dump()
{
     for(int l =1; l<_gridSizeY-1; l++) {
          for(int k=1; k<_gridSizeX-1; k++) {

               int     ped = _cellHead[l][k];

               if(ped==LIST_EMPTY) continue;

               printf("Cell[%d][%d] = { ",l,k);
               // all neighbor cells
               for (int i = l - 1; i <= l + 1; ++i) {
                    for (int j = k - 1; j <= k + 1; ++j) {
                         // dummy cells will be empty
                         int p =  _cellHead[i][j];
                         // all peds in one cell
                         while (p != LIST_EMPTY) {
                              printf("%d, ",p+1);
                              // next ped
                              p = _list[p];
                         }
                    }
               }
               printf("}\n");
          }
     }
}

void LCGrid::dumpCellsOnly()
{
     for(int l =1; l<_gridSizeY-1; l++) {
          for(int k=1; k<_gridSizeX-1; k++) {

               int     ped = _cellHead[l][k];

               if(ped==LIST_EMPTY) continue;

               printf("Cell[%d][%d] = { ",l,k);

               // all neighbor cells
               // dummy cells will be empty
               int p =  _cellHead[l][k];
               // all peds in one cell
               while (p != LIST_EMPTY) {
                    printf("%d, ",p+1);
                    // next ped
                    p = _list[p];
               }
               printf("}\n");
          }
     }
}

std::string LCGrid::ToXML()
{
     std::string grid;
     for (double x=_gridXmin;x<=_gridXmax;x+=_cellSize)
     {
          char wall[500] = "";
          grid.append("\t\t<wall>\n");
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",x*FAKTOR, _gridYmin * FAKTOR);
          grid.append(wall);
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",x*FAKTOR, _gridYmax * FAKTOR);
          grid.append(wall);
          grid.append("\t\t</wall>\n");
     }
     for (double y=_gridYmin;y<=_gridYmax;y+=_cellSize)
     {
          char wall[500] = "";
          grid.append("\t\t<wall>\n");
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",_gridXmin*FAKTOR, y * FAKTOR);
          grid.append(wall);
          sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",_gridXmax*FAKTOR, y * FAKTOR);
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
