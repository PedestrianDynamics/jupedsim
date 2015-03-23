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
    delete grid
    delete[] flag;
    delete[] dist2Wall;
    delete[] speedInitial;
    delete[] cost;

}

FloorfieldViaFM::FloorfieldViaFM(const Building* const buildingArg, const double hxArg, const double hyArg) {
    //ctor

    //parse building and create list of walls/obstacles (find xmin xmax, ymin, ymax, and add border?)

    //create rectgrid (grid->createGrid())

    //'grid->GetnPoints()' and create all data fields: cost, gradient, speed, dis2wall, flag, secondaryKey

    //call fkt: linescan und set Distance2Wall mit 0 fuer alle Wandpunkte, speed mit lowspeed
    //this step includes Linescanalgorithmus? (maybe included in parsing above)

    // @continue

    //call fkt: calculateDistanzefield

    //call fkt: calculateFloorfield(bool useDis2Wall)
}

FloorfieldViaFM::FloorfieldViaFM(const FloorfieldViaFM& other)
{
    //copy ctor
}

//FloorfieldViaFM& FloorfieldViaFM::operator=(const FloorfieldViaFM& rhs)
//{
//    if (this == &rhs) return *this; // handle self assignment     //@todo: ar.graf change this pasted code to proper code
//    //assignment operator
//    return *this;
//}

