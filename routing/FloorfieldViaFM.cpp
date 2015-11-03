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
#define TESTING
#define GEO_UP_SCALE 1
#include "FloorfieldViaFM.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

FloorfieldViaFM::FloorfieldViaFM()
{
    //ctor (very ugly)
    std::cerr << "The defaultconsturctor FloorfieldViaFM should not be called!" << std::endl;
}

FloorfieldViaFM::~FloorfieldViaFM()
{
    //dtor
    delete grid;
    if (flag) delete[] flag;
    if (dist2Wall) delete[] dist2Wall;
    if (speedInitial) delete[] speedInitial;
    if (cost) delete[] cost;
    if (neggrad) delete[] neggrad;
    if (dist2Wall) delete[] dist2Wall;
    if (trialfield) delete[] trialfield;

}

FloorfieldViaFM::FloorfieldViaFM(const Building* const buildingArg, const double hxArg, const double hyArg,
                                 const double wallAvoidDistance, const bool useDistancefield, const std::string& filename) {
    //ctor
    threshold = -1; //negative value means: ignore threshold
    threshold = wallAvoidDistance; //@todo: ar.graf wallavoid, alpha_r and alpha_s must be taken into consideration here!

    if (hxArg != hyArg) std::cerr << "ERROR: hx != hy <=========";
    //parse building and create list of walls/obstacles (find xmin xmax, ymin, ymax, and add border?)
    parseBuilding(buildingArg, hxArg, hyArg);

    //create rectgrid (grid->createGrid())                                                                      <- DONE in parseBuilding

    //'grid->GetnPoints()' and create all data fields: cost, neggradient, speed, dis2wall, flag, secondaryKey      <- DONE in parseBuilding

    //call fkt: linescan und set Distance2Wall mit 0 fuer alle Wandpunkte, speed mit lowspeed                   <- DONE in parseBuilding
    //this step includes Linescanalgorithmus? (maybe included in parsing above)

    //testoutput("AALineScan.vtk", "AALineScan.txt", dist2Wall);

    resetGoalAndCosts(wall, numOfExits);

    calculateDistanceField(-1.); //negative threshold is ignored, so all distances get calculated. this is important since distances is used for slowdown/redirect

    //testoutput("AADistanceField.vtk","AADistanceField.txt", dist2Wall);
    //std::cout<< "Test (50/101): " << grid->getKeyAtXY(50., 101.) << " " << grid->get_x_fromKey(grid->getKeyAtXY(50., 101.)) << " " << grid->get_y_fromKey(grid->getKeyAtXY(50., 101.)) << std::endl;

    calculateFloorfield(useDistancefield); //use distance2Wall

    //testoutput("AAFloorfield.vtk","AAFloorfield.txt", cost);
    writeFF(filename);
}

FloorfieldViaFM::FloorfieldViaFM(const std::string& filename) {

//                    FileHeaderExample: (GEO_UP_SCALE is assumed to be 1.0)
//                    # vtk DataFile Version 3.0
//                    Testdata: Fast Marching: Test:
//                    ASCII
//                    DATASET STRUCTURED_POINTS
//                    DIMENSIONS 322 226 1
//                    ORIGIN 50 90 0
//                    SPACING 0.062500 0.062500 1
//                    POINT_DATA 72772
//                    SCALARS Cost float 1
//                    LOOKUP_TABLE default
//                    0.505725
//                    0.505725
//                    0.505725
//                    ...

// comments show lineformat in .vtk file (below)
    std::ifstream file(filename);
    std::string line;

    std::getline(file, line); //# vtk DataFile Version 3.0
    std::getline(file, line); //Testdata: Fast Marching: Test:
    std::getline(file, line); //ASCII
    std::getline(file, line); //DATASET STRUCTURED_POINTS
    std::getline(file, line); //DIMENSIONS {x} {y} {z}

    std::stringstream inputline(line);

    long int iMax, jMax, c;
    std::string dummy;
    double fdummy;
    long int nPoints;
    double xMin;
    double yMin;
    double xMax;
    double yMax;
    double hx;
    double hy;


    inputline >> dummy >> iMax >> jMax >> c ;

    std::getline(file, line); //ORIGIN x y z
    inputline.str("");
    inputline.clear();
    inputline << line;
    inputline >> dummy >> xMin >> yMin >> c;

    std::getline(file, line); //SPACING 0.062500 0.062500 1
    inputline.str("");
    inputline.clear();
    inputline << line;
    inputline >> dummy >> hx >> hy >> c;
    xMax = xMin + hx*iMax;
    yMax = yMin + hy*jMax;

    std::getline(file, line); //POINT_DATA 72772
    inputline.str("");
    inputline.clear();
    inputline.flush();
    inputline << line;
    inputline >> dummy >> nPoints;

//    std::cerr << inputline.str() << std::endl;
//    std::cerr << hx << std::endl;
//    std::cerr << hy << std::endl;
//    std::cerr << iMax << std::endl;
//    std::cerr << jMax << std::endl;
//    std::cerr << xMin << std::endl;
//    std::cerr << yMin << std::endl;


    //create Rect Grid
    grid = new RectGrid(nPoints, xMin, yMin, xMax, yMax, hx, hy, iMax, jMax, true);

    //create arrays
    flag = new int[nPoints];                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, -7 = outside)
    dist2Wall = new double[nPoints];
    speedInitial = new double[nPoints];
    cost = new double[nPoints];
    neggrad = new Point[nPoints];
    dirToWall = new Point[nPoints];
    trialfield = new Trial[nPoints];                 //created with other arrays, but not initialized yet

    std::getline(file, line);   //SCALARS Cost float 1
    std::getline(file, line);   //LOOKUP_TABLE default

    for (long int i = 0; i < nPoints; ++i) {
        std::getline(file, line);
        inputline.str("");
        inputline << line;
        inputline >> dist2Wall[i];  //0.505725
        //std::cerr << dist2Wall[i] << std::endl;
        inputline.clear();
    }

    std::getline(file, line);       //VECTORS Gradient float

    for (long int i = 0; i < nPoints; ++i) {
        std::getline(file, line);
        inputline.str("");
        inputline << line;
        inputline >> neggrad[i]._x >> neggrad[i]._y >> fdummy;  //0.989337 7.88255 0.0
        inputline.clear();
    }

    std::getline(file, line);       //VECTORS Gradient float

    for (long int i = 0; i < nPoints; ++i) {
        std::getline(file, line);
        inputline.str("");
        inputline << line;
        inputline >> dirToWall[i]._x >> dirToWall[i]._y >> fdummy;  //0.989337 7.88255 0.0
        inputline.clear();
    }
    file.close();
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

