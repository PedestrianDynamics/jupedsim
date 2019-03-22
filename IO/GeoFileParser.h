/**
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
 **/
//
// Created by laemmel on 30.03.16.
//

#ifndef JPSCORE_GEOFILEPARSER_H
#define JPSCORE_GEOFILEPARSER_H

#include "../general/Configuration.h"
#include "../geometry/Building.h"
#include "../geometry/GeometryReader.h"
#include "../geometry/Trips.h"
#include "../tinyxml/tinyxml.h"

//TODO: the class name GeoFileParser is misleading as the ``geometry'' file contains among others also relations (transitions)
//TODO: between geometries/rooms. Probably, EnvironmentFileParser would be better, still parts of the environment are
//TODO: currently stored in the ``ini'' file (e.g. goals) [gl march '16]
//TODO: currently stored in the ``ini'' file (e.g. goals) [gl march '16]
class GeoFileParser : public GeometryReader {

public:
     GeoFileParser(Configuration* configuration);
     ~GeoFileParser();
     virtual void LoadBuilding(Building* building) override;

     virtual bool LoadTrafficInfo(Building* building) override;
     bool parseDoorNode(TiXmlElement * xDoor, int id, Building* building);
     Goal* parseGoalNode(TiXmlElement * e);
     Transition* parseTransitionNode(TiXmlElement * xTrans, Building * building);
     Goal* parseWaitingAreaNode(TiXmlElement * e);

private:
     Configuration* _configuration;

     bool LoadGeometry(Building* building);

     bool LoadRoutingInfo(Building* filename);

     bool ReadGoal();

     bool ReadWaitingArea();
};

#endif //JPSCORE_GEOFILEPARSER_H
