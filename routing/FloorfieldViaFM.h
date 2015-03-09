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
#include "mesh/RectGrid.h"
#include "../geometry/Wall.h"
#include "../geometry/Point.h"

//maybe put following in macros.h
#define LOWSPEED 0.001

class FloorfieldViaFM
{
    public:
        FloorfieldViaFM();
        FloorfieldViaFM(Building* buildingArg, double hxArg, double hyArg);
        virtual ~FloorfieldViaFM();
        FloorfieldViaFM(const FloorfieldViaFM& other);
        FloorfieldViaFM& operator=(const FloorfieldViaFM& other);


    protected:
    private:
        RectGrid* grid;
        std::vector<Wall*> wall;
        unsigned long int nWalls; //wall.size() yields the same

        //stuff to handle wrapper grid
        double offsetX;
        double offsetY;

        //GridPoint Data in independant arrays (shared primary key)
        int* flag;
        double* dist2Wall;
        double* speedInital;
        double* cost;
        unsigned long int* secKey;
        Point* grad;

};

#endif // FLOORFIELDVIAFM_H
