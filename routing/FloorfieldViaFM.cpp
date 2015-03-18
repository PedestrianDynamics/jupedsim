/**
 * \file        FloorfieldViaFM.cpp
 * \date        Mar 05, 2015
 * \version     N/A (v0.6)
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
 * Implementation of classes for ...
 *
 *
 **/

#include "FloorfieldViaFM.h"

FloorfieldViaFM::FloorfieldViaFM()
{
    //ctor
}

FloorfieldViaFM::~FloorfieldViaFM()
{
    //dtor
}

FloorfieldViaFM::FloorfieldViaFM(const Building* const buildingArg, const double hxArg, const double hyArg) {
    //ctor

    //parse building and create list of walls/obstacles (find xmin xmax, ymin, ymax, and add border?)

    //create rectgrid (grid->createGrid())

    //'grid->GetnPoints()' and create all data fields: cost, gradient, speed, dis2wall, flag, secondaryKey

    //call fkt: init Distance2Wall mit 0 fuer alle Wandpunkte, speed mit lowspeed
    //this step includes Linescanalgorithmus? (maybe included in parsing above)

    //call fkt: calculateDistanzefield

    //call fkt: calculateFloorfield(bool useDis2Wall)
}

FloorfieldViaFM::FloorfieldViaFM(const FloorfieldViaFM& other)
{
    //copy ctor
}

FloorfieldViaFM& FloorfieldViaFM::operator=(const FloorfieldViaFM& rhs)
{
    if (this == &rhs) return *this; // handle self assignment     //@todo: ar.graf change this pasted code to proper code
    //assignment operator
    return *this;
}

void FloorfieldViaFM::parseBuilding(const Building* const buildingArg) {
    //create a list of walls
    std::vector<Room*> allRooms = buildingArg->GetAllRooms();
    for (std::vector<Room*>::iterator itRoom = allRooms.begin(); itRoom != allRooms.end(); ++itRoom) {

        std::vector<SubRoom*> allSubrooms = (*itRoom)->GetAllSubRooms();
        for (std::vector<SubRoom*>::iterator itSubroom = allSubrooms.begin(); itSubroom != allSubrooms.end(); ++itSubroom) {

            std::vector<Obstacle*> allObstacles = (*itSubroom)->GetAllObstacles();
            for (std::vector<Obstacle*>::iterator itObstacles = allObstacles.begin(); itObstacles != allObstacles.end(); ++itObstacles) {

                std::vector<Wall> allObsWalls = (*itObstacles)->GetAllWalls();
                for (std::vector<Wall>::iterator itObsWall = allObsWalls.begin(); itObsWall != allObsWalls.end(); ++itObsWall) {
                    wall.push_back(&(*itObsWall));
                }
            }

            std::vector<Wall> allWalls = (*itSubroom)->GetAllWalls();
            for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
                wall.push_back(&(*itWall));
            }
        }
    }
    //create Rect Grid
    //linescan using (std::vector<Wall*>)
}

void FloorfieldViaFM::resetGoalAndCosts(const Goal* const goalArg) {
/* this fkt shall set all Costdata to unknown and the goal points to 0
   we have to watch how to calc indices as goal only knows coords from input file
*/

// set all cost data to "unknown" (remember to change flag accordingly)
// get lines/walls from goals
// use linescan to ititialize cost grid

}

void FloorfieldViaFM::lineScan(const std::vector<Wall*>& wallArg, double* const target, const double outside, const double inside) {
// use RectGrid to go thru lines and check intersection with any wall
// if intersection is found, then change (outside to inside) or (inside to outside)
// and keep setting values
// maybe calc and save intersections for each line and then init target array
// (*)
// take line x: calc intersection with wall y: if intersects then store intersectionpoint in a vector<int key>
// now sort vector<int key> and go thru targetline and set values
// empty vector<int key> and continue with line x+1 at (*)

    //grid handeling local vars:
    unsigned long int iMax  = grid->GetiMax();
    unsigned long int jMax  = grid->GetjMax();
    double xMin             = grid->GetxMin();
    double yMin             = grid->GetyMin();
    double xMax             = grid->GetxMax();
    double yMax             = grid->GetyMax();
    double hx               = grid->Gethx();
    double hy               = grid->Gethy();
    std::vector<double> xIntersection;

    Point linestart(xMin,j*hy+yMin);
    Point lineend  (xMax,j*hy+yMin);
    Line currLine = currLine(linestart, lineend);
    for(std::vector<Wall*>.iterator itWall = wallArg.begin(); itWall != wallArg.end(); ++itWall) {
        double distance = currLine.GetIntersectionDistance( *(*itWall) );
        if ( (distance >= 0.) && (distance < 100000) ) {
            xIntersection.push_back(distance);
        }
    }
    // how to calc (i,j) from x/y coord:
    //unsigned long int i = (unsigned long int)(((currPoint.GetX()-xMin)/hx)+.5);
    //unsigned long int j = (unsigned long int)(((currPoint.GetY()-yMin)/hy)+.5);
}

void FloorfieldViaFM::calculateDistanceField() {
    //sanity check (fields <> 0)

}
