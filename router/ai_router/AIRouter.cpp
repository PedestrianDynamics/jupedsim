/**
 * \file        AIRouter.cpp
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
 *
 * TODO update to work with door states (OPEN, CLOSE, TEMP_CLOSE)
 *
 **/
#include "AIRouter.h"

#include "BrainStorage.h"
#include "router/ai_router/cognitiveMap/cognitivemap.h"
#include "geometry/Building.h"
#include "geometry/NavLine.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "router/Router.h"

#include <tinyxml.h>

AIRouter::AIRouter()
{
    building=nullptr;
//    cm_storage=nullptr;
    //sensor_manager=nullptr;

}

AIRouter::AIRouter(int id, RoutingStrategy s) : Router(id, s)
{
    building=nullptr;
//    cm_storage=nullptr;
    //sensor_manager=nullptr;
}

AIRouter::~AIRouter()
{
     //delete brain_storage;
     //delete sensor_manager;

}

int AIRouter::FindExit(Pedestrian * p)
{

    // Update isovist and return next target every given update step
    if (std::fmod(std::roundf(p->GetGlobalTime()*100),std::roundf(1.0/AI_UPDATE_RATE*100))==0.0)// && p->GetGlobalTime()>0)
    {
        // Find next destination
        int status = FindDestination(p);

        return status;

    }
    return 1;
}

int AIRouter::FindDestination(Pedestrian * p)
{

        //--------------------COGMAP----------------------------

        //Update isovists
        // Update (red) currently visible polygon
        (*brain_storage)[p]->GetPerceptionAbilities().UpdateCurrentEnvironment();
        // Update (blue) already explored polygon
        (*brain_storage)[p]->GetPerceptionAbilities().UpdateSeenEnv();

        // update cogmap
        (*brain_storage)[p]->GetCognitiveMap().UpdateMap();

        //Find next appropriate landmark
        (*brain_storage)[p]->GetCognitiveMap().FindNextTarget();

        // find possible crossings in seenEnvironment e.g. to proceed to landmark
        std::vector<NavLine> navLines = (*brain_storage)[p]->GetPerceptionAbilities().GetPossibleNavLines(
                    (*brain_storage)[p]->GetPerceptionAbilities().GetSeenEnvironmentAsPointVec(building->GetRoom(p->GetRoomID())));

        NavLine nextNvLine=(*brain_storage)[p]->FindApprCrossing(navLines);

        std::ofstream myfile2;
        std::string str2 = "./isovists/navLinesBefore_pos"+std::to_string(p->GetID())+"_"+std::to_string((int)(std::round(p->GetGlobalTime()*100)))+".txt";
        myfile2.open (str2);
        myfile2 << std::to_string(nextNvLine.GetCentre()._x) << " " << std::to_string(nextNvLine.GetCentre()._y) << std::endl;
        myfile2.close();


        // find possible crossings in currentEnv to proceed to nextNvLine
        navLines = (*brain_storage)[p]->GetPerceptionAbilities().GetPossibleNavLines(
                    (*brain_storage)[p]->GetPerceptionAbilities().GetCurrentEnvironmentAsPointVec());
        //Find appropriate visible crossing to reach next app. landmark
        nextNvLine=(*brain_storage)[p]->FindApprVisibleCrossing(nextNvLine,navLines);



        std::ofstream myfile;
        std::string str = "./isovists/navLines_pos"+std::to_string(p->GetID())+"_"+std::to_string((int)(std::round(p->GetGlobalTime()*100)))+".txt";
        myfile.open (str);
        myfile << std::to_string(nextNvLine.GetCentre()._x) << " " << std::to_string(nextNvLine.GetCentre()._y) << std::endl;
        myfile.close();

        p->SetExitLine(&nextNvLine);

        return 1;
}



bool AIRouter::Init(Building * b)
{
     Log->Write("INFO:\tInit the Cognitive Map Router Engine");
     building = b;

     LoadRoutingInfos(GetRoutingInfoFile());

     //Init Cognitive Map Storage, second parameter: decides whether cognitive Map is empty or complete
     if (getOptions().find("CognitiveMapFiles")==getOptions().end() && getOptions().find("SignFiles")==getOptions().end())
        brain_storage = std::unique_ptr<AIBrainStorage>(new AIBrainStorage(building));
     else if (getOptions().find("CognitiveMapFiles")==getOptions().end())
         brain_storage = std::unique_ptr<AIBrainStorage>(new AIBrainStorage(building,"",getOptions().at("SignFiles")[0]));
     else if (getOptions().find("SignFiles")==getOptions().end())
        brain_storage = std::unique_ptr<AIBrainStorage>(new AIBrainStorage(building,getOptions().at("CognitiveMapFiles")[0]));
     else
         brain_storage = std::unique_ptr<AIBrainStorage>(
                 new AIBrainStorage(building,getOptions().at("CognitiveMapFiles")[0],
                                  getOptions().at("SignFiles")[0]));

     Log->Write("INFO:\tCognitiveMapStorage initialized");
     //cm_storage->ParseCogMap();

     //Init Sensor Manager
     //sensor_manager = SensorManager::InitWithAllSensors(b, cm_storage);
     //sensor_manager = SensorManager::InitWithCertainSensors(b, brain_storage.get(), getOptions());
     //Log->Write("INFO:\tSensorManager initialized");
     return true;
}