void FloorfieldViaFM::getDirectionAt(const Point& position, Point& direction){
    long int key = grid->getKeyAtPoint(position);
    direction.SetX(neggrad[key].GetX());
    direction.SetY(neggrad[key].GetY());
}

void FloorfieldViaFM::getDir2WallAt(const Point& position, Point& direction){
    long int key = grid->getKeyAtPoint(position);
    direction.SetX(dirToWall[key].GetX());
    direction.SetY(dirToWall[key].GetY());
}

double FloorfieldViaFM::getDistance2WallAt(const Point& position) {
    long int key = grid->getKeyAtPoint(position);
    return dist2Wall[key];
}

void FloorfieldViaFM::parseBuilding(const Building* const buildingArg, const double stepSizeX, const double stepSizeY) {
    //init min/max before parsing
    double xMin = FLT_MAX;
    double xMax = -FLT_MAX;
    double yMin = xMin;
    double yMax = xMax;
    //create a list of walls
    const std::map<int, Transition*>& allTransitions = buildingArg->GetAllTransitions();
    for (auto& trans : allTransitions) {
        if (
            //((trans.second->GetRoom1()) && (trans.second->GetRoom1()->GetID() == -1)) ||
            //((trans.second->GetRoom2()) && (trans.second->GetRoom2()->GetID() == -1)) ||
            //(trans.second->GetCaption().compare("main exit") == 0) ||
            //(trans.second->GetType().compare("emergency") == 0)
            trans.second->IsExit()
           )
        {
            wall.emplace_back(Wall( trans.second->GetPoint1()*GEO_UP_SCALE, trans.second->GetPoint2()*GEO_UP_SCALE ));
        }
    }
    numOfExits = wall.size();
    for (const auto& itRoom : buildingArg->GetAllRooms()) {
        for (const auto& itSubroom : itRoom.second->GetAllSubRooms()) {
            std::vector<Obstacle*> allObstacles = itSubroom.second->GetAllObstacles();
            for (std::vector<Obstacle*>::iterator itObstacles = allObstacles.begin(); itObstacles != allObstacles.end(); ++itObstacles) {

                std::vector<Wall> allObsWalls = (*itObstacles)->GetAllWalls();
                for (std::vector<Wall>::iterator itObsWall = allObsWalls.begin(); itObsWall != allObsWalls.end(); ++itObsWall) {
                    wall.emplace_back(Wall( (*itObsWall).GetPoint1()*GEO_UP_SCALE, (*itObsWall).GetPoint2()*GEO_UP_SCALE ));

                    // xMin xMax
                    if ((*itObsWall).GetPoint1().GetX()*GEO_UP_SCALE < xMin) xMin = (*itObsWall).GetPoint1().GetX()*GEO_UP_SCALE;
                    if ((*itObsWall).GetPoint2().GetX()*GEO_UP_SCALE < xMin) xMin = (*itObsWall).GetPoint2().GetX()*GEO_UP_SCALE;
                    if ((*itObsWall).GetPoint1().GetX()*GEO_UP_SCALE > xMax) xMax = (*itObsWall).GetPoint1().GetX()*GEO_UP_SCALE;
                    if ((*itObsWall).GetPoint2().GetX()*GEO_UP_SCALE > xMax) xMax = (*itObsWall).GetPoint2().GetX()*GEO_UP_SCALE;

                    // yMin yMax
                    if ((*itObsWall).GetPoint1().GetY()*GEO_UP_SCALE < yMin) yMin = (*itObsWall).GetPoint1().GetY()*GEO_UP_SCALE;
                    if ((*itObsWall).GetPoint2().GetY()*GEO_UP_SCALE < yMin) yMin = (*itObsWall).GetPoint2().GetY()*GEO_UP_SCALE;
                    if ((*itObsWall).GetPoint1().GetY()*GEO_UP_SCALE > yMax) yMax = (*itObsWall).GetPoint1().GetY()*GEO_UP_SCALE;
                    if ((*itObsWall).GetPoint2().GetY()*GEO_UP_SCALE > yMax) yMax = (*itObsWall).GetPoint2().GetY()*GEO_UP_SCALE;
                }
            }

            std::vector<Wall> allWalls = itSubroom.second->GetAllWalls();
            for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
                wall.emplace_back( Wall((*itWall).GetPoint1()*GEO_UP_SCALE, (*itWall).GetPoint2()*GEO_UP_SCALE) );
                //std::cout << &(*itWall) << std::endl;
                //std::cout << wall[0].GetPoint1().GetX() << std::endl;

                // xMin xMax
                if ((*itWall).GetPoint1().GetX()*GEO_UP_SCALE < xMin) xMin = (*itWall).GetPoint1().GetX()*GEO_UP_SCALE;
                if ((*itWall).GetPoint2().GetX()*GEO_UP_SCALE < xMin) xMin = (*itWall).GetPoint2().GetX()*GEO_UP_SCALE;
                if ((*itWall).GetPoint1().GetX()*GEO_UP_SCALE > xMax) xMax = (*itWall).GetPoint1().GetX()*GEO_UP_SCALE;
                if ((*itWall).GetPoint2().GetX()*GEO_UP_SCALE > xMax) xMax = (*itWall).GetPoint2().GetX()*GEO_UP_SCALE;

                // yMin yMax
                if ((*itWall).GetPoint1().GetY()*GEO_UP_SCALE < yMin) yMin = (*itWall).GetPoint1().GetY()*GEO_UP_SCALE;
                if ((*itWall).GetPoint2().GetY()*GEO_UP_SCALE < yMin) yMin = (*itWall).GetPoint2().GetY()*GEO_UP_SCALE;
                if ((*itWall).GetPoint1().GetY()*GEO_UP_SCALE > yMax) yMax = (*itWall).GetPoint1().GetY()*GEO_UP_SCALE;
                if ((*itWall).GetPoint2().GetY()*GEO_UP_SCALE > yMax) yMax = (*itWall).GetPoint2().GetY()*GEO_UP_SCALE;
            }
        }
    }
    //create Rect Grid
    grid = new RectGrid();
    grid->setBoundaries(xMin, yMin, xMax, yMax);
    grid->setSpacing(stepSizeX, stepSizeY);
    grid->createGrid();

    //create arrays
    flag = new int[grid->GetnPoints()];                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, -7 = outside)
    dist2Wall = new double[grid->GetnPoints()];
    speedInitial = new double[grid->GetnPoints()];
    cost = new double[grid->GetnPoints()];
    neggrad = new Point[grid->GetnPoints()];
    dirToWall = new Point[grid->GetnPoints()];
    trialfield = new Trial[grid->GetnPoints()];                 //created with other arrays, but not initialized yet

    //linescan using (std::vector<Wall*>)
    //lineScan(wall, dist2Wall, 0., -3.);

    //DRAWLINES-Mechanism
    //init grid with -3 as used in lineScan mechanic
    for(long int i = 0; i < grid->GetnPoints(); ++i) {
        dist2Wall[i] = -3.;
    }
    drawLinesOnGrid(wall, dist2Wall, 0.);
}

