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
#include "BrainStorage.h"

#include "pedestrian/Pedestrian.h"

#include <tinyxml.h>

BrainStorage::BrainStorage(const Building * const b, std::string cogMapStatus, std::string cogMapFiles)
     : _building(b)
{
    _cogMapStatus=cogMapStatus;
    _cogMapFiles=cogMapFiles;


    //internal graph networks in every subroom
    for (auto it=_building->GetAllRooms().begin(); it!=_building->GetAllRooms().end(); ++it)
    {
        for (auto it2=it->second->GetAllSubRooms().begin(); it2!=it->second->GetAllSubRooms().end(); ++it2)
        {
            InitInternalNetwork(it2->second.get());
        }
    }
}

BrainStorage::~BrainStorage()
{

}

BStorageValueType BrainStorage::operator[] (BStorageKeyType key)
{
     BStorageType::iterator it = _brains.find(key);
     if(it == _brains.end()) {
          CreateBrain(key);
     }

//     PStorageType::iterator itP= perception_abilities.find(key);
//     if (itP == perception_abilities.end())
//     {
//         CreatePerceptionAbility(key);
//     }

     return _brains[key];
}

void BrainStorage::ParseCogMap(BStorageKeyType ped)
{

    _regions.clear();

    //create filename

    int groupId = ped->GetGroup();
    if (_cogMapFiles=="")
        return;
    std::string cMFileName=_cogMapFiles;
    cMFileName.replace(cMFileName.size()-4,4,std::to_string(groupId)+".xml");

    auto cogMapFilenameWithPath = _building->GetProjectRootDir() / cMFileName;

    Log->Write(cogMapFilenameWithPath.string());
    TiXmlDocument doccogMap(cogMapFilenameWithPath.string());
    if (!doccogMap.LoadFile()) {
         Log->Write("ERROR: \t%s", doccogMap.ErrorDesc());
         Log->Write("\t could not parse the cognitive map file");
         Log->Write("Cognitive map not specified");
         return;
    }

    TiXmlElement* xRootNode = doccogMap.RootElement();
    if( ! xRootNode ) {
         Log->Write("ERROR:\tRoot element does not exist");
         Log->Write("Cognitive map not specified");
         return;
    }

    if( xRootNode->ValueStr () != "cognitiveMap" ) {
         Log->Write("ERROR:\tRoot element value is not 'cognitiveMap'.");
         Log->Write("Cognitive map not specified");
         return;
    }
    if(xRootNode->Attribute("unit"))
         if(std::string(xRootNode->Attribute("unit")) != "m") {
              Log->Write("ERROR:\tOnly the unit m (meters) is supported. \n\tYou supplied [%s]",xRootNode->Attribute("unit"));
              Log->Write("Cognitive map not specified");
              return;
         }

    double version = xmltof(xRootNode->Attribute("version"), -1);

    if (version != std::stod(JPS_VERSION) && version != std::stod(JPS_OLD_VERSION)) {
         Log->Write(" \tWrong geometry version!");
         Log->Write(" \tOnly version >= %s supported",JPS_VERSION);
         Log->Write(" \tPlease update the version of your geometry file to %s",JPS_VERSION);
         Log->Write("Cognitive map not specified");
         return;
    }

    //processing the regions node
    TiXmlNode*  xRegionsNode = xRootNode->FirstChild("regions");
    if (!xRegionsNode) {
         Log->Write("ERROR: \tCognitive map file without region definition!");
         Log->Write("Cognitive map not specified");
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
             Log->Write("ERROR: \tCognitive map file without landmark definition!");
             Log->Write("No landmarks specified");
             return;
        }



        for(TiXmlElement* xLandmark = xLandmarksNode->FirstChildElement("landmark"); xLandmark;
                  xLandmark = xLandmark->NextSiblingElement("landmark"))
        {

            std::string landmark_id = xmltoa(xLandmark->Attribute("id"), "-1");
            std::string landmark_caption = xmltoa(xLandmark->Attribute("caption"));
            std::string type = xmltoa(xLandmark->Attribute("type"),"-1");
            std::string roomId = xmltoa(xLandmark->Attribute("subroom1_id"),"-1");
            std::string pxreal = xmltoa(xLandmark->Attribute("pxreal"),"-1");
            std::string pyreal = xmltoa(xLandmark->Attribute("pyreal"),"-1");
            std::string landmark_pxinmap = xmltoa(xLandmark->Attribute("px"),"-1");
            std::string landmark_pyinmap = xmltoa(xLandmark->Attribute("py"),"-1");
            std::string landmark_a = xmltoa(xLandmark->Attribute("a"),"-1");
            std::string landmark_b = xmltoa(xLandmark->Attribute("b"),"-1");

            ptrLandmark landmark (new Landmark(Point(std::stod(pxreal),std::stod(pyreal))));

            if (roomId=="NaN")
            {
                Log->Write("ERROR:\t Subroom Id is NaN!");
                return;
            }
            landmark->SetId(std::stoi(landmark_id));
            landmark->SetCaption(landmark_caption);
            landmark->SetType(type);
            landmark->SetRealPos(Point(std::stod(pxreal),std::stod(pyreal)));
            landmark->SetPosInMap(Point(std::stod(landmark_pxinmap),std::stod(landmark_pyinmap)));
            landmark->SetA(std::stod(landmark_a));
            landmark->SetB(std::stod(landmark_b));
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
                //std::cout << assolandmark->GetId() << std::endl;
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
             //return;
        }

        else
        {
            for(TiXmlElement* xConnection = xConnectionsNode->FirstChildElement("connection"); xConnection;
                      xConnection = xConnection->NextSiblingElement("connection"))
            {
                std::string idC = xmltoa(xConnection->Attribute("id"), "-1");
                std::string captionC = xmltoa(xConnection->Attribute("caption"));
                std::string typeC = xmltoa(xConnection->Attribute("type"),"-1");
                std::string landmark1 = xmltoa(xConnection->Attribute("landmark1_id"),"-1");
                std::string landmark2 = xmltoa(xConnection->Attribute("landmark2_id"),"-1");


                ptrConnection connection = std::make_shared<Connection>(std::stoi(idC),captionC,typeC,
                                                                        region->GetLandmarkByID(std::stoi(landmark1)),
                                                                        region->GetLandmarkByID(std::stoi(landmark2)));

                region->AddConnection(connection);
                Log->Write("INFO:\tConnection added!");
            }
        }

        _regions.push_back(region);
        Log->Write("INFO:\tRegion added!");
    }
}