void FloorfieldViaFM::parseBuilding(const Building* const buildingArg) {
    //init min/max before parsing
    double xMin = 100000.;
    double xMax = -100000.;
    double yMin = xMin;
    double yMax = xMax;
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
                    // xMin xMax
                    if ((*itObsWall).GetPoint1().GetX() < xMin) xMin = (*itObsWall).GetPoint1().GetX();
                    if ((*itObsWall).GetPoint2().GetX() < xMin) xMin = (*itObsWall).GetPoint2().GetX();
                    if ((*itObsWall).GetPoint1().GetX() > xMax) xMax = (*itObsWall).GetPoint1().GetX();
                    if ((*itObsWall).GetPoint2().GetX() > xMax) xMax = (*itObsWall).GetPoint2().GetX();

                    // yMin yMax
                    if ((*itObsWall).GetPoint1().GetY() < yMin) yMin = (*itObsWall).GetPoint1().GetY();
                    if ((*itObsWall).GetPoint2().GetY() < yMin) yMin = (*itObsWall).GetPoint2().GetY();
                    if ((*itObsWall).GetPoint1().GetY() > yMax) yMax = (*itObsWall).GetPoint1().GetY();
                    if ((*itObsWall).GetPoint2().GetY() > yMax) yMax = (*itObsWall).GetPoint2().GetY();
                }
            }

            std::vector<Wall> allWalls = (*itSubroom)->GetAllWalls();
            for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
                wall.push_back(&(*itWall));
                // xMin xMax
                if ((*itWall).GetPoint1().GetX() < xMin) xMin = (*itWall).GetPoint1().GetX();
                if ((*itWall).GetPoint2().GetX() < xMin) xMin = (*itWall).GetPoint2().GetX();
                if ((*itWall).GetPoint1().GetX() > xMax) xMax = (*itWall).GetPoint1().GetX();
                if ((*itWall).GetPoint2().GetX() > xMax) xMax = (*itWall).GetPoint2().GetX();

                // yMin yMax
                if ((*itWall).GetPoint1().GetY() < yMin) yMin = (*itWall).GetPoint1().GetY();
                if ((*itWall).GetPoint2().GetY() < yMin) yMin = (*itWall).GetPoint2().GetY();
                if ((*itWall).GetPoint1().GetY() > yMax) yMax = (*itWall).GetPoint1().GetY();
                if ((*itWall).GetPoint2().GetY() > yMax) yMax = (*itWall).GetPoint2().GetY();
            }
        }
    }
    //create Rect Grid
    grid = new RectGrid();
    grid->setBoundaries(xMin, yMin, xMax, yMax);
    grid->setSpacing( .05, .05);
    grid->createGrid();

    //create arrays
    flag = new int[grid->GetnPoints()];                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final)
    dist2Wall = new double[grid->GetnPoints()];
    speedInitial = new double[grid->GetnPoints()];
    cost = new double[grid->GetnPoints];

    //linescan using (std::vector<Wall*>)
    lineScan(allWalls, dist2Wall, 0., -2.);
    for (unsigned long int i = 0; i < grid->GetnPoints(); ++i) {
        if (dist2Wall[i] == 0.) {               //outside
            speedInitial[i] = .001;
            cost[i]         = -7.; // @todo: ar.graf
            flag[i]         = -7;
        } else {                                //inside
            speedInitial[i] = 1.;
            cost[i]         = -2.;
            flag[i]         = 0;
        }
    }

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

    for(unsigned long int j = 0; j < jMax; ++j) { // @todo ar.graf if segfault during writing, check if j < jMax

        //init line with "(outside+inside)/2"
        for (unsigned long int initp = 0; initp < iMax; ++initp) {
            target[j*iMax+initp] = (outside + inside) / 2;
        } //init done
        xIntersection.clear();

        Point linestart(xMin,j*hy+yMin);
        Point lineend  (xMax,j*hy+yMin);
        Line currLine = Line(linestart, lineend);
        for(std::vector<Wall*>::const_iterator itWall = wallArg.begin(); itWall != wallArg.end(); ++itWall) {
            //if wall is horizontal, we must deal with it by setting wall value all along
            //note: if wall is horizontal, then adjacent walls should yield intersectionpoints and make
            //      linescan fill the points under the horizontal wall anyway. lets check that. \____/
            if (    ((**itWall).GetPoint1().GetY() == (linestart.GetY()))  &&  ((**itWall).IsHorizontal())   ) {
                unsigned long int istart, iend;
                istart = ((**itWall).GetPoint1().GetX() - xMin)/hx + .5;
                iend   = ((**itWall).GetPoint2().GetX() - xMin)/hx + .5;
                if (istart > iend) {
                    unsigned long int temp = istart;
                    istart = iend;
                    iend = temp;
                }
                for (unsigned long int i = istart; i <= iend; ++i) {
                    target[j*iMax+i] = outside;
                }
            } else {
                double distance = currLine.GetIntersectionDistance( *(*itWall) );
                if ( (distance >= 0.) && (distance < 100000) ) {        //check if Line.cpp can be changed (infinity == 100000) seems quite finite
                    xIntersection.push_back(sqrt(distance));
                }
            }
        } //all walls intersected with currLine
        //now init the line using the intersections
        std::unique(xIntersection.begin(), xIntersection.end());
        std::sort(xIntersection.begin(), xIntersection.end());
        unsigned long int old = 0;
        unsigned long int upTo = 0;
        target[j*iMax+0] = outside;
        double filler = outside;
        for (unsigned int ithCross = 0; ithCross < xIntersection.size(); ++ithCross) {
            upTo = (xIntersection[ithCross] - xMin)/hx + .5;
            for (unsigned long int iCurrSegment = old+1; iCurrSegment < upTo; ++iCurrSegment) {
                if (target[j*iMax+iCurrSegment] == (outside + inside) / 2) {
                    target[j*iMax+iCurrSegment] = filler;
                }
            }
            if (filler == outside) {
                filler = inside;
            } else {
                filler = outside;
            }
            target[j*iMax+upTo]=outside; //intersections always walls or obstacles
            old = upTo;
        }
        for (unsigned long int rest = old+1; rest < iMax; ++rest) {
            target[j*iMax+rest] = outside;
        }
        //secure check if all gridpoints got set
        for (unsigned long int initp = 0; initp < iMax; ++initp) {
            if (  target[j*iMax+initp] == ((outside + inside)/2)  ) {
                //sth went wrong
                std::cerr << "Error in Linescan\n";
            };
        } //sec check done
    } //loop over all lines
}

void FloorfieldViaFM::calculateDistanceField() {
    //sanity check (fields <> 0)

}