void FloorfieldViaFM::resetGoalAndCosts(const Goal* const goalArg) {
/* this fkt shall set all Costdata to unknown and the goal points to 0
   we have to watch how to calc indices as goal only knows coords from input file
*/

// set all cost data to "unknown" (remember to change flag accordingly)
// get lines/walls from goals
// use linescan to ititialize cost grid

}

void FloorfieldViaFM::resetGoalAndCosts(std::vector<Wall>& wallArg, int numOfExits) {
    std::vector<Wall> exits(wallArg.begin(), wallArg.begin()+numOfExits);
    drawLinesOnGrid(exits, dist2Wall, -3.); //mark exits as not walls (no malus near exit lines)

    for (long int i = 0; i < grid->GetnPoints(); ++i) {
        if (dist2Wall[i] == 0.) {               //outside
            speedInitial[i] = .001;
            cost[i]         = -7.;  // @todo: ar.graf
            flag[i]         = -7;    // meaning outside
        } else {                                //inside
            speedInitial[i] = 1.;
            cost[i]         = -2.;
            flag[i]         = 0;
        }
        //set Trialptr to fieldelements
        trialfield[i].key = i;
        trialfield[i].flag = flag + i;              //ptr!
        trialfield[i].cost = dist2Wall + i;         //ptr!
        trialfield[i].speed = speedInitial + i;     //ptr!
        trialfield[i].father = nullptr;
        trialfield[i].child = nullptr;
    }
    drawLinesOnGrid(exits, cost, 0.); //already mark targets/exits in cost array (for floorfieldcalc)
    for (long int i=0; i < grid->GetnPoints(); ++i) {
        if (cost[i] == 0.) {
            neggrad[i].SetX(0.);
            neggrad[i].SetY(0.);
            dirToWall[i].SetX(0.);
            dirToWall[i].SetY(0.);
        }
    }
}

