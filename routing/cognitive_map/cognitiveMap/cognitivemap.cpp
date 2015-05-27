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
              xLandmark = xLandmark->NextSiblingElement("landmark")) {

         ptrLandmark = new Landmark();
         //make_unique<Song>

         std::string id = xmltoa(xLandmark->Attribute("id"), "-1");
         //room->SetID(xmltoi(room_id.c_str(), -1));

         //std::string caption = "room " + room_id;
         Landmark->SetCaption(xmltoa(xRoom->Attribute("caption")); //, caption.c_str()));

         //double position = xmltof(xRoom->Attribute("zpos"), 0.0);

         //if(position>6.0) position+=50;
         //room->SetZPos(position);

         //parsing the subrooms
         //processing the rooms node
         //TiXmlNode*  xSubroomsNode = xRoom->FirstChild("subroom");

         for(TiXmlElement* xSubRoom = xRoom->FirstChildElement("subroom"); xSubRoom;
                   xSubRoom = xSubRoom->NextSiblingElement("subroom")) {


              string subroom_id = xmltoa(xSubRoom->Attribute("id"), "-1");
              string SubroomClosed = xmltoa(xSubRoom->Attribute("closed"), "0");
              string type = xmltoa(xSubRoom->Attribute("class"),"subroom");

              //get the equation of the plane if any
              double A_x = xmltof(xSubRoom->Attribute("A_x"), 0.0);
              double B_y = xmltof(xSubRoom->Attribute("B_y"), 0.0);

              // assume either the old "C_z" or the new "C"
              double C_z = xmltof(xSubRoom->Attribute("C_z"), 0.0);
              C_z = xmltof(xSubRoom->Attribute("C"), C_z);

              SubRoom* subroom = NULL;

}

