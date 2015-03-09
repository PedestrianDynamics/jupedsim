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

FloorfieldViaFM::FloorfieldViaFM(Building* buildingArg, double hxArg, double hyArg) {
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
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void FloorfieldViaFM::parseBuilding(Building* const buildingArg) {
    //create a list of walls
    std::vector<Room*>& allRooms = buildingArg->GetAllRooms();
    for (std::vector<Room*>::iterator itRoom = allRooms.begin(); itRoom != allRooms.end(); ++itRoom) {

        std::vector<SubRoom*>& allSubrooms = (*itRoom)->GetAllSubRooms();
        for (std::vector<SubRoom*>::iterator itSubroom = allSubrooms.begin(); itSubroom != allSubrooms.end(); ++itSubroom) {

            std::vector<Obstacle*>& allObstacles = (*itSubroom)->GetAllObstacles();
            for (std::vector<Obstacle*>::iterator itObstacles = allObstacles.begin(); itObstacles != allObstacles.end(); ++itObstacles) {

                std::vector<Wall>& allObsWalls = (*itObstacles)->GetAllWalls();
                for (std::vector<Wall>::iterator itObsWall = allObsWalls.begin(); itObsWall != allObsWalls.end(); ++itObsWall) {
                    wall.push_back(&(*itObsWall));
                }
            }

            std::vector<Wall>& allWalls = (*itSubroom)->GetAllWalls();
            for (std::vector<Wall>::iterator itWall = allWalls.begin(); itWall != allWalls.end(); ++itWall) {
                wall.push_back(&(*itWall));
            }
        }
    }
    //std::vector<Room*> _rooms
}

void FloorfieldViaFM::calculateDistanceField() {
    //sanity check (fields <> 0)

}