void FloorfieldViaFM::lineScan(std::vector<Wall>& wallArg, double* const target, const double outside, const double inside) {
// use RectGrid to go thru lines and check intersection with any wall

// maybe calc and save intersections for each line and then init target array
// (*)
// take line x: calc intersection with wall y: if intersects then store intersectionpoint in a vector<int key>
// now sort vector<int key> and go thru targetline and set values
// empty vector<int key> and continue with line x+1 at (*)

    //grid handeling local vars:
    long int iMax  = grid->GetiMax();
    long int jMax  = grid->GetjMax();
    double xMin             = grid->GetxMin();
    double yMin             = grid->GetyMin();
    double xMax             = grid->GetxMax();
    //double yMax             = grid->GetyMax();
    double hx               = grid->Gethx();
    double hy               = grid->Gethy();
    std::vector<double> xIntersection;

    for(long int j = 0; j < jMax; ++j) { // @todo ar.graf if segfault during writing, check if j < jMax

        //init line with "(outside+inside)/2"
        for (long int initp = 0; initp < iMax; ++initp) {
            target[j*iMax+initp] = (outside + inside) / 2;
        } //init done
        xIntersection.clear();

        Point linestart(xMin,j*hy+yMin);
        Point lineend  (xMax,j*hy+yMin);
        Line currLine = Line(linestart, lineend);
        for(std::vector<Wall>::iterator itWall = wallArg.begin(); itWall != wallArg.end(); ++itWall) {
            //if wall is horizontal, we must deal with it by setting wall value all along
            //note: if wall is horizontal, then adjacent walls should yield intersectionpoints and make
            //      linescan fill the points under the horizontal wall anyway. lets check that. \____/
            if (    ((*itWall).GetPoint1().GetY() == (linestart.GetY()))  &&  ((*itWall).IsHorizontal())   ) {
                long int istart, iend;
                istart = ((*itWall).GetPoint1().GetX() - xMin)/hx + .5;
                iend   = ((*itWall).GetPoint2().GetX() - xMin)/hx + .5;
                if (istart > iend) {
                    long int temp = istart;
                    istart = iend;
                    iend = temp;
                }
                for (long int i = istart; i <= iend; ++i) {
                    target[j*iMax+i] = outside;
                }
            } else {
                double distance = currLine.GetDistanceToIntersectionPoint(*itWall);
                if ( (distance >= 0.) && (distance < 100000) ) {        //check if Line.cpp can be changed (infinity == 100000) seems quite finite
                    xIntersection.push_back(sqrt(distance));
                }
            }
        } //all walls intersected with currLine
        //now init the line using the intersections
        std::unique(xIntersection.begin(), xIntersection.end());
        std::sort(xIntersection.begin(), xIntersection.end());

        long int old = 0;
        long int upTo = 0;
        target[j*iMax+0] = outside;
        double filler = outside;
        for (unsigned int ithCross = 0; ithCross < xIntersection.size(); ++ithCross) {
            upTo = (xIntersection[ithCross])/hx + .5;
            for (long int iCurrSegment = old+1; iCurrSegment < upTo; ++iCurrSegment) {
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
        for (long int rest = old+1; rest < iMax; ++rest) {
            target[j*iMax+rest] = filler;
        }
        //secure check if all gridpoints got set
        for (long int initp = 0; initp < iMax; ++initp) {
            if (  target[j*iMax+initp] == ((outside + inside)/2)  ) {
                //sth went wrong
                std::cerr << "Error in Linescan\n";
            };
        } //sec check done
    } //loop over all lines
}

void FloorfieldViaFM::drawLinesOnGrid(std::vector<Wall>& wallArg, double* const target, const double outside) { //no init, plz init elsewhere
// i~x; j~y;
//http://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm
//src in answer of "Avi"; adapted to fit this application

//    //init with inside value:
//    long int indexMax = grid->GetnPoints();
//    for (long int i = 0; i < indexMax; ++i) {
//        target[i] = inside;
//    }

    //grid handeling local vars:
    long int iMax  = grid->GetiMax();

    long int iStart, iEnd;
    long int jStart, jEnd;
    long int iDot, jDot;
    long int key;
    long int deltaX, deltaY, deltaX1, deltaY1, px, py, xe, ye, i; //Bresenham Algorithm

    for (auto& wall : wallArg) {
        key = grid->getKeyAtPoint(wall.GetPoint1());
        iStart = grid->get_i_fromKey(key);
        jStart = grid->get_j_fromKey(key);

        key = grid->getKeyAtPoint(wall.GetPoint2());
        iEnd = grid->get_i_fromKey(key);
        jEnd = grid->get_j_fromKey(key);

        deltaX = (int) (iEnd - iStart);
        deltaY = (int) (jEnd - jStart);
        deltaX1 = fabs( (int) (iEnd - iStart));
        deltaY1 = fabs( (int) (jEnd - jStart));

        px = 2*deltaY1 - deltaX1;
        py = 2*deltaX1 - deltaY1;

        if(deltaY1<=deltaX1) {
            if(deltaX>=0) {
                iDot = iStart;
                jDot = jStart;
                xe = iEnd;
            } else {
                iDot = iEnd;
                jDot = jEnd;
                xe = iStart;
            }
            target[jDot*iMax + iDot] = outside;
            for (i=0; iDot < xe; ++i) {
                ++iDot;
                if(px<0) {
                    px+=2*deltaY1;
                } else {
                    if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
                        ++jDot;
                    } else {
                        --jDot;
                    }
                    px+=2*(deltaY1-deltaX1);
                }
                target[jDot*iMax + iDot] = outside;
            }
        } else {
            if(deltaY>=0) {
                iDot = iStart;
                jDot = jStart;
                ye = jEnd;
            } else {
                iDot = iEnd;
                jDot = jEnd;
                ye = jStart;
            }
            target[jDot*iMax + iDot] = outside;
            for(i=0; jDot<ye; ++i) {
                ++jDot;
                if (py<=0) {
                    py+=2*deltaX1;
                } else {
                    if((deltaX<0 && deltaY<0) || (deltaX>0 && deltaY>0)) {
                        ++iDot;
                    } else {
                        --iDot;
                    }
                    py+=2*(deltaX1-deltaY1);
                }
                target[jDot*iMax + iDot] = outside;
            }
        }
    } //loop over all walls

} //drawLinesOnGrid

void FloorfieldViaFM::calculateFloorfield(bool useDistance2Wall) {

    Trial* smallest = nullptr;
    Trial* biggest = nullptr;

    double* modifiedspeed = new double[grid->GetnPoints()];
    if (useDistance2Wall && (threshold > 0)) {
        double temp;            //needed to only slowdown band of threshold. outside of band modifiedspeed should be 1
        for (long int i = 0; i < grid->GetnPoints(); ++i) {
            temp = (dist2Wall[i] < threshold) ? dist2Wall[i] : threshold;
            modifiedspeed[i] = 0.001 + 0.999 * (temp/threshold); //linear ramp from wall (0.001) to thresholddistance (1.000)
        }
    } else {
        if (useDistance2Wall) {     //favor middle of hallways/rooms
            for (long int i = 0; i < grid->GetnPoints(); ++i) {
                if (threshold == 0.) {  //special case if user set (use_wall_avoidance = true, wall_avoid_distance = 0.0) and thus wants a plain floorfield
                    modifiedspeed[i] = speedInitial[i];
                } else {                //this is the regular case for "favor middle of hallways/rooms
                    modifiedspeed[i] = 0.001 + 0.999 * (dist2Wall[i]/10); // @todo: ar.graf  (10 ist ein hardgecodeter wert.. sollte ggf. angepasst werden)
                }
            }
        } else {                    //do not use Distance2Wall
            for (long int i = 0; i < grid->GetnPoints(); ++i) {
                modifiedspeed[i] = speedInitial[i];
            }
        }
    }

    //re-init memory
    for (long int i = 0; i < grid->GetnPoints(); ++i) {
        if (dist2Wall[i] == 0.) {               //outside
            //speedInitial[i] = .001;
            //cost[i]         = -7.;  // @todo: ar.graf
            flag[i]         = -7;   // -7 => outside
        } else {                                //inside
            //speedInitial[i] = 1.;
            //cost[i]         = -2.;
            flag[i]         = 0;
        }
        //set Trialptr to fieldelements
        trialfield[i].key = i;
        trialfield[i].flag = flag + i;
        trialfield[i].cost = cost + i;
        trialfield[i].speed = modifiedspeed + i;
        trialfield[i].father = nullptr;
        trialfield[i].child = nullptr;
    }

    for (long int i = 0; i < grid->GetnPoints(); ++i) {
        if (cost[i] == 0.) {
            flag[i] = 3;
        }
    }



    //init narrowband
    for (long int i = 0; i < grid->GetnPoints(); ++i) {
        if (flag[i] == 3) {
            checkNeighborsAndAddToNarrowband(smallest, biggest, i, [&] (const long int key) { this->checkNeighborsAndCalcFloorfield(key); } );
        }
    }

    //inital narrowband done, now loop (while not empty: remove smallest, add neighbors of removed)
    while (smallest != nullptr) {
        long int keyOfSmallest = smallest->key;
        flag[keyOfSmallest] = 3;
        trialfield[keyOfSmallest].removecurr(smallest, biggest, trialfield+keyOfSmallest);
        checkNeighborsAndAddToNarrowband(smallest, biggest, keyOfSmallest, [&] (const long int key) { this->checkNeighborsAndCalcFloorfield(key);} );
    }
    delete[] modifiedspeed;
}

void FloorfieldViaFM::calculateDistanceField(const double thresholdArg) {  //if threshold negative, then ignore it

#ifdef TESTING
    //sanity check (fields <> 0)
    if (flag == 0) return;                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, 4 = added to trial but not calculated, -7 = outside)
    if (dist2Wall == 0) return;
    if (speedInitial == 0) return;
    if (cost == 0) return;
    if (neggrad == 0) return;
    if (trialfield == 0) return;
#endif //TESTING

    //using dist2Wall-array to store this-function's results, (pseudo)"speedfunction" 1 all around
    //stop if smallest value in narrowband is >= threshold

    //  setting startingpoints of wave (dist2Wall = 0) is done in "parseBuilding"

    //  go thru dist2Wall and add every neighbor of "0"s (only if their flag is 0 and therefore "inside")
    Trial* smallest = nullptr;
    Trial* biggest = nullptr;

    for (long int i = 0; i < grid->GetnPoints(); ++i) {
        if (dist2Wall[i] == 0) {
            checkNeighborsAndAddToNarrowband(smallest, biggest, i, [&] (const long int key) { this->checkNeighborsAndCalcDist2Wall(key);} );
        }
    }

    //inital narrowband done, now loop (while not empty: remove smallest, add neighbors of removed)
    while (smallest != nullptr) {
        long int keyOfSmallest = smallest->key;
        flag[keyOfSmallest] = 3;
        if ((thresholdArg > 0) && (trialfield[keyOfSmallest].cost[0] > thresholdArg)) {    //set rest of nearfield and rest of unknown to this max value:

            //rest of nearfield
            Trial* iter = smallest->child;
            while (iter != nullptr) {
                iter[0].cost[0] = trialfield[keyOfSmallest].cost[0];
                iter[0].flag[0] = 3;
                dirToWall[iter[0].key].SetX(0.);
                dirToWall[iter[0].key].SetY(0.);
                iter = iter->child;
            }

            //rest of unknown
            for (long int i = 0; i < grid->GetnPoints(); ++i) {
                if (flag[i] == 0) {
                    flag[i] = 3;
                    dist2Wall[i] = dist2Wall[keyOfSmallest];
                    dirToWall[i].SetX(0.);
                    dirToWall[i].SetY(0.);
                }
            }
            smallest = nullptr;
        } else {
            trialfield[keyOfSmallest].removecurr(smallest, biggest, trialfield+keyOfSmallest);
            checkNeighborsAndAddToNarrowband(smallest, biggest, keyOfSmallest, [&] (const long int key) { this->checkNeighborsAndCalcDist2Wall(key);} );
        }
    }

    //ignore next two lines of comments
    //frage: wo plaziere ich das "update nachbarschaft"? beim adden eines elements? lieber nicht beim calc, da sonst eine rekursion startet
    //bei verbesserung von single to doublesided update ... soll hier auch die nachbarschaft aktualisiert werden?

} //calculateDistancField