void BrainStorage::CreateBrain(BStorageKeyType ped)
{

     //todo: the possibility to have more then one creator.

     _brains.insert(std::make_pair(ped, std::make_shared<Brain>(_building,
                                                                ped,
                                                                &_roominternalNetworks)));//  creator->CreateCognitiveMap(ped)));


     // if cognitive map is complete (complete graph network is known by every ped)
     if (_cogMapStatus=="complete")
     {
         //adding all SubRooms as Vertex
         for(auto&& itr_room: _building->GetAllRooms())
         {
              for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
              {
                   _brains[ped]->GetCognitiveMap().GetGraphNetwork()->Add(itr_subroom.second.get());
              }
         }

         //Add crossings as edges
         for(auto&& itr_cross: _building->GetAllCrossings())
         {
             if (itr_cross.second->IsOpen())
              _brains[ped]->GetCognitiveMap().GetGraphNetwork()->Add(itr_cross.second);
         }

         //Add transitions as edges
         for(auto&& itr_trans: _building->GetAllTransitions())
         {
              if (itr_trans.second->IsOpen())
              {
                  if(itr_trans.second->IsExit())
                  {
                       _brains[ped]->GetCognitiveMap().GetGraphNetwork()->AddExit(itr_trans.second);
                  }
                  else
                  {
                       _brains[ped]->GetCognitiveMap().GetGraphNetwork()->Add(itr_trans.second);
                  }
              }
         }
     }

     // load cogmap and do first cognitive step
     ParseCogMap(ped);
     _brains[ped]->GetCognitiveMap().AddRegions(_regions);
     _brains[ped]->GetCognitiveMap().InitLandmarkNetworksInRegions();
     _brains[ped]->GetCognitiveMap().FindMainDestination();
     //debug
     //cognitive_maps[ped]->GetNavigationGraph()->WriteToDotFile(building->GetProjectRootDir());

}

void BrainStorage::InitInternalNetwork(const SubRoom* sub_room)
{

    _roominternalNetworks.emplace(sub_room,std::make_shared<InternNavigationNetwork>(sub_room));

    for (Crossing* crossing:sub_room->GetAllCrossings())
    {
        _roominternalNetworks[sub_room]->AddVertex(crossing);
    }

    for (Transition* transition:sub_room->GetAllTransitions())
    {
        _roominternalNetworks[sub_room]->AddVertex(transition);
    }

    for (Hline* hline:sub_room->GetAllHlines())
    {
        _roominternalNetworks[sub_room]->AddVertex(hline);
    }

    _roominternalNetworks[sub_room]->EstablishConnections();

}
