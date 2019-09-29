/**
 * \file        BuildingOld.cpp
 * \date        Oct 1, 2014
 * \version     v0.8
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
 *
 **/


#include "BuildingOld.h"
#include <geometry/SubRoom.h>
#include <geometry/Room.h>
#include <tinyxml.h>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num()    0
#define omp_get_max_threads()   1
#endif

using namespace std;


BuildingOld::BuildingOld()
{
     _caption = "no_caption";
     _projectFilename = "";
     _geometryFilename= "";
     _routingEngine = nullptr;
     _linkedCellGrid = nullptr;
     _savePathway = false;
}

BuildingOld::~BuildingOld()
{
     //
     // for (int i = 0; i < GetNumberOfRooms(); i++)
     //    delete _rooms[i];

     if (_pathWayStream.is_open())
          _pathWayStream.close();


     for (map<int, Crossing*>::const_iterator iter = _crossings.begin();
               iter != _crossings.end(); ++iter) {
          delete iter->second;
     }
     for (map<int, Transition*>::const_iterator iter = _transitions.begin();
               iter != _transitions.end(); ++iter) {
          delete iter->second;
     }
     for (map<int, Hline*>::const_iterator iter = _hLines.begin();
               iter != _hLines.end(); ++iter) {
          delete iter->second;
     }
     for (map<int, Goal*>::const_iterator iter = _goals.begin();
               iter != _goals.end(); ++iter) {
          delete iter->second;
     }
}

/************************************************************
 Setter-Funktionen
 ************************************************************/
void BuildingOld::SetCaption(const std::string& s)
{
     _caption = s;
}