void FloorfieldViaFM::checkNeighborsAndAddToNarrowband(Trial* &smallest, Trial* &biggest, const long int key, std::function<void (const long int)> checkNeighborsAndCalc) {
    long int aux = -1;

    directNeighbor dNeigh = grid->getNeighbors(key);

    //check for valid neigh
    aux = dNeigh.key[0];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, 4 = added to trial but not calculated, -7 = outside)
    if ((aux != -2) && (flag[aux] == 0)) {
        flag[aux] = 4;      //4 = added to trial but not calculated
        checkNeighborsAndCalc(aux);
        trialfield[aux].insert(smallest, biggest, trialfield + aux);
    }
    aux = dNeigh.key[1];
    if ((aux != -2) && (flag[aux] == 0)) {
        flag[aux] = 4;      //4 = added to trial but not calculated
        checkNeighborsAndCalc(aux);
        trialfield[aux].insert(smallest, biggest, trialfield + aux);
    }
    aux = dNeigh.key[2];
    if ((aux != -2) && (flag[aux] == 0)) {
        flag[aux] = 4;      //4 = added to trial but not calculated
        checkNeighborsAndCalc(aux);
        trialfield[aux].insert(smallest, biggest, trialfield + aux);
    }
    aux = dNeigh.key[3];
    if ((aux != -2) && (flag[aux] == 0)) {
        flag[aux] = 4;      //4 = added to trial but not calculated
        checkNeighborsAndCalc(aux);
        trialfield[aux].insert(smallest, biggest, trialfield + aux);
    }
}

