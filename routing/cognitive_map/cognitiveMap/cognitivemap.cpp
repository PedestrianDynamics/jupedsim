#include "cognitivemap.h"
#include "../../../geometry/Point.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Ellipse.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../tinyxml/tinyxml.h"

CognitiveMap::CognitiveMap()
{

}


CognitiveMap::CognitiveMap(ptrBuilding b, ptrPed ped)
{
    _building=b;
    _ped=ped;
}

CognitiveMap::~CognitiveMap()
{

}

void CognitiveMap::AddLandmark(ptrLandmark landmark)
{
    if (std::find(_landmarks.begin(), _landmarks.end(), landmark)!=_landmarks.end())
    {
        return;
    }
    else
        _landmarks.push_back(landmark);
}


void CognitiveMap::ParseLandmarks(const std::string &geometryfile)
{
    std::string geoFilenameWithPath = _building->GetProjectRootDir() + geometryfile;

    TiXmlDocument docGeo(geoFilenameWithPath);
    if (!docGeo.LoadFile()) {
         Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
         Log->Write("\t could not parse the geometry file");
         Log->Write("No waypoints specified");
         return;
    }

    TiXmlElement* xRootNode = docGeo.RootElement();
    if( ! xRootNode ) {
         Log->Write("ERROR:\tRoot element does not exist");
         Log->Write("No waypoints specified");
         return ;
    }

    if( xRootNode->ValueStr () != "geometry" ) {
         Log->Write("ERROR:\tRoot element value is not 'geometry'.");
         Log->Write("No waypoints specified");
         return;
    }
    if(xRootNode->Attribute("unit"))
         if(string(xRootNode->Attribute("unit")) != "m") {
              Log->Write("ERROR:\tOnly the unit m (meters) is supported. \n\tYou supplied [%s]",xRootNode->Attribute("unit"));
              Log->Write("No waypoints specified");
              return;
         }

    double version = xmltof(xRootNode->Attribute("version"), -1);

    if (version != std::stod(JPS_VERSION) && version != std::stod(JPS_OLD_VERSION)) {
         Log->Write(" \tWrong geometry version!");
         Log->Write(" \tOnly version >= %s supported",JPS_VERSION);
         Log->Write(" \tPlease update the version of your geometry file to %s",JPS_VERSION);
         Log->Write("No waypoints specified");
         return;
    }


    //processing the rooms node
    TiXmlNode*  xLandmarksNode = xRootNode->FirstChild("landmarks");
    if (!xRoomsNode) {
         Log->Write("ERROR: \tGeometry file without landmark definition!");
         Log->Write("No waypoints specified");
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

        ptrLandmark landmark = new Landmark(Point(std::stod(lx),std::stod(ly)));

        landmark->SetId(std::stoi(id));
        landmark->SetCaption(caption);
        landmark->SetRoom(_building->GetSubRoomByUID(std::stoi(roomId)));


        for(TiXmlElement* xAsso = xAsso->FirstChildElement("association"); xAsso;
           xAsso = xAsso->NextSiblingElement("association"))
        {

            std::string asso_id = xmltoa(xAsso->Attribute("id"), "-1");
            std::string asso_caption = xmltoa(xAsso->Attribute("caption"), "0");
            //std::string asso_type = xmltoa(xAsso->Attribute("type"),"-1");
            std::string asso_x = xmltoa(xAsso->Attribute("px"),"-1");
            std::string asso_y = xmltoa(xAsso->Attribute("py"),"-1");
            std::string asso_a = xmltoa(xAsso->Attribute("a"),"-1");
            std::string asso_b = xmltoa(xAsso->Attribute("b"),"-1");

            ptrWaypoint waypoint = new Waypoint(Point((std::stod(asso_x),std::stod(asso_y))),
                                                std::stod(asso_a),std::stod(asso_b));
            waypoint->SetId(std::stod(asso_id));
            waypoint->SetCaption(asso_caption);

            landmark->AddAssociation(new Association(landmark,waypoint));


        }



    }

}