void BuildingOld::SetRoutingEngine(RoutingEngine* r)
{
     _routingEngine = r;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

string BuildingOld::GetCaption() const
{
     return _caption;
}

RoutingEngine* BuildingOld::GetRoutingEngine() const
{
     return _routingEngine;
}

int BuildingOld::GetNumberOfRooms() const
{
     return _rooms.size();
}

int BuildingOld::GetNumberOfGoals() const
{
     return _transitions.size() + _hLines.size() + _crossings.size();
}

const std::map<int, std::unique_ptr<Room> >& BuildingOld::GetAllRooms() const
{
     return _rooms;
}

Room* BuildingOld::GetRoom(int index) const
{
     //todo: obsolete since the check is done by .at()
     if(_rooms.count(index)==0)
     {
          Log->Write("ERROR: Wrong 'index' in CBuiling::GetRoom() Room ID: %d size: %d",index, _rooms.size());
          Log->Write("\tControl your rooms ID and make sure they are in the order 0, 1, 2,.. ");
          return nullptr;
     }
     //return _rooms[index];
     return _rooms.at(index).get();
}


LCGrid* BuildingOld::GetGrid() const
{
     return _linkedCellGrid;
}

void BuildingOld::AddRoom(Room* room)
{
     _rooms[room->GetID()]=std::unique_ptr<Room>(room);
}

void BuildingOld::AddSurroundingRoom()
{
     Log->Write("INFO: \tAdding the room 'outside' ");
     // first look for the geometry boundaries
     double x_min = FLT_MAX;
     double x_max = -FLT_MAX;
     double y_min = FLT_MAX;
     double y_max = -FLT_MAX;
     //finding the bounding of the grid
     // and collect the pedestrians

     for(auto&& itr_room: _rooms)
     {
          for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
          {
               for (auto&& wall:itr_subroom.second->GetAllWalls())
               {
                    double x1 = wall.GetPoint1()._x;
                    double y1 = wall.GetPoint1()._y;
                    double x2 = wall.GetPoint2()._x;
                    double y2 = wall.GetPoint2()._y;

                    double xmax = (x1 > x2) ? x1 : x2;
                    double xmin = (x1 > x2) ? x2 : x1;
                    double ymax = (y1 > y2) ? y1 : y2;
                    double ymin = (y1 > y2) ? y2 : y1;

                    x_min = (xmin <= x_min) ? xmin : x_min;
                    x_max = (xmax >= x_max) ? xmax : x_max;
                    y_max = (ymax >= y_max) ? ymax : y_max;
                    y_min = (ymin <= y_min) ? ymin : y_min;
               }
               for(auto&& obs:itr_subroom.second->GetAllObstacles())
               {
                    for(auto&& wall: obs->GetAllWalls())
                    {
                         double x1 = wall.GetPoint1()._x;
                         double y1 = wall.GetPoint1()._y;
                         double x2 = wall.GetPoint2()._x;
                         double y2 = wall.GetPoint2()._y;

                         double xmax = (x1 > x2) ? x1 : x2;
                         double xmin = (x1 > x2) ? x2 : x1;
                         double ymax = (y1 > y2) ? y1 : y2;
                         double ymin = (y1 > y2) ? y2 : y1;

                         x_min = (xmin <= x_min) ? xmin : x_min;
                         x_max = (xmax >= x_max) ? xmax : x_max;
                         y_max = (ymax >= y_max) ? ymax : y_max;
                         y_min = (ymin <= y_min) ? ymin : y_min;
                    }
               }
          }
     }

     for(auto&& itr_goal:_goals)
     {
          for(auto&& wall: itr_goal.second->GetAllWalls())
          {
               double x1 = wall.GetPoint1()._x;
               double y1 = wall.GetPoint1()._y;
               double x2 = wall.GetPoint2()._x;
               double y2 = wall.GetPoint2()._y;

               double xmax = (x1 > x2) ? x1 : x2;
               double xmin = (x1 > x2) ? x2 : x1;
               double ymax = (y1 > y2) ? y1 : y2;
               double ymin = (y1 > y2) ? y2 : y1;

               x_min = (xmin <= x_min) ? xmin : x_min;
               x_max = (xmax >= x_max) ? xmax : x_max;
               y_max = (ymax >= y_max) ? ymax : y_max;
               y_min = (ymin <= y_min) ? ymin : y_min;
          }
     }
     //make the grid slightly larger.
     x_min = x_min - 10.0;
     x_max = x_max + 10.0;
     y_min = y_min - 10.0;
     y_max = y_max + 10.0;
     Log->Write("INFO: \tAdding surrounding room X: %f, Y: %f -- %f,  %f\n", x_min, x_max, y_min, y_max);
     SubRoom* bigSubroom = new NormalSubRoom();
     bigSubroom->SetType("Subroom");
     bigSubroom->SetPlanEquation(0,0,1); //@todo: dummy values

     bigSubroom->SetRoomID(_rooms.size());
     bigSubroom->SetSubRoomID(0); // should be the single subroom
     bigSubroom->AddWall(Wall(Point(x_min, y_min), Point(x_min, y_max)));
     bigSubroom->AddWall(Wall(Point(x_min, y_max), Point(x_max, y_max)));
     bigSubroom->AddWall(Wall(Point(x_max, y_max), Point(x_max, y_min)));
     bigSubroom->AddWall(Wall(Point(x_max, y_min), Point(x_min, y_min)));
     vector<Line*> goals = vector<Line*>(); // dummy vector
     bigSubroom->ConvertLineToPoly(goals);

     Room * bigRoom = new Room();
     bigRoom->AddSubRoom(bigSubroom);
     bigRoom->SetCaption("outside");
     bigRoom->SetID(_rooms.size());
     bigRoom->SetZPos(0); //@todo: dummy value
     AddRoom(bigRoom);
     _xMin = x_min;
     _xMax = x_max;
     _yMin = y_min;
     _yMax = y_max;
}


bool BuildingOld::InitGeometry()
{
     Log->Write("INFO: \tInit Geometry");

     for(auto&& itr_room: _rooms)
     {
          for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
          {
               //create a close polyline out of everything
               vector<Line*> goals = vector<Line*>();

               //  collect all crossings
               for(auto&& cros:itr_subroom.second->GetAllCrossings())
               {
                    goals.push_back(cros);
               }
               //collect all transitions
               for(auto&& trans:itr_subroom.second->GetAllTransitions())
               {
                    goals.push_back(trans);
               }
               // initialize the poly
               if(! itr_subroom.second->ConvertLineToPoly(goals))
                    return false;
               itr_subroom.second->CalculateArea();

               //do the same for the obstacles that are closed
               for(auto&& obst:itr_subroom.second->GetAllObstacles())
               {
                    //if (obst->GetClosed() == 1)
                    if(!obst->ConvertLineToPoly())
                         return false;
               }
          }
     }

     // look and save the neighbor subroom for improving the runtime
     // that information is already present in the crossing/transitions

     for(const auto & cross: _crossings)
     {
          SubRoom* s1=cross.second->GetSubRoom1();
          SubRoom* s2=cross.second->GetSubRoom2();
          if(s1) s1->AddNeighbor(s2);
          if(s2) s2->AddNeighbor(s1);
     }

     for(const auto & trans: _transitions)
     {
          SubRoom* s1=trans.second->GetSubRoom1();
          SubRoom* s2=trans.second->GetSubRoom2();
          if(s1) s1->AddNeighbor(s2);
          if(s2) s2->AddNeighbor(s1);
     }

     Log->Write("INFO: \tInit Geometry successful!!!\n");

     return true;
}


const string& BuildingOld::GetProjectFilename() const
{
     return _projectFilename;
}

void BuildingOld::SetProjectFilename(const std::string &filename)
{
     _projectFilename=filename;
}

void BuildingOld::SetProjectRootDir(const std::string &filename)
{
     _projectRootDir= filename;
}

const string& BuildingOld::GetProjectRootDir() const
{
     return _projectRootDir;
}
const std::string& BuildingOld::GetGeometryFilename() const
{
     return _geometryFilename;
}

bool BuildingOld::LoadGeometry(const std::string &geometryfile)
{
     //get the geometry filename from the project file
     string geoFilenameWithPath= _projectRootDir + geometryfile;

     if(geometryfile=="")
     {
          TiXmlDocument doc(_projectFilename);
          if (!doc.LoadFile()) {
               Log->Write("ERROR: \t%s", doc.ErrorDesc());
               Log->Write("\t could not parse the project file");
               return false;
          }

          Log->Write("INFO: \tParsing the geometry file");
          TiXmlElement* xMainNode = doc.RootElement();

          if(xMainNode->FirstChild("geometry")) {
               _geometryFilename=xMainNode->FirstChild("geometry")->FirstChild()->Value();
               geoFilenameWithPath=_projectRootDir+_geometryFilename;
               Log->Write("INFO: \tgeometry <"+_geometryFilename+">");
          }
     }

     TiXmlDocument docGeo(geoFilenameWithPath);
     if (!docGeo.LoadFile()) {
          Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
          Log->Write("\t could not parse the geometry file");
          return false;
     }

     TiXmlElement* xRootNode = docGeo.RootElement();
     if( ! xRootNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          return false;
     }

     if( xRootNode->ValueStr () != "geometry" ) {
          Log->Write("ERROR:\tRoot element value is not 'geometry'.");
          return false;
     }
     if(xRootNode->Attribute("unit"))
          if(string(xRootNode->Attribute("unit")) != "m") {
               Log->Write("ERROR:\tOnly the unit m (meters) is supported. \n\tYou supplied [%s]",xRootNode->Attribute("unit"));
               return false;
          }

     double version = xmltof(xRootNode->Attribute("version"), -1);

     if (version != std::stod(JPS_VERSION) && version != std::stod(JPS_OLD_VERSION)) {
          Log->Write(" \tWrong geometry version!");
          Log->Write(" \tOnly version >= %s supported",JPS_VERSION);
          Log->Write(" \tPlease update the version of your geometry file to %s",JPS_VERSION);
          return false;
     }

     _caption = xmltoa(xRootNode->Attribute("caption"), "virtual building");
     //The file has two main nodes
     //<rooms> and <transitions>

     //processing the rooms node
     TiXmlNode*  xRoomsNode = xRootNode->FirstChild("rooms");
     if (!xRoomsNode) {
          Log->Write("ERROR: \tThe geometry should have at least one room and one subroom");
          return false;
     }

     for(TiXmlElement* xRoom = xRoomsNode->FirstChildElement("room"); xRoom;
               xRoom = xRoom->NextSiblingElement("room")) {

          Room* room = new Room();
          //make_unique<Song>

          string room_id = xmltoa(xRoom->Attribute("id"), "-1");
          room->SetID(xmltoi(room_id.c_str(), -1));

          string caption = "room " + room_id;
          room->SetCaption(
                    xmltoa(xRoom->Attribute("caption"), caption.c_str()));

          double position = xmltof(xRoom->Attribute("zpos"), 0.0);

          //if(position>6.0) position+=50;
          room->SetZPos(position);

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

               if (type == "stair") {
                    if(xSubRoom->FirstChildElement("up")==NULL) {
                         Log->Write("ERROR:\t the attribute <up> and <down> are missing for the stair");
                         Log->Write("ERROR:\t check your geometry file");
                         return false;
                    }
                    double up_x = xmltof( xSubRoom->FirstChildElement("up")->Attribute("px"), 0.0);
                    double up_y = xmltof( xSubRoom->FirstChildElement("up")->Attribute("py"), 0.0);
                    double down_x = xmltof( xSubRoom->FirstChildElement("down")->Attribute("py"), 0.0);
                    double down_y = xmltof( xSubRoom->FirstChildElement("down")->Attribute("py"), 0.0);
                    subroom = new Stair();
                    ((Stair*)subroom)->SetUp(Point(up_x,up_y));
                    ((Stair*)subroom)->SetDown(Point(down_x,down_y));
               } else {
                    //normal subroom or corridor
                    subroom = new NormalSubRoom();
               }

               subroom->SetType(type);
               subroom->SetPlanEquation(A_x,B_y,C_z);
               subroom->SetRoomID(room->GetID());
               subroom->SetSubRoomID(xmltoi(subroom_id.c_str(), -1));

               //static int p_id=1;
               //cout<<endl<<"wall polygon: "<< p_id++<<endl;
               //looking for polygons (walls)
               for(TiXmlElement* xPolyVertices = xSubRoom->FirstChildElement("polygon"); xPolyVertices;
                         xPolyVertices = xPolyVertices->NextSiblingElement("polygon")) {

                    for (TiXmlElement* xVertex = xPolyVertices->FirstChildElement(
                              "vertex");
                              xVertex && xVertex != xPolyVertices->LastChild("vertex");
                              xVertex = xVertex->NextSiblingElement("vertex")) {

                         double x1 = xmltof(xVertex->Attribute("px"));
                         double y1 = xmltof(xVertex->Attribute("py"));
                         double x2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("px"));
                         double y2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("py"));
                         subroom->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
                         //printf("%0.2f %0.2f %0.2f %0.2f\n",x1,y1,x2,y2);
                    }

               }

               //looking for obstacles
               for(TiXmlElement* xObstacle = xSubRoom->FirstChildElement("obstacle"); xObstacle;
                         xObstacle = xObstacle->NextSiblingElement("obstacle")) {

                    int id = xmltof(xObstacle->Attribute("id"), -1);
                    int height = xmltof(xObstacle->Attribute("height"), 0);
                    //double ObstClosed = xmltof(xObstacle->Attribute("closed"), 0);
                    string ObstCaption = xmltoa(xObstacle->Attribute("caption"),"-1");

                    Obstacle* obstacle = new Obstacle();
                    obstacle->SetId(id);
                    obstacle->SetCaption(ObstCaption);
                    //obstacle->SetClosed(ObstClosed);
                    obstacle->SetHeight(height);

                    //looking for polygons (walls)
                    for(TiXmlElement* xPolyVertices = xObstacle->FirstChildElement("polygon"); xPolyVertices;
                              xPolyVertices = xPolyVertices->NextSiblingElement("polygon")) {

                         for (TiXmlElement* xVertex = xPolyVertices->FirstChildElement(
                                   "vertex");
                                   xVertex && xVertex != xPolyVertices->LastChild("vertex");
                                   xVertex = xVertex->NextSiblingElement("vertex")) {

                              double x1 = xmltof(xVertex->Attribute("px"));
                              double y1 = xmltof(xVertex->Attribute("py"));
                              double x2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("px"));
                              double y2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("py"));
                              obstacle->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
                         }
                    }
                    subroom->AddObstacle(obstacle);
               }
               room->AddSubRoom(subroom);
          }

          //parsing the crossings
          TiXmlNode*  xCrossingsNode = xRoom->FirstChild("crossings");
          if(xCrossingsNode)
               for(TiXmlElement* xCrossing = xCrossingsNode->FirstChildElement("crossing"); xCrossing;
                         xCrossing = xCrossing->NextSiblingElement("crossing")) {

                    int id = xmltoi(xCrossing->Attribute("id"), -1);
                    int sub1_id = xmltoi(xCrossing->Attribute("subroom1_id"), -1);
                    int sub2_id = xmltoi(xCrossing->Attribute("subroom2_id"), -1);

                    double x1 = xmltof(     xCrossing->FirstChildElement("vertex")->Attribute("px"));
                    double y1 = xmltof(     xCrossing->FirstChildElement("vertex")->Attribute("py"));
                    double x2 = xmltof(     xCrossing->LastChild("vertex")->ToElement()->Attribute("px"));
                    double y2 = xmltof(     xCrossing->LastChild("vertex")->ToElement()->Attribute("py"));

                    Crossing* c = new Crossing();
                    c->SetID(id);
                    c->SetPoint1(Point(x1, y1));
                    c->SetPoint2(Point(x2, y2));

                    c->SetSubRoom1(room->GetSubRoom(sub1_id));
                    c->SetSubRoom2(room->GetSubRoom(sub2_id));
                    c->SetRoom1(room);
                    AddCrossing(c);

                    room->GetSubRoom(sub1_id)->AddCrossing(c);
                    room->GetSubRoom(sub2_id)->AddCrossing(c);
               }

          AddRoom(room);
     }
     //exit(0);

     // all rooms are read, now proceed with transitions
     TiXmlNode*  xTransNode = xRootNode->FirstChild("transitions");
     if(xTransNode)
          for(TiXmlElement* xTrans = xTransNode->FirstChildElement("transition"); xTrans;
                    xTrans = xTrans->NextSiblingElement("transition")) {

               int id = xmltoi(xTrans->Attribute("id"), -1);
               // string caption = "door " + id;
               string caption = "door ";
               caption += to_string(id);
               caption = xmltoa(xTrans->Attribute("caption"), caption.c_str());
               int room1_id = xmltoi(xTrans->Attribute("room1_id"), -1);
               int room2_id = xmltoi(xTrans->Attribute("room2_id"), -1);
               int subroom1_id = xmltoi(xTrans->Attribute("subroom1_id"), -1);
               int subroom2_id = xmltoi(xTrans->Attribute("subroom2_id"), -1);
               string type = xmltoa(xTrans->Attribute("type"), "normal");

               double x1 = xmltof(     xTrans->FirstChildElement("vertex")->Attribute("px"));
               double y1 = xmltof(     xTrans->FirstChildElement("vertex")->Attribute("py"));

               double x2 = xmltof(     xTrans->LastChild("vertex")->ToElement()->Attribute("px"));
               double y2 = xmltof(     xTrans->LastChild("vertex")->ToElement()->Attribute("py"));


               Transition* t = new Transition();
               t->SetID(id);
               t->SetCaption(caption);
               t->SetPoint1(Point(x1, y1));
               t->SetPoint2(Point(x2, y2));
               t->SetType(type);

               if (room1_id != -1 && subroom1_id != -1) {
                    //Room* room = _rooms[room1_id];
                    Room* room = GetRoom(room1_id);
                    SubRoom* subroom = room->GetSubRoom(subroom1_id);

                    //subroom->AddGoalID(t->GetUniqueID());
                    //MPI
                    room->AddTransitionID(t->GetUniqueID());
                    t->SetRoom1(room);
                    t->SetSubRoom1(subroom);

                    //new implementation
                    subroom->AddTransition(t);
               }
               if (room2_id != -1 && subroom2_id != -1) {
                    auto&& room = _rooms[room2_id];
                    SubRoom* subroom = room->GetSubRoom(subroom2_id);
                    //subroom->AddGoalID(t->GetUniqueID());
                    //MPI
                    room->AddTransitionID(t->GetUniqueID());
                    t->SetRoom2(room.get());
                    t->SetSubRoom2(subroom);

                    //new implementation
                    subroom->AddTransition(t);
               }

               AddTransition(t);
          }

     Log->Write("INFO: \tLoading building file successful!!!\n");

     //everything went fine
     return true;
}


