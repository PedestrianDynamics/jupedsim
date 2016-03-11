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
          cognitive_maps[key]->AddRegions(_regions);
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
         Log->Write("No landmarks specified");
         return;
    }

    TiXmlElement* xRootNode = docGeo.RootElement();
    if( ! xRootNode ) {
         Log->Write("ERROR:\tRoot element does not exist");
         Log->Write("No landmarks specified");
         return ;
    }

    if( xRootNode->ValueStr () != "geometry" ) {
         Log->Write("ERROR:\tRoot element value is not 'geometry'.");
         Log->Write("No landmarks specified");
         return;
    }
    if(xRootNode->Attribute("unit"))
         if(std::string(xRootNode->Attribute("unit")) != "m") {
              Log->Write("ERROR:\tOnly the unit m (meters) is supported. \n\tYou supplied [%s]",xRootNode->Attribute("unit"));
              Log->Write("No landmarks specified");
              return;
         }

    double version = xmltof(xRootNode->Attribute("version"), -1);

    if (version != std::stod(JPS_VERSION) && version != std::stod(JPS_OLD_VERSION)) {
         Log->Write(" \tWrong geometry version!");
         Log->Write(" \tOnly version >= %s supported",JPS_VERSION);
         Log->Write(" \tPlease update the version of your geometry file to %s",JPS_VERSION);
         Log->Write("No landmarks specified");
         return;
    }

    //processing the regions node
    TiXmlNode*  xRegionsNode = xRootNode->FirstChild("regions");
    if (!xRegionsNode) {
         Log->Write("ERROR: \tGeometry file without regions definition!");
         Log->Write("No landmarks specified");
         return;
    }

    for(TiXmlElement* xRegion = xRegionsNode->FirstChildElement("region"); xRegion;
              xRegion = xRegion->NextSiblingElement("region"))
    {

        std::string id = xmltoa(xRegion->Attribute("id"), "-1");
        std::string caption = xmltoa(xRegion->Attribute("caption"));
        std::string pxinmap = xmltoa(xRegion->Attribute("px"),"-1");
        std::string pyinmap = xmltoa(xRegion->Attribute("py"),"-1");
        std::string a = xmltoa(xRegion->Attribute("a"),"-1");
        std::string b = xmltoa(xRegion->Attribute("b"),"-1");

        ptrRegion region (new Region(Point(std::stod(pxinmap),std::stod(pyinmap))));

        region->SetId(std::stoi(id));
        region->SetCaption(caption);
        region->SetPosInMap(Point(std::stod(pxinmap),std::stod(pyinmap)));
        region->SetA(std::stod(a));
        region->SetB(std::stod(b));



        //processing the landmarks node
        TiXmlNode*  xLandmarksNode = xRegion->FirstChild("landmarks");
        if (!xLandmarksNode) {
             Log->Write("ERROR: \tGeometry file without landmark definition!");
             Log->Write("No landmarks specified");
             return;
        }



        for(TiXmlElement* xLandmark = xLandmarksNode->FirstChildElement("landmark"); xLandmark;
                  xLandmark = xLandmark->NextSiblingElement("landmark"))
        {

            std::string id = xmltoa(xLandmark->Attribute("id"), "-1");
            std::string caption = xmltoa(xLandmark->Attribute("caption"));
            std::string type = xmltoa(xLandmark->Attribute("type"),"-1");
            std::string roomId = xmltoa(xLandmark->Attribute("subroom1_id"),"-1");
            std::string pxreal = xmltoa(xLandmark->Attribute("pxreal"),"-1");
            std::string pyreal = xmltoa(xLandmark->Attribute("pyreal"),"-1");
            std::string pxinmap = xmltoa(xLandmark->Attribute("px"),"-1");
            std::string pyinmap = xmltoa(xLandmark->Attribute("py"),"-1");
            std::string a = xmltoa(xLandmark->Attribute("a"),"-1");
            std::string b = xmltoa(xLandmark->Attribute("b"),"-1");

            ptrLandmark landmark (new Landmark(Point(std::stod(pxreal),std::stod(pyreal))));

            landmark->SetId(std::stoi(id));
            landmark->SetCaption(caption);
            landmark->SetType(type);
            landmark->SetRealPos(Point(std::stod(pxreal),std::stod(pyreal)));
            landmark->SetPosInMap(Point(std::stod(pxinmap),std::stod(pyinmap)));
            landmark->SetA(std::stod(a));
            landmark->SetB(std::stod(b));
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
                int connection = std::stoi(xmltoa(xAsso->Attribute("connectedwith"),"-1"));
                //std::string priority = xmltoa(xAsso->Attribute("priority"),"-1");

                ptrLandmark assolandmark (new Landmark(Point(std::stod(asso_x),std::stod(asso_y)),
                                                    std::stod(asso_a),std::stod(asso_b)));
                assolandmark->SetId(std::stod(asso_id));
                std::cout << assolandmark->GetId() << std::endl;
                assolandmark->SetCaption(asso_caption);
                //assolandmark->AddConnection(std::stoi(connection));
                //assolandmark->SetPriority(std::stod(priority));
                bool connected=false;
                if (connection==landmark->GetId())
                    connected=true;
                landmark->AddAssociation(std::make_shared<Association>(landmark,assolandmark,connected));


            }

            region->AddLandmark(landmark);
            Log->Write("INFO:\tLandmark added!");

        }

        //processing the connections node
        TiXmlNode*  xConnectionsNode = xRegion->FirstChild("connections");
        if (!xConnectionsNode) {
             //Log->Write("ERROR: \tGeometry file without landmark definition!");
             Log->Write("No connections specified");
             return;
        }



        for(TiXmlElement* xConnection = xConnectionsNode->FirstChildElement("connection"); xConnection;
                  xConnection = xConnection->NextSiblingElement("connection"))
        {
            std::string id = xmltoa(xConnection->Attribute("id"), "-1");
            std::string caption = xmltoa(xConnection->Attribute("caption"));
            std::string type = xmltoa(xConnection->Attribute("type"),"-1");
            std::string landmark1 = xmltoa(xConnection->Attribute("landmark1_id"),"-1");
            std::string landmark2 = xmltoa(xConnection->Attribute("landmark2_id"),"-1");


            ptrConnection connection = std::make_shared<Connection>(std::stoi(id),caption,type,
                                                                    region->GetLandmarkByID(std::stoi(landmark1)),
                                                                    region->GetLandmarkByID(std::stoi(landmark2)));
            region->AddConnection(connection);
            Log->Write("INFO:\tConnection added!");
        }

        _regions.push_back(region);
        Log->Write("INFO:\tRegion added!");
    }
}

void CognitiveMapStorage::CreateCognitiveMap(CMStorageKeyType ped)
{
     //todo: the possibility to have more then one creator.
     cognitive_maps.insert(std::make_pair(ped, creator->CreateCognitiveMap(ped)));
     cognitive_maps[ped]->AddRegions(_regions);
     cognitive_maps[ped]->InitLandmarkNetworksInRegions();
     cognitive_maps[ped]->FindMainDestination();
     //debug
     //cognitive_maps[ped]->GetNavigationGraph()->WriteToDotFile(building->GetProjectRootDir());
}
