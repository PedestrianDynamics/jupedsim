/**
 * \file        CognitiveMapStorage.cpp
 * \date        Feb 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * Cognitive Map Storage
 *
 *
 **/


#include "CognitiveMapStorage.h"
#include "AbstractCognitiveMapCreator.h"
#include "EmptyCognitiveMapCreator.h"
#include "CompleteCognitiveMapCreator.h"

#include "../../tinyxml/tinyxml.h"
#include <memory>

#include "../../geometry/Building.h"
//#include "NavigationGraph.h"


CognitiveMapStorage::CognitiveMapStorage(const Building * const b, std::string cogMapStatus)
     : _building(b)
{
    if (cogMapStatus == "empty")
    creator = new EmptyCognitiveMapCreator(b);
    else
    creator = new CompleteCognitiveMapCreator(b);
}

CognitiveMapStorage::~CognitiveMapStorage()
{
     delete creator;
     for (auto it=cognitive_maps.begin(); it!=cognitive_maps.end(); ++it)
     {
        delete it->second;
     }
     cognitive_maps.clear();
}

CMStorageValueType CognitiveMapStorage::operator[] (CMStorageKeyType key)
{
     CMStorageType::iterator it = cognitive_maps.find(key);
     if(it == cognitive_maps.end()) {
          CreateCognitiveMap(key);
          cognitive_maps[key]->AddLandmarksSC(_landmarks);
     }

     return cognitive_maps[key];
}

void CognitiveMapStorage::ParseLandmarks()
{
    std::string geoFilenameWithPath = _building->GetProjectRootDir() + _building->GetGeometryFilename();

    TiXmlDocument docGeo(geoFilenameWithPath);
    if (!docGeo.LoadFile()) {
         Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
         Log->Write("\t could not parse the geometry file");
         Log->Write("WARNING:No waypoints specified");
         return;
    }

    TiXmlElement* xRootNode = docGeo.RootElement();
    if( ! xRootNode ) {
         Log->Write("ERROR:\tRoot element does not exist");
         Log->Write("WARNING:No waypoints specified");
         return ;
    }

    if( xRootNode->ValueStr () != "geometry" ) {
         Log->Write("ERROR:\tRoot element value is not 'geometry'.");
         Log->Write("WARNING:No waypoints specified");
         return;
    }
    if(xRootNode->Attribute("unit"))
         if(std::string(xRootNode->Attribute("unit")) != "m") {
              Log->Write("ERROR:\tOnly the unit m (meters) is supported. \n\tYou supplied [%s]",xRootNode->Attribute("unit"));
              Log->Write("WARNING:No waypoints specified");
              return;
         }

    double version = xmltof(xRootNode->Attribute("version"), -1);

    if (version != std::stod(JPS_VERSION) && version != std::stod(JPS_OLD_VERSION)) {
         Log->Write(" \tWrong geometry version!");
         Log->Write(" \tOnly version >= %s supported",JPS_VERSION);
         Log->Write(" \tPlease update the version of your geometry file to %s",JPS_VERSION);
         Log->Write("WARNING:No waypoints specified");
         return;
    }


    //processing the rooms node
    TiXmlNode*  xLandmarksNode = xRootNode->FirstChild("landmarks");
    if (!xLandmarksNode) {
         Log->Write("WARNING: \tGeometry file without landmark definition!");
         Log->Write("WARNING:No waypoints specified");
         return;
    }

    for(TiXmlElement* xLandmark = xLandmarksNode->FirstChildElement("landmark"); xLandmark;
              xLandmark = xLandmark->NextSiblingElement("landmark"))
    {

        std::string id = xmltoa(xLandmark->Attribute("id"), "-1");
        std::string caption = xmltoa(xLandmark->Attribute("caption"));
        std::string roomId = xmltoa(xLandmark->Attribute("subroom1_id"),"-1");
        std::string lx = xmltoa(xLandmark->Attribute("px"),"-1");
        std::string ly = xmltoa(xLandmark->Attribute("py"),"-1");

        ptrLandmark landmark (new Landmark(Point(std::stod(lx),std::stod(ly))));

        landmark->SetId(std::stoi(id));
        landmark->SetCaption(caption);
        landmark->SetRoom(_building->GetSubRoomByUID(std::stoi(roomId)));

        //processing the rooms node
        TiXmlNode*  xAssociationsNode = xLandmark->FirstChild("associations");
        if (!xAssociationsNode) {
             Log->Write("Landmark with no association!");
             continue;
        }

        for(TiXmlElement* xAsso = xAssociationsNode->FirstChildElement("association"); xAsso;
           xAsso = xAsso->NextSiblingElement("association"))
        {
            std::string asso_id = xmltoa(xAsso->Attribute("id"), "-1");
            std::string asso_caption = xmltoa(xAsso->Attribute("caption"), "0");
            //std::string asso_type = xmltoa(xAsso->Attribute("type"),"-1");
            std::string asso_x = xmltoa(xAsso->Attribute("px"),"-1");
            std::string asso_y = xmltoa(xAsso->Attribute("py"),"-1");
            std::string asso_a = xmltoa(xAsso->Attribute("a"),"-1");
            std::string asso_b = xmltoa(xAsso->Attribute("b"),"-1");
            std::string priority = xmltoa(xAsso->Attribute("priority"),"-1");

            ptrWaypoint waypoint (new Waypoint(Point(std::stod(asso_x),std::stod(asso_y)),
                                                std::stod(asso_a),std::stod(asso_b)));
            waypoint->SetId(std::stod(asso_id));
            waypoint->SetCaption(asso_caption);
            waypoint->SetPriority(std::stod(priority));

            landmark->AddAssociation(std::make_shared<Association>(landmark,waypoint));
        }

        _landmarks.push_back(landmark);
        Log->Write("INFO:\tLandmark added!");

    }

}

void CognitiveMapStorage::CreateCognitiveMap(CMStorageKeyType ped)
{
     //todo: the possibility to have more then one creator.
     cognitive_maps.insert(std::make_pair(ped, creator->CreateCognitiveMap(ped)));

     //debug
     //cognitive_maps[ped]->GetNavigationGraph()->WriteToDotFile(building->GetProjectRootDir());
}