void FloorfieldViaFM::checkNeighborsAndCalcDist2Wall(const long int key) {
    double row;
    double col;
    long int aux;
    bool pointsUp;
    bool pointsRight;

    row = 100000.;
    col = 100000.;
    aux = -1;

    directNeighbor dNeigh = grid->getNeighbors(key);

    aux = dNeigh.key[0];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, 4 = added to trial but not calculated, -7 = outside)
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0))                                                      //gridpoint holds a calculated value
    {
        row = trialfield[aux].cost[0];
        pointsRight = true;
        if (row < 0) {
            std::cerr << "hier ist was schief " << row << " " << aux << " " << flag[aux] << std::endl;
            row = 100000;
        }
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }
    aux = dNeigh.key[2];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0) &&                                                    //gridpoint holds a calculated value
         (trialfield[aux].cost[0] < row))                                       //calculated value promises smaller cost
    {
        row = trialfield[aux].cost[0];
        pointsRight = false;
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }

    aux = dNeigh.key[1];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, 4 = added to trial but not calculated, -7 = outside)
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0))                                                      //gridpoint holds a calculated value
    {
        col = trialfield[aux].cost[0];
        pointsUp = true;
        if (col < 0) {
            std::cerr << "hier ist was schief " << col << " " << aux << " " << flag[aux] << std::endl;
            col = 100000;
        }
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }
    aux = dNeigh.key[3];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0) &&                                                    //gridpoint holds a calculated value
         (trialfield[aux].cost[0] < col))                                       //calculated value promises smaller cost
    {
        col = trialfield[aux].cost[0];
        pointsUp = false;
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }
    if (col == 100000.) { //one sided update with row
        trialfield[key].cost[0] = onesidedCalc(row, grid->Gethx());
        trialfield[key].flag[0] = 1;
        if (pointsRight) {
            dirToWall[key].SetX(-(cost[key+1]-cost[key])/grid->Gethx());
            dirToWall[key].SetY(0.);
        } else {
            dirToWall[key].SetX(-(cost[key]-cost[key-1])/grid->Gethx());
            dirToWall[key].SetY(0.);
        }
        dirToWall[key] = dirToWall[key].Normalized();
        return;
    }

    if (row == 100000.) { //one sided update with col
        trialfield[key].cost[0] = onesidedCalc(col, grid->Gethy());
        trialfield[key].flag[0] = 1;
        if (pointsUp) {
            dirToWall[key].SetX(0.);
            dirToWall[key].SetY(-(cost[key+(grid->GetiMax())]-cost[key])/grid->Gethy());
        } else {
            dirToWall[key].SetX(0.);
            dirToWall[key].SetY(-(cost[key]-cost[key-(grid->GetiMax())])/grid->Gethy());
        }
        dirToWall[key] = dirToWall[key].Normalized();
        return;
    }

    //two sided update
    double precheck = twosidedCalc(row, col, grid->Gethx());
    if (precheck >= 0) {
        trialfield[key].cost[0] = precheck;
        trialfield[key].flag[0] = 2;
        if (pointsUp && pointsRight) {
            dirToWall[key].SetX(-(cost[key+1]-cost[key])/grid->Gethx());
            dirToWall[key].SetY(-(cost[key+(grid->GetiMax())]-cost[key])/grid->Gethy());
        }
        if (pointsUp && !pointsRight) {
            dirToWall[key].SetX(-(cost[key]-cost[key-1])/grid->Gethx());
            dirToWall[key].SetY(-(cost[key+(grid->GetiMax())]-cost[key])/grid->Gethy());
        }
        if (!pointsUp && pointsRight) {
            dirToWall[key].SetX(-(cost[key+1]-cost[key])/grid->Gethx());
            dirToWall[key].SetY(-(cost[key]-cost[key-(grid->GetiMax())])/grid->Gethy());
        }
        if (!pointsUp && !pointsRight) {
            dirToWall[key].SetX(-(cost[key]-cost[key-1])/grid->Gethx());
            dirToWall[key].SetY(-(cost[key]-cost[key-(grid->GetiMax())])/grid->Gethy());
        }
    } else {
        std::cerr << "else in twosided Dist " << std::endl;
    }
    dirToWall[key] = dirToWall[key].Normalized();
}

