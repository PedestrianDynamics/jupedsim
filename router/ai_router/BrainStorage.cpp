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

#include "router/ai_router/cognitiveMap/internnavigationnetwork.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <tinyxml.h>

#include <memory>

AIBrainStorage::AIBrainStorage(const Building * const b, const std::string &cogMapFiles, const std::string &signFiles)
     : _building(b),_visibleEnv(VisibleEnvironment(b)),_cogMapFiles(cogMapFiles),_signFiles(signFiles)
{

    //Set signs
    ParseSigns();

}

Cortex* AIBrainStorage::operator[] (BStorageKeyType key)
{
     BStorageType::iterator it = _corteces.find(key);
     if(it == _corteces.end()) {
          CreateCortex(key);
     }

     return _corteces[key].get();
}

void AIBrainStorage::ParseCogMap(BStorageKeyType ped)
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

    if (version < std::stod(JPS_OLD_VERSION) ) {
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

        std::string idR = xmltoa(xRegion->Attribute("id"), "-1");
        std::string captionR = xmltoa(xRegion->Attribute("caption"));
        std::string pxinmapR = xmltoa(xRegion->Attribute("px"),"-1");
        std::string pyinmapR = xmltoa(xRegion->Attribute("py"),"-1");
        std::string aR = xmltoa(xRegion->Attribute("a"),"-1");
        std::string bR = xmltoa(xRegion->Attribute("b"),"-1");

        AIRegion region (Point(std::stod(pxinmapR),std::stod(pyinmapR)));
        region.SetId(std::stoi(idR));

        region.SetCaption(captionR);
        region.SetPosInMap(Point(std::stod(pxinmapR),std::stod(pyinmapR)));
        region.SetA(std::stod(aR));
        region.SetB(std::stod(bR));



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

            AILandmark landmark(Point(std::stod(pxreal),std::stod(pyreal)));

            if (roomId=="NaN")
            {
                Log->Write("ERROR:\t Subroom Id is NaN!");
                return;
            }
            landmark.SetId(std::stoi(id));
            landmark.SetCaption(caption);
            landmark.SetType(type);
            landmark.SetRealPos(Point(std::stod(pxreal),std::stod(pyreal)));
            landmark.SetPosInMap(Point(std::stod(pxinmap),std::stod(pyinmap)));
            landmark.SetA(std::stod(a));
            landmark.SetB(std::stod(b));
            //landmark->SetRoom(_building->GetSubRoomByUID(std::stoi(roomId)));

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

                AILandmark assolandmark(Point(std::stod(asso_x),std::stod(asso_y)),
                                                    std::stod(asso_a),std::stod(asso_b));
                assolandmark.SetId(std::stod(asso_id));
                //std::cout << assolandmark.GetId() << std::endl;
                assolandmark.SetCaption(asso_caption);
                //assolandmark->AddConnection(std::stoi(connection));
                //assolandmark->SetPriority(std::stod(priority));
                bool connected=false;
                if (connection==landmark.GetId())
                    connected=true;
                landmark.AddAssociation(AIAssociation(&landmark,&assolandmark,connected));

                region.AddLandmarkSubCs(assolandmark);


            }

            region.AddLandmark(landmark);
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


                AIConnection connection = AIConnection(std::stoi(idC),captionC,typeC,std::stoi(landmark1),std::stoi(landmark2));

                region.AddConnection(connection);
                Log->Write("INFO:\tConnection added!");
            }
        }

        _regions.push_back(region);
        Log->Write("INFO:\tRegion added!");
    }
}

void AIBrainStorage::CreateCortex(BStorageKeyType ped)
{

     //todo: the possibility to have more then one creator.

     _corteces.insert(std::make_pair(ped, std::unique_ptr<Cortex>(new Cortex(_building,
                                                                ped,
                                                                &_visibleEnv,
                                                                &_roominternalNetworks))));//  creator->CreateCognitiveMap(ped)));




     // load cogmap and do first cognitive step
     ParseCogMap(ped);
     _corteces[ped]->GetCognitiveMap().AddRegions(_regions);
     _corteces[ped]->GetCognitiveMap().InitLandmarkNetworksInRegions();
     _corteces[ped]->GetCognitiveMap().FindMainDestination();
     //debug
     //cognitive_maps[ped]->GetNavigationGraph()->WriteToDotFile(building->GetProjectRootDir());
}

void AIBrainStorage::InitInternalNetwork(const SubRoom* sub_room)
{

    _roominternalNetworks.emplace(sub_room,ptrIntNetwork(new InternNavigationNetwork(sub_room)));

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

void AIBrainStorage::ParseSigns() {

    if (_signFiles=="")
    {
        Log->Write("INFO: \tNo signs specified \n");
        return;
    }
    auto signFilenameWithPath = _building->GetProjectRootDir() / _signFiles;

    Log->Write("INFO: \tRead Signs from %s", signFilenameWithPath.string().c_str());
    TiXmlDocument docSigns(signFilenameWithPath.string());
    if (!docSigns.LoadFile()) {
        Log->Write("WARNING: \t%s", docSigns.ErrorDesc());
        Log->Write("\t could not parse the sign file");
        Log->Write("No signs specified");
        return;
    }

    TiXmlElement *xRootNode = docSigns.RootElement();
    if (!xRootNode) {
        Log->Write("WARNING:\tRoot element does not exist");
        Log->Write("No signs specified");
        return;
    }

    if (xRootNode->ValueStr() != "signage") {
        Log->Write("WARNING:\tRoot element value is not 'signage'.");
        Log->Write("No signs specified");
        return;
    }
    if (xRootNode->Attribute("unit"))
        if (std::string(xRootNode->Attribute("unit")) != "m") {
            Log->Write("WARNING:\tOnly the unit m (meters) is supported. \n\tYou supplied [%s]",
                       xRootNode->Attribute("unit"));
            Log->Write("No signs specified");
            return;
        }

    double version = xmltof(xRootNode->Attribute("version"), -1);

    if (version < std::stod(JPS_OLD_VERSION)) {
        Log->Write("WARNING: \tWrong file version!");
        Log->Write(" \tOnly version >= %s supported", JPS_VERSION);
        Log->Write(" \tPlease update the version of your file to %s", JPS_VERSION);
        Log->Write("No signs specified");
        return;
    }

    //processing the regions node
    TiXmlNode *xSignsNode = xRootNode->FirstChild("signs");
    if (!xSignsNode) {
        Log->Write("WARNING: \tSignage file without signs!");
        Log->Write("No signs specified");
        return;
    }


    for (TiXmlElement *xSign = xSignsNode->FirstChildElement("sign"); xSign;
         xSign = xSign->NextSiblingElement("sign")) {

        std::string id = xmltoa(xSign->Attribute("id"), "-1");
        std::string room_id = xmltoa(xSign->Attribute("room_id"), "-1");
        std::string px = xmltoa(xSign->Attribute("px"), "-1");
        std::string py = xmltoa(xSign->Attribute("py"), "-1");
        std::string alpha = xmltoa(xSign->Attribute("alpha"), "-1");
        std::string alphaPointing = xmltoa(xSign->Attribute("alphaPointing"), "-1");

        Log->Write("Sign read!");

        _visibleEnv.AddSign(_building->GetAllRooms().at(std::stod(room_id)).get(),Sign(std::stod(id), std::stod(room_id), Point(std::stod(px), std::stod(py)),
                             std::stod(alpha), std::stod(alphaPointing)));

    }

}

void AIBrainStorage::DeleteCortex(BStorageKeyType ped)
{
    _corteces.erase(ped);

}
