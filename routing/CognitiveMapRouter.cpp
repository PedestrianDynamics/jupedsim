/**
 * @file   CognitiveMapRouter.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   February, 2014
 * @brief  Routing Engine for Cognitive Map
 *
 */

#include "CognitiveMapRouter.h"
#include "Router.h"

#include "cognitive_map/CognitiveMapStorage.h"
#include "cognitive_map/CognitiveMap.h"
#include "cognitive_map/NavigationGraph.h"
#include "cognitive_map/sensor/SensorManager.h"

#include "../geometry/SubRoom.h"
#include "../geometry/NavLine.h"
#include "../geometry/Building.h"

#include "../pedestrian/Pedestrian.h"
#include "../tinyxml/tinyxml.h"

CognitiveMapRouter::CognitiveMapRouter()
{
}

CognitiveMapRouter::~CognitiveMapRouter()
{
    delete cm_storage;

}

int CognitiveMapRouter::FindExit(Pedestrian * p)
{
    //check for former goal.
    if(p->GetLastDestination() == -1) {
        //no former goal. so initial route has to be choosen
        //this is needed for initialisation
        p->ChangedSubRoom();
    }

    if(p->GetNextDestination() == -1 || p->ChangedSubRoom()) {
        sensor_manager->execute(p);

        const NavLine * destination = (*cm_storage)[p]->GetDestination();
        if(destination != NULL) {
            p->SetExitLine(destination);
            p->SetExitIndex(destination->GetUniqueID());
        }
    }

    return 1;

}

void CognitiveMapRouter::Init(Building * b)
{
    Log->Write("INFO:\tInit the Cognitive Map  Router Engine");
    building = b;

    //Load Routing file with HLines and further informations.
    LoadRoutingInfos(GetRoutingInfoFile());

    //Init Cognitive Map Storage
    cm_storage = new CognitiveMapStorage(building);
    Log->Write("INFO:\tInitialized CognitiveMapStorage");
    //Init Sensor Manager
    sensor_manager = SensorManager::InitWithAllSensors(b, cm_storage);
    Log->Write("INFO:\tInitialized SensorManager");
}


void CognitiveMapRouter::LoadRoutingInfos(const std::string &filename){

    if(filename=="") return;

    Log->Write("INFO:\tLoading extra routing information for the CognitiveMap Router.");
    Log->Write("INFO:\t  from file "+filename);

    TiXmlDocument docRouting(filename);
    if (!docRouting.LoadFile()){
        Log->Write("ERROR: \t%s", docRouting.ErrorDesc());
        Log->Write("ERROR: \t could not parse the routing file [%s]",filename.c_str());
        exit(EXIT_FAILURE);
    }

    TiXmlElement* xRootNode = docRouting.RootElement();
    if( ! xRootNode ) {
        Log->Write("ERROR:\tRoot element does not exist");
        exit(EXIT_FAILURE);
    }

    if( xRootNode->ValueStr () != "routing" ) {
        Log->Write("ERROR:\tRoot element value is not 'routing'.");
        exit(EXIT_FAILURE);
    }

    std::string  version = xRootNode->Attribute("version");
    if (version != JPS_VERSION) {
        Log->Write("ERROR: \tOnly version  %d.%d supported",JPS_VERSION_MAJOR,JPS_VERSION_MINOR);
        Log->Write("ERROR: \tparsing routing file failed!");
        exit(EXIT_FAILURE);
    }

    for(TiXmlElement* xHlinesNode = xRootNode->FirstChildElement("Hlines"); xHlinesNode;
        xHlinesNode = xHlinesNode->NextSiblingElement("Hlines")) {


        for(TiXmlElement* hline = xHlinesNode->FirstChildElement("Hline"); hline;
            hline = hline->NextSiblingElement("Hline")) {

            double id = xmltof(hline->Attribute("id"), -1);
            int room_id = xmltoi(hline->Attribute("room_id"), -1);
            int subroom_id = xmltoi(hline->Attribute("subroom_id"), -1);

            double x1 = xmltof(	hline->FirstChildElement("vertex")->Attribute("px"));
            double y1 = xmltof(	hline->FirstChildElement("vertex")->Attribute("py"));
            double x2 = xmltof(	hline->LastChild("vertex")->ToElement()->Attribute("px"));
            double y2 = xmltof(	hline->LastChild("vertex")->ToElement()->Attribute("py"));

            Room* room = building->GetRoom(room_id);
            SubRoom* subroom = room->GetSubRoom(subroom_id);

            //new implementation
            Hline* h = new Hline();
            h->SetID(id);
            h->SetPoint1(Point(x1, y1));
            h->SetPoint2(Point(x2, y2));
            h->SetRoom(room);
            h->SetSubRoom(subroom);

            building->AddHline(h);
            subroom->AddHline(h);
        }
    }
    Log->Write("INFO:\tDone with loading extra routing information");
}


std::string CognitiveMapRouter::GetRoutingInfoFile() const {

    TiXmlDocument doc(building->GetProjectFilename());
    if (!doc.LoadFile()){
        Log->Write("ERROR: \t%s", doc.ErrorDesc());
        Log->Write("ERROR: \t could not parse the project file");
        exit(EXIT_FAILURE);
    }

    // everything is fine. proceed with parsing
    TiXmlElement* xMainNode = doc.RootElement();
    TiXmlNode* xRouters=xMainNode->FirstChild("route_choice_models");

    std::string nav_line_file="";

    for(TiXmlElement* e = xRouters->FirstChildElement("router"); e;
        e = e->NextSiblingElement("router")) {

        std::string strategy=e->Attribute("description");


        if (e->FirstChild("parameters")->FirstChildElement("navigation_lines"))
            nav_line_file=e->FirstChild("parameters")->FirstChildElement("navigation_lines")->Attribute("file");

    }
    if (nav_line_file == "")
        return nav_line_file;
    else
        return building->GetProjectRootDir()+nav_line_file;
}