void FloorfieldViaFM::checkNeighborsAndCalcFloorfield(const long int key) {
    double row;
    double col;
    long int aux;
    bool pointsUp;
    bool pointsRight;

    row = 100000.;
    col = 100000.;
    aux = -1;


    directNeighbor dNeigh = grid->getNeighbors(key);

    aux = dNeigh.key[0];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, 4 = added to trial but not calculated, -7 = outside)
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0) &&
         (flag[aux] != -7))                                                      //gridpoint holds a calculated value
    {
        row = trialfield[aux].cost[0];
        pointsRight = true;
        if (row < 0) {
            std::cerr << "hier ist was schief " << row << " " << aux << " " << flag[aux] << std::endl;
            row = 100000;
        }
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }
    aux = dNeigh.key[2];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0) &&
         (flag[aux] != -7) &&                                                 //gridpoint holds a calculated value
         (trialfield[aux].cost[0] < row))                                       //calculated value promises smaller cost
    {
        row = trialfield[aux].cost[0];
        pointsRight = false;
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }

    aux = dNeigh.key[1];
    //hint: trialfield[i].cost = dist2Wall + i; <<< set in parseBuilding after linescan call
    //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, 4 = added to trial but not calculated, -7 = outside)
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0) &&
         (flag[aux] != -7))                                                      //gridpoint holds a calculated value
    {
        col = trialfield[aux].cost[0];
        pointsUp = true;
        if (col < 0) {
            std::cerr << "hier ist was schief " << col << " " << aux << " " << flag[aux] << std::endl;
            col = 100000;
        }
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }
    aux = dNeigh.key[3];
    if  ((aux != -2) &&                                                         //neighbor is a gridpoint
         (flag[aux] != 0) &&
         (flag[aux] != -7) &&                                                  //gridpoint holds a calculated value
         (trialfield[aux].cost[0] < col))                                       //calculated value promises smaller cost
    {
        col = trialfield[aux].cost[0];
        pointsUp = false;
        //todo: add directioninfo to calc neggradient later OR recheck neighbor later again
    }
    if (col == 100000.) { //one sided update with row
        trialfield[key].cost[0] = onesidedCalc(row, grid->Gethx()/trialfield[key].speed[0]);
        trialfield[key].flag[0] = 1;
        if (pointsRight) {
            neggrad[key].SetX(-(cost[key+1]-cost[key])/grid->Gethx());
            neggrad[key].SetY(0.);
        } else {
            neggrad[key].SetX(-(cost[key]-cost[key-1])/grid->Gethx());
            neggrad[key].SetY(0.);
        }
        return;
    }

    if (row == 100000.) { //one sided update with col
        trialfield[key].cost[0] = onesidedCalc(col, grid->Gethy()/trialfield[key].speed[0]);
        trialfield[key].flag[0] = 1;
        if (pointsUp) {
            neggrad[key].SetX(0.);
            neggrad[key].SetY(-(cost[key+(grid->GetiMax())]-cost[key])/grid->Gethy());
        } else {
            neggrad[key].SetX(0.);
            neggrad[key].SetY(-(cost[key]-cost[key-(grid->GetiMax())])/grid->Gethy());
        }
        return;
    }

    //two sided update
    double precheck = twosidedCalc(row, col, grid->Gethx()/trialfield[key].speed[0]);
    if (precheck >= 0) {
        trialfield[key].cost[0] = precheck;
        trialfield[key].flag[0] = 2;
        if (pointsUp && pointsRight) {
            neggrad[key].SetX(-(cost[key+1]-cost[key])/grid->Gethx());
            neggrad[key].SetY(-(cost[key+(grid->GetiMax())]-cost[key])/grid->Gethy());
        }
        if (pointsUp && !pointsRight) {
            neggrad[key].SetX(-(cost[key]-cost[key-1])/grid->Gethx());
            neggrad[key].SetY(-(cost[key+(grid->GetiMax())]-cost[key])/grid->Gethy());
        }
        if (!pointsUp && pointsRight) {
            neggrad[key].SetX(-(cost[key+1]-cost[key])/grid->Gethx());
            neggrad[key].SetY(-(cost[key]-cost[key-(grid->GetiMax())])/grid->Gethy());
        }
        if (!pointsUp && !pointsRight) {
            neggrad[key].SetX(-(cost[key]-cost[key-1])/grid->Gethx());
            neggrad[key].SetY(-(cost[key]-cost[key-(grid->GetiMax())])/grid->Gethy());
        }
    } else {
        std::cerr << "else in twosided Floor " << precheck << " " << row << " " << col << std::endl;
    }
}

void FloorfieldViaFM::update(const long int key, double* target, double* speedlocal) {  //oblolete and NOT implemented
    //wenn ein wert berechnet wurde bei (key), dann schaue in seiner nachbarschaft
    //ob ein singlecalc'ed value (flag == 1) ist. dieser kann ggf. verbessert werden.
    directNeighbor dNeigh = grid->getNeighbors(key);
    //keine schleife, sondern jeden nachbarn separat behandeln
    //bei singled diagonal nach berechnetem wert absuchen und mit ihm zusammen den nachbarn verbessern
    //bei         zweitem in der linie und mit ihm zentralen diff-quotienten bilden um (nachbarn oder self) zu verbessern
    // ich glaube: self
    long int aux = dNeigh.key[0];
    if ( (aux != -2) && (flag[aux] == 1) ) {

    }

} //update

inline double FloorfieldViaFM::onesidedCalc(double xy, double hDivF) {
    //if (xy < 0) std::cerr << "error in onesided " << xy << std::endl;   //todo: performance
    return xy + hDivF;
}