void BuildingOld::WriteToErrorLog() const
{
     Log->Write("GEOMETRY: ");
     for (int i = 0; i < GetNumberOfRooms(); i++) {
          Room* r = GetRoom(i);
          r->WriteToErrorLog();
     }
     Log->Write("ROUTING: ");

     for (map<int, Crossing*>::const_iterator iter = _crossings.begin();
               iter != _crossings.end(); ++iter) {
          iter->second->WriteToErrorLog();
     }
     for (map<int, Transition*>::const_iterator iter = _transitions.begin();
               iter != _transitions.end(); ++iter) {
          iter->second->WriteToErrorLog();
     }
     for (map<int, Hline*>::const_iterator iter = _hLines.begin();
               iter != _hLines.end(); ++iter) {
          iter->second->WriteToErrorLog();
     }
     Log->Write("\n");
}

Room* BuildingOld::GetRoom(string caption) const
{
     for(const auto& it: _rooms)
     {
          if(it.second->GetCaption()==caption)
               return it.second.get();
     }
     Log->Write("ERROR: Room not found with caption " + caption);
     //return NULL;
     exit(EXIT_FAILURE);
}

void BuildingOld::AddCrossing(Crossing* line)
{
     if (_crossings.count(line->GetID()) != 0) {
          char tmp[CLENGTH];
          sprintf(tmp,
                    "ERROR: Duplicate index for crossing found [%d] in Routing::AddCrossing()",
                    line->GetID());
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
     _crossings[line->GetID()] = line;
}

void BuildingOld::AddTransition(Transition* line)
{
     if (_transitions.count(line->GetID()) != 0) {
          char tmp[CLENGTH];
          sprintf(tmp,
                    "ERROR: Duplicate index for transition found [%d] in Routing::AddTransition()",
                    line->GetID());
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
     _transitions[line->GetID()] = line;
}

void BuildingOld::AddHline(Hline* line)
{
     if (_hLines.count(line->GetID()) != 0) {
          // check if the lines are identical
          Hline* ori= _hLines[line->GetID()];
          if(ori->operator ==(*line)) {
               Log->Write("INFO: \tSkipping identical hlines with ID [%d]",line->GetID());
               return;
          } else {
               Log->Write(
                         "ERROR: Duplicate index for hlines found [%d] in Routing::AddHline(). You have [%d] hlines",
                         line->GetID(), _hLines.size());
               exit(EXIT_FAILURE);
          }
     }
     _hLines[line->GetID()] = line;
}

void BuildingOld::AddGoal(Goal* goal)
{
     if (_goals.count(goal->GetId()) != 0) {
          Log->Write(
                    "ERROR: Duplicate index for goal found [%d] in Routing::AddGoal()",
                    goal->GetId());
          exit(EXIT_FAILURE);
     }
     _goals[goal->GetId()] = goal;
}

const map<int, Crossing*>& BuildingOld::GetAllCrossings() const
{
     return _crossings;
}

const map<int, Transition*>& BuildingOld::GetAllTransitions() const
{
     return _transitions;
}

const map<int, Hline*>& BuildingOld::GetAllHlines() const
{
     return _hLines;
}

const map<int, Goal*>& BuildingOld::GetAllGoals() const
{
     return _goals;
}

Transition* BuildingOld::GetTransition(string caption) const
{
     //eventually
     map<int, Transition*>::const_iterator itr;
     for(itr = _transitions.begin(); itr != _transitions.end(); ++itr) {
          if (itr->second->GetCaption() == caption)
               return itr->second;
     }

     Log->Write("WARNING: No Transition with Caption: " + caption);
     exit(EXIT_FAILURE);
}

Transition* BuildingOld::GetTransition(int ID)
{
     if (_transitions.count(ID) == 1) {
          return _transitions[ID];
     } else {
          if (ID == -1)
               return NULL;
          else {
               Log->Write(
                         "ERROR: I could not find any transition with the 'ID' [%d]. You have defined [%d] transitions",
                         ID, _transitions.size());
               exit(EXIT_FAILURE);
          }
     }
}

Goal* BuildingOld::GetFinalGoal(int ID)
{
     if (_goals.count(ID) == 1) {
          return _goals[ID];
     } else {
          if (ID == -1)
               return NULL;
          else {
               Log->Write(
                         "ERROR: I could not find any goal with the 'ID' [%d]. You have defined [%d] goals",
                         ID, _goals.size());
               exit(EXIT_FAILURE);
          }
     }
}

Crossing* BuildingOld::GetTransOrCrossByName(string caption) const
{
     {
          //eventually
          map<int, Transition*>::const_iterator itr;
          for(itr = _transitions.begin(); itr != _transitions.end(); ++itr)
          {
               if (itr->second->GetCaption() == caption)
                    return itr->second;
          }
     }
     {
          //finally the  crossings
          map<int, Crossing*>::const_iterator itr;
          for(itr = _crossings.begin(); itr != _crossings.end(); ++itr)
          {
               if (itr->second->GetCaption() == caption)
                    return itr->second;
          }
     }

     Log->Write("WARNING: No Transition or Crossing with Caption: " + caption);
     return NULL;
}

Hline* BuildingOld::GetTransOrCrossByUID(int id) const
{
     {
          //eventually transitions
          map<int, Transition*>::const_iterator itr;
          for(itr = _transitions.begin(); itr != _transitions.end(); ++itr)
          {
               if (itr->second->GetUniqueID()== id)
                    return itr->second;
          }
     }
     {
          //then the  crossings
          map<int, Crossing*>::const_iterator itr;
          for(itr = _crossings.begin(); itr != _crossings.end(); ++itr)
          {
               if (itr->second->GetUniqueID() == id)
                    return itr->second;
          }
     }
     {
          //finally the  hlines
          for(auto itr = _hLines.begin(); itr != _hLines.end(); ++itr)
          {
               if (itr->second->GetUniqueID() == id)
                    return itr->second;
          }
     }
     Log->Write("ERROR: No Transition or Crossing or hline with ID %d: " ,id);
     return NULL;
}

SubRoom* BuildingOld::GetSubRoomByUID( int uid)
{
     for(auto&& itr_room: _rooms)
     {
          for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
          {
               if(itr_subroom.second->GetUID()==uid)
                    return itr_subroom.second.get();
          }
     }
     Log->Write("ERROR:\t No subroom exits with the unique id %d",uid);
     return NULL;
}

bool BuildingOld::IsVisible(const Point& p1, const Point& p2, const std::vector<SubRoom*>& subrooms, bool considerHlines)
{
     //loop over all subrooms if none is provided
     if (subrooms.empty())
     {
          for(auto&& itr_room: _rooms)
          {
               for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
               {
                    if(itr_subroom.second->IsVisible(p1,p2,considerHlines)==false) return false;
               }
          }
     }
     else
     {
          for(auto&& sub: subrooms)
          {
               if(sub && sub->IsVisible(p1,p2,considerHlines)==false) return false;
          }
     }

     return true;
}

bool BuildingOld::SanityCheck()
{
     Log->Write("INFO: \tChecking the geometry for artifacts");
     bool status = true;

     for(auto&& itr_room: _rooms)
     {
          for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
          {
               if (!itr_subroom.second->SanityCheck())
                    status = false;
          }
     }

     Log->Write("INFO: \t...Done!!!\n");
     return status;
}