const optStorage &AIRouter::getOptions() const
{
    return options;
}

void AIRouter::addOption(const std::string &key, const std::vector<std::string> &value)
{
    options.insert(std::make_pair(key, value));
}

bool AIRouter::LoadRoutingInfos(const fs::path &filename)
{
    if(filename.empty()) return true;

    Log->Write("INFO:\tLoading extra routing information for the global/quickest path router");
    Log->Write("INFO:\t  from the file "+filename.string());

    TiXmlDocument docRouting(filename.string());
    if (!docRouting.LoadFile()) {
         Log->Write("ERROR: \t%s", docRouting.ErrorDesc());
         Log->Write("ERROR: \t could not parse the routing file [%s]",filename.string().c_str());
         return false;
    }

    TiXmlElement* xRootNode = docRouting.RootElement();
    if( ! xRootNode ) {
         Log->Write("ERROR:\tRoot element does not exist");
         return false;
    }

    if( xRootNode->ValueStr () != "routing" ) {
         Log->Write("ERROR:\tRoot element value is not 'routing'.");
         return false;
    }

    std::string  version = xRootNode->Attribute("version");
    if (version < JPS_OLD_VERSION) {
         Log->Write("ERROR: \tOnly version greater than %d supported",JPS_OLD_VERSION);
         Log->Write("ERROR: \tparsing routing file failed!");
         return false;
    }
    int HlineCount = 0;
    for(TiXmlElement* xHlinesNode = xRootNode->FirstChildElement("Hlines"); xHlinesNode;
              xHlinesNode = xHlinesNode->NextSiblingElement("Hlines")) {


         for(TiXmlElement* hline = xHlinesNode->FirstChildElement("Hline"); hline;
                   hline = hline->NextSiblingElement("Hline")) {

              double id = xmltof(hline->Attribute("id"), -1);
              int room_id = xmltoi(hline->Attribute("room_id"), -1);
              int subroom_id = xmltoi(hline->Attribute("subroom_id"), -1);

              double x1 = xmltof(     hline->FirstChildElement("vertex")->Attribute("px"));
              double y1 = xmltof(     hline->FirstChildElement("vertex")->Attribute("py"));
              double x2 = xmltof(     hline->LastChild("vertex")->ToElement()->Attribute("px"));
              double y2 = xmltof(     hline->LastChild("vertex")->ToElement()->Attribute("py"));

              Room* room = building->GetRoom(room_id);
              SubRoom* subroom = room->GetSubRoom(subroom_id);

              //new implementation
              Hline* h = new Hline();
              h->SetID(id);
              h->SetPoint1(Point(x1, y1));
              h->SetPoint2(Point(x2, y2));
              h->SetRoom1(room);
              h->SetSubRoom1(subroom);

              if(building->AddHline(h))
              {
                   subroom->AddHline(h);
                   HlineCount++;
                   //h is freed in building
              }
              else
              {
                   delete h;
              }
         }
    }
    Log->Write("INFO:\tDone with loading extra routing information. Loaded <%d> Hlines", HlineCount);
    return true;
}

fs::path AIRouter::GetRoutingInfoFile()
{

    TiXmlDocument doc(building->GetProjectFilename().string());
    if (!doc.LoadFile()) {
         Log->Write("ERROR: \t%s", doc.ErrorDesc());
         Log->Write("ERROR: \t GlobalRouter: could not parse the project file");
         return "";
    }

    // everything is fine. proceed with parsing
    TiXmlElement* xMainNode = doc.RootElement();
    TiXmlNode* xRouters=xMainNode->FirstChild("route_choice_models");
    fs::path nav_line_file{};

    for(TiXmlElement* e = xRouters->FirstChildElement("router"); e;
              e = e->NextSiblingElement("router"))
    {

         std::string strategy=e->Attribute("description");

         if(strategy=="cognitive_map")
         {
              if(e->FirstChild("parameters"))
              {
                   if (e->FirstChild("parameters")->FirstChildElement("navigation_lines"))
                        nav_line_file=e->FirstChild("parameters")->FirstChildElement("navigation_lines")->Attribute("file");
              }
         }
    }

    if (nav_line_file.empty())
         return nav_line_file;
    else
        return building->GetProjectRootDir() / nav_line_file;
}

void AIRouter::DeleteCortex(const Pedestrian *ped)
{
    brain_storage->DeleteCortex(ped);

}
