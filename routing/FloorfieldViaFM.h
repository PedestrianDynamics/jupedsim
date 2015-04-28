/**
 * \file        FloorfieldViaFM.h
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

#ifndef FLOORFIELDVIAFM_H
#define FLOORFIELDVIAFM_H

#include <vector>
#include <cmath>
#include "mesh/RectGrid.h"
#include "../geometry/Wall.h"
#include "../geometry/Point.h"
#include "../geometry/Building.h"
#include "../geometry/SubRoom.h" //check: should Room.h include SubRoom.h??
#include "../routing/mesh/Trial.h"

//maybe put following in macros.h
#define LOWSPEED 0.001

class FloorfieldViaFM
{
    public:
        FloorfieldViaFM();
        FloorfieldViaFM(const Building* const buildingArg, const double hxArg, const double hyArg);
        virtual ~FloorfieldViaFM();
        FloorfieldViaFM(const FloorfieldViaFM& other);
        //FloorfieldViaFM& operator=(const FloorfieldViaFM& other);

        void parseBuilding(const Building* const buildingArg, const double stepSizeX, const double stepSizeY);
        void resetGoalAndCosts(const Goal* const goalArg);
        void lineScan(std::vector<Wall>& wallArg, double* const target, const double outside, const double inside);
        void calculateDistanceField(const double threshold);

        void update(const long int key, double* target, double* speedlocal);

#ifdef TESTING
        void setGrid(RectGrid* gridArg) {grid = gridArg;}
        Trial* getTrial() {return trialfield;}
#endif //TESTING

    protected:
    private:
        RectGrid* grid;
        std::vector<Wall> wall;


        //stuff to handle wrapper grid (unused, cause RectGrid handles offset)
        double offsetX;
        double offsetY;

        //GridPoint Data in independant arrays (shared primary key)
        int* flag;                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final)
        double* dist2Wall;
        double* speedInitial;
        double* cost;
        long int* secKey;  //secondary key to address ... not used yet
        Point* grad; //gradients
        Trial* trialfield;

};

#endif // FLOORFIELDVIAFM_H