inline double FloorfieldViaFM::twosidedCalc(double x, double y, double hDivF) { //on error return -2
    double determinante = (2*hDivF*hDivF - (x-y)*(x-y));
    if (determinante >= 0) {
        return (x + y + sqrt(determinante))/2;
    } else {
        return (x < y) ? (x + hDivF) : (y + hDivF);
    }
    std::cerr << "error in two-sided 2!!!!!!!!!!!!!!!!!!!!!!! o_O??" << std::endl;
    return -2.; //this line should never execute
} //twosidedCalc

void FloorfieldViaFM::testoutput(const char* filename1, const char* filename2, const double* target) {
//security debug check
    std::ofstream file;
    std::ofstream file2;
    int numX = (int) ((grid->GetxMax()-grid->GetxMin())/grid->Gethx());
    int numY = (int) ((grid->GetyMax()-grid->GetyMin())/grid->Gethy());
    int numTotal = numX * numY;
    //std::cerr << numTotal << " numTotal" << std::endl;
    //std::cerr << grid->GetnPoints() << " grid" << std::endl;
    file.open(filename1);
    file2.open(filename2);
    file << "# vtk DataFile Version 3.0" << std::endl;
    file << "Testdata: Fast Marching: Test: " << std::endl;
    file << "ASCII" << std::endl;
    file << "DATASET STRUCTURED_POINTS" << std::endl;
    file << "DIMENSIONS " <<
                                std::to_string(grid->GetiMax()) <<
                                " " <<
                                std::to_string(grid->GetjMax()) <<
                                " 1" << std::endl;
    file << "ORIGIN " << grid->GetxMin()/GEO_UP_SCALE << " " << grid->GetyMin()/GEO_UP_SCALE << " 0" << std::endl;
    file << "SPACING " << std::to_string(grid->Gethx()/GEO_UP_SCALE) << " " << std::to_string(grid->Gethy()/GEO_UP_SCALE) << " 1" << std::endl;
    file << "POINT_DATA " << std::to_string(numTotal) << std::endl;
    file << "SCALARS Cost float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (long int i = 0; i < grid->GetnPoints(); ++i) {
        file << target[i] << std::endl;
        Point iPoint = grid->getPointFromKey(i);
        file2 << iPoint.GetX()/GEO_UP_SCALE /*- grid->GetxMin()*/ << " " << iPoint.GetY()/GEO_UP_SCALE /*- grid->GetyMin()*/ << " " << target[i] << std::endl;
    }

    if (target == cost) {
        file << "VECTORS Gradient float" << std::endl;
        for (int i = 0; i < grid->GetnPoints(); ++i) {
            file << neggrad[i].GetX()/GEO_UP_SCALE << " " << neggrad[i].GetY()/GEO_UP_SCALE << " 0.0" << std::endl;
        }
    }

    file.close();
    file2.close();

    //std::cerr << "INFO: \tFile closed: " << filename1 << std::endl;
}

void FloorfieldViaFM::writeFF(const std::string& filename) {
    std::ofstream file;

    int numX = (int) ((grid->GetxMax()-grid->GetxMin())/grid->Gethx());
    int numY = (int) ((grid->GetyMax()-grid->GetyMin())/grid->Gethy());
    int numTotal = numX * numY;
    //std::cerr << numTotal << " numTotal" << std::endl;
    //std::cerr << grid->GetnPoints() << " grid" << std::endl;
    file.open(filename);

    file << "# vtk DataFile Version 3.0" << std::endl;
    file << "Testdata: Fast Marching: Test: " << std::endl;
    file << "ASCII" << std::endl;
    file << "DATASET STRUCTURED_POINTS" << std::endl;
    file << "DIMENSIONS " <<
                                std::to_string(grid->GetiMax()) <<
                                " " <<
                                std::to_string(grid->GetjMax()) <<
                                " 1" << std::endl;
    file << "ORIGIN " << grid->GetxMin()/GEO_UP_SCALE << " " << grid->GetyMin()/GEO_UP_SCALE << " 0" << std::endl;
    file << "SPACING " << std::to_string(grid->Gethx()/GEO_UP_SCALE) << " " << std::to_string(grid->Gethy()/GEO_UP_SCALE) << " 1" << std::endl;
    file << "POINT_DATA " << std::to_string(numTotal) << std::endl;
    file << "SCALARS Dist2Wall float 1" << std::endl;
    file << "LOOKUP_TABLE default" << std::endl;
    for (long int i = 0; i < grid->GetnPoints(); ++i) {
        file << dist2Wall[i]/GEO_UP_SCALE << std::endl; //@todo: change target to all dist2wall
        //Point iPoint = grid->getPointFromKey(i);
        //file2 << iPoint.GetX()/GEO_UP_SCALE /*- grid->GetxMin()*/ << " " << iPoint.GetY()/GEO_UP_SCALE /*- grid->GetyMin()*/ << " " << target[i] << std::endl;
    }

    file << "VECTORS Gradient float" << std::endl;
    for (int i = 0; i < grid->GetnPoints(); ++i) {
        file << neggrad[i].GetX()/GEO_UP_SCALE << " " << neggrad[i].GetY()/GEO_UP_SCALE << " 0.0" << std::endl;
    }

    file << "VECTORS Dir2Wall float" << std::endl;
    for (int i = 0; i < grid->GetnPoints(); ++i) {
        file << dirToWall[i].GetX()/GEO_UP_SCALE << " " << dirToWall[i].GetY()/GEO_UP_SCALE << " 0.0" << std::endl;
    }

    if (cost != nullptr) {
        file << "SCALARS Cost float 1" << std::endl;
        file << "LOOKUP_TABLE default" << std::endl;
        for (long int i = 0; i < grid->GetnPoints(); ++i) {
            file << cost[i]/GEO_UP_SCALE << std::endl;
        }
    }
    file.close();
}
