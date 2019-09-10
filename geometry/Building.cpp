/**
 * \file        Building.cpp
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


#include "Building.h"
#include "../geometry/SubRoom.h"
#include "../geometry/Room.h"
#include "../tinyxml/tinyxml.h"

#ifdef _SIMULATOR
#include "../pedestrian/Pedestrian.h"
#include "../mpi/LCGrid.h"
#include "../routing/SafestPathRouter.h"
#include "../routing/RoutingEngine.h"
#include "../pedestrian/PedDistributor.h"
#endif

//#undef _OPENMP

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num()    0
#define omp_get_max_threads()   1
#endif

using namespace std;


Building::Building()
{
     _caption = "no_caption";
     _projectFilename = "";
     _geometryFilename= "";
     _routingEngine = nullptr;
     _linkedCellGrid = nullptr;
     _savePathway = false;
}

#ifdef _SIMULATOR
Building::Building(const std::string& filename, const std::string& rootDir, RoutingEngine& engine, PedDistributor& distributor, double linkedCellSize)
:_projectFilename(filename), _projectRootDir(rootDir), _routingEngine(&engine)
{
     _caption = "no_caption";
     _savePathway = false;
     _linkedCellGrid = nullptr;

     //todo: what happens if any of these  methods failed (return false)? throw exception ?
     this->LoadGeometry();
     this->LoadRoutingInfo(filename);
     //this->AddSurroundingRoom();
     this->InitGeometry();
     this->LoadTrafficInfo();
     distributor.Distribute(this);
     this->InitGrid(linkedCellSize);
     _routingEngine->Init(this);
     this->SanityCheck();
}
#endif

Building::~Building()
{
     //
     // for (int i = 0; i < GetNumberOfRooms(); i++)
     //    delete _rooms[i];

#ifdef _SIMULATOR
     for(unsigned int p=0;p<_allPedestians.size();p++)
     {
          //delete _allPedestians[p];
     }
     _allPedestians.clear();
     delete _linkedCellGrid;
#endif

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
void Building::SetCaption(const std::string& s)
{
     _caption = s;
}

void Building::SetRoutingEngine(RoutingEngine* r)
{
     _routingEngine = r;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

string Building::GetCaption() const
{
     return _caption;
}

RoutingEngine* Building::GetRoutingEngine() const
{
     return _routingEngine;
}

int Building::GetNumberOfRooms() const
{
     return _rooms.size();
}

int Building::GetNumberOfGoals() const
{
     return _transitions.size() + _hLines.size() + _crossings.size();
}

const std::map<int, std::unique_ptr<Room> >& Building::GetAllRooms() const
{
     return _rooms;
}

Room* Building::GetRoom(int index) const
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


LCGrid* Building::GetGrid() const
{
     return _linkedCellGrid;
}

void Building::AddRoom(Room* room)
{
     _rooms[room->GetID()]=std::unique_ptr<Room>(room);
}

void Building::AddSurroundingRoom()
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
                    double x1 = wall.GetPoint1().GetX();
                    double y1 = wall.GetPoint1().GetY();
                    double x2 = wall.GetPoint2().GetX();
                    double y2 = wall.GetPoint2().GetY();

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
                         double x1 = wall.GetPoint1().GetX();
                         double y1 = wall.GetPoint1().GetY();
                         double x2 = wall.GetPoint2().GetX();
                         double y2 = wall.GetPoint2().GetY();

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
               double x1 = wall.GetPoint1().GetX();
               double y1 = wall.GetPoint1().GetY();
               double x2 = wall.GetPoint2().GetX();
               double y2 = wall.GetPoint2().GetY();

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


bool Building::InitGeometry()
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


const string& Building::GetProjectFilename() const
{
     return _projectFilename;
}

void Building::SetProjectFilename(const std::string &filename)
{
     _projectFilename=filename;
}

void Building::SetProjectRootDir(const std::string &filename)
{
     _projectRootDir= filename;
}

const string& Building::GetProjectRootDir() const
{
     return _projectRootDir;
}
const std::string& Building::GetGeometryFilename() const
{
     return _geometryFilename;
}

bool Building::LoadGeometry(const std::string &geometryfile)
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


void Building::WriteToErrorLog() const
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

Room* Building::GetRoom(string caption) const
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

void Building::AddCrossing(Crossing* line)
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

void Building::AddTransition(Transition* line)
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

void Building::AddHline(Hline* line)
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

void Building::AddGoal(Goal* goal)
{
     if (_goals.count(goal->GetId()) != 0) {
          Log->Write(
                    "ERROR: Duplicate index for goal found [%d] in Routing::AddGoal()",
                    goal->GetId());
          exit(EXIT_FAILURE);
     }
     _goals[goal->GetId()] = goal;
}

const map<int, Crossing*>& Building::GetAllCrossings() const
{
     return _crossings;
}

const map<int, Transition*>& Building::GetAllTransitions() const
{
     return _transitions;
}

const map<int, Hline*>& Building::GetAllHlines() const
{
     return _hLines;
}

const map<int, Goal*>& Building::GetAllGoals() const
{
     return _goals;
}

Transition* Building::GetTransition(string caption) const
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

Transition* Building::GetTransition(int ID)
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

Goal* Building::GetFinalGoal(int ID)
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

Crossing* Building::GetTransOrCrossByName(string caption) const
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

Hline* Building::GetTransOrCrossByUID(int id) const
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

SubRoom* Building::GetSubRoomByUID( int uid)
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

//bool Building::IsVisible(Line* l1, Line* l2, bool considerHlines)
//{
//
//     for(auto&& itr_room: _rooms)
//     {
//          for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
//          {
//               if(itr_subroom.second->IsVisible(l1,l2,considerHlines)==false) return false;
//          }
//     }
//     return true;
//}

bool Building::IsVisible(const Point& p1, const Point& p2, const std::vector<SubRoom*>& subrooms, bool considerHlines)
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

bool Building::SanityCheck()
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


#ifdef _SIMULATOR


void Building::UpdateGrid()
{
     _linkedCellGrid->Update(_allPedestians);
}

void Building::InitGrid(double cellSize)
{
     // first look for the geometry boundaries
     double x_min = FLT_MAX;
     double x_max = FLT_MIN;
     double y_min = FLT_MAX;
     double y_max = FLT_MIN;

     //finding the bounding of the grid
     // and collect the pedestrians
     for(auto&& itr_room: _rooms)
     {
          for(auto&& itr_subroom: itr_room.second->GetAllSubRooms())
          {
               for (auto&& wall:itr_subroom.second->GetAllWalls())
               {
                    double x1 = wall.GetPoint1().GetX();
                    double y1 = wall.GetPoint1().GetY();
                    double x2 = wall.GetPoint2().GetX();
                    double y2 = wall.GetPoint2().GetY();

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

     //make the grid slightly larger.
     x_min = x_min - 1*cellSize;
     x_max = x_max + 1*cellSize;
     y_min = y_min - 1*cellSize;
     y_max = y_max + 1*cellSize;

     double boundaries[4] = { x_min, x_max, y_min, y_max };

     //no algorithms
     // the domain is made of a single cell
     if(cellSize==-1) {
          Log->Write("INFO: \tBrute Force will be used for neighborhoods query");
          if ( (x_max-x_min) < (y_max-y_min) ) {
               cellSize=(y_max-y_min);
          } else {
               cellSize=(x_max-x_min);
          }

     } else {
          Log->Write("INFO: \tInitializing the grid with cell size: %f ", cellSize);
     }

     //_linkedCellGrid = new LCGrid(boundaries, cellSize, _allPedestians.size());
     _linkedCellGrid = new LCGrid(boundaries, cellSize, Pedestrian::GetAgentsCreated());
     _linkedCellGrid->ShallowCopy(_allPedestians);

     Log->Write("INFO: \tDone with Initializing the grid ");
}

bool Building::LoadRoutingInfo(const string &filename)
{
     Log->Write("INFO:\tLoading extra routing information");
     if (filename == "") {
          Log->Write("INFO:\t No file supplied !");
          Log->Write("INFO:\t done with loading extra routing information");
          return true;
     }
     TiXmlDocument docRouting(filename);
     if (!docRouting.LoadFile()) {
          Log->Write("ERROR: \t%s", docRouting.ErrorDesc());
          Log->Write("ERROR: \t could not parse the routing file");
          return false;
     }

     TiXmlElement* xRootNode = docRouting.RootElement();
     if( ! xRootNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          return false;
     }

     if (! xRootNode->FirstChild("routing"))
     {
          return true; // no extra routing information
     }
     //load goals and routes
     TiXmlNode*  xGoalsNode = xRootNode->FirstChild("routing")->FirstChild("goals");


     if(xGoalsNode)
          for(TiXmlElement* e = xGoalsNode->FirstChildElement("goal"); e;
                    e = e->NextSiblingElement("goal")) {

               int id = xmltof(e->Attribute("id"), -1);
               int isFinal= string(e->Attribute("final"))=="true"?true:false;
               string caption = xmltoa(e->Attribute("caption"),"-1");

               Goal* goal = new Goal();
               goal->SetId(id);
               goal->SetCaption(caption);
               goal->SetIsFinalGoal(isFinal);

               //looking for polygons (walls)
               for(TiXmlElement* xPolyVertices = e->FirstChildElement("polygon"); xPolyVertices;
                         xPolyVertices = xPolyVertices->NextSiblingElement("polygon")) {

                    for (TiXmlElement* xVertex = xPolyVertices->FirstChildElement(
                              "vertex");
                              xVertex && xVertex != xPolyVertices->LastChild("vertex");
                              xVertex = xVertex->NextSiblingElement("vertex")) {

                         double x1 = xmltof(xVertex->Attribute("px"));
                         double y1 = xmltof(xVertex->Attribute("py"));
                         double x2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("px"));
                         double y2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("py"));
                         goal->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
                    }
               }

               if(!goal->ConvertLineToPoly())
                    return false;

               AddGoal(goal);
               _routingEngine->AddFinalDestinationID(goal->GetId());
          }

     //load routes
     TiXmlNode*  xTripsNode = xRootNode->FirstChild("routing")->FirstChild("routes");

     if(xTripsNode)
          for(TiXmlElement* trip = xTripsNode->FirstChildElement("route"); trip;
                    trip = trip->NextSiblingElement("route")) {

               double id = xmltof(trip->Attribute("id"), -1);
               if (id == -1) {
                    Log->Write("ERROR:\t id missing for trip");
                    return false;
               }
               string sTrip = trip->FirstChild()->ValueStr();
               vector<string> vTrip;
               vTrip.clear();

               char* str = (char*) sTrip.c_str();
               char *p = strtok(str, ":");
               while (p) {
                    vTrip.push_back(xmltoa(p));
                    p = strtok(NULL, ":");
               }
               _routingEngine->AddTrip(vTrip);
          }
     Log->Write("INFO:\tdone with loading extra routing information");
     return true;
}

bool Building::LoadTrafficInfo()
{

     Log->Write("INFO:\tLoading  the traffic info file");

     string trafficFile="";
     TiXmlDocument doc(_projectFilename);
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \t could not parse the project file");
          return false;
     }

     TiXmlNode* xRootNode = doc.RootElement()->FirstChild("traffic_constraints");
     if( ! xRootNode ) {
          Log->Write("WARNING:\tcould not find any traffic information");
          return true;
     }

     //processing the rooms node
     TiXmlNode*  xRoomsNode = xRootNode->FirstChild("rooms");
     if(xRoomsNode)
          for(TiXmlElement* xRoom = xRoomsNode->FirstChildElement("room"); xRoom;
                    xRoom = xRoom->NextSiblingElement("room")) {

               double id = xmltof(xRoom->Attribute("room_id"), -1);
               string state = xmltoa(xRoom->Attribute("state"), "good");
               RoomState status = (state == "good") ? ROOM_CLEAN : ROOM_SMOKED;
               GetRoom(id)->SetState(status);
          }

     //processing the doors node
     TiXmlNode*  xDoorsNode = xRootNode->FirstChild("doors");
     if(xDoorsNode)
          for(TiXmlElement* xDoor = xDoorsNode->FirstChildElement("door"); xDoor;
                    xDoor = xDoor->NextSiblingElement("door")) {

               int id = xmltoi(xDoor->Attribute("trans_id"), -1);
               string state = xmltoa(xDoor->Attribute("state"), "open");

               //store transition in a map and call getTransition/getCrossin
               if (state == "open") {
                    GetTransition(id)->Open();
               } else if (state == "close") {
                    GetTransition(id)->Close();
               } else {
                    Log->Write("WARNING:\t Unknown door state: %s", state.c_str());
               }
          }
     Log->Write("INFO:\tDone with loading traffic info file");
     return true;
}


void Building::DeletePedestrian(Pedestrian* &ped)
{
     vector<Pedestrian*>::iterator it;
     it = find(_allPedestians.begin(), _allPedestians.end(), ped);
     if (it == _allPedestians.end()) {
          Log->Write ("\tERROR: \tPed not found with ID %d ",ped->GetID());
          exit(EXIT_FAILURE);
          return;
     } else {
          // save the path history for this pedestrian before removing from the simulation
          if (_savePathway) {
               string results;
               string path = (*it)->GetPath();
               vector<string> brokenpaths;
               StringExplode(path, ">", &brokenpaths);
               for (unsigned int i = 0; i < brokenpaths.size(); i++) {
                    vector<string> tags;
                    StringExplode(brokenpaths[i], ":", &tags);
                    string room = _rooms[atoi(tags[0].c_str())]->GetCaption();
                    string trans =GetTransition(atoi(tags[1].c_str()))->GetCaption();
                    //ignore crossings/hlines
                    if (trans != "")
                         _pathWayStream << room << " " << trans << endl;
               }

          }
          //cout << "rescued agent: " << (*it)->GetID()<<endl;

          static int totalPeds= _allPedestians.size();
          _allPedestians.erase(it);

          int nowPeds= _allPedestians.size();
          Log->ProgressBar(totalPeds, totalPeds-nowPeds);
     }
     //update the stats before deleting
     Transition* trans =GetTransitionByUID(ped->GetExitIndex());
     if(trans)
     {
          trans->IncreaseDoorUsage(1, ped->GetGlobalTime());
     }
     delete ped;
}

const vector<Pedestrian*>& Building::GetAllPedestrians() const
{
     return _allPedestians;
}

void Building::AddPedestrian(Pedestrian* ped)
{
     for(unsigned int p = 0;p<_allPedestians.size();p++){
          Pedestrian* ped1=_allPedestians[p];
          if(ped->GetID()==ped1->GetID()){
               cout<<"Pedestrian already in the room ??? "<<ped->GetID()<<endl;
               return;
          }
     }
     _allPedestians.push_back(ped);
}

void Building::GetPedestrians(int room, int subroom, std::vector<Pedestrian*>& peds) const
{
     //for(unsigned int p = 0;p<_allPedestians.size();p++){
     //     Pedestrian* ped=_allPedestians[p];

     for (auto&& ped : _allPedestians)
     {
          if ((room == ped->GetRoomID()) && (subroom == ped->GetSubRoomID()))
          {
               peds.push_back(ped);
          }
     }
}

//obsolete
void Building::InitSavePedPathway(const string &filename)
{
     _pathWayStream.open(filename.c_str());
     _savePathway = true;

     if (_pathWayStream.is_open()) {
          Log->Write("#INFO:\tsaving pedestrian paths to [ " + filename + " ]");
          _pathWayStream << "##pedestrian ways" << endl;
          _pathWayStream << "#nomenclature roomid  caption" << endl;
          //              for (unsigned int r=0;r< pRooms.size();r++){
          //                      Room* room= GetRoom(r);
          //                      const vector<int>& goals=room->GetAllTransitionsIDs();
          //
          //                      for(unsigned int g=0;g<goals.size();g++){
          //                              int exitid=goals[g];
          //                              string exit_caption=pRouting->GetGoal(exitid)->GetCaption();
          //                              PpathWayStream<<exitid<<" "<<exit_caption<<endl;
          //                      }
          //              }
          //
          _pathWayStream << "#data room exit_id" << endl;
     } else {
          Log->Write("#INFO:\t Unable to open [ " + filename + " ]");
          Log->Write("#INFO:\t saving to stdout");

     }
}


void Building::StringExplode(string str, string separator,
          vector<string>* results)
{
     size_t found;
     found = str.find_first_of(separator);
     while (found != string::npos) {
          if (found > 0) {
               results->push_back(str.substr(0, found));
          }
          str = str.substr(found + 1);
          found = str.find_first_of(separator);
     }
     if (str.length() > 0) {
          results->push_back(str);
     }
}

Pedestrian* Building::GetPedestrian(int pedID) const
{
     for(unsigned int p=0;p<_allPedestians.size();p++)
     {
          Pedestrian* ped = _allPedestians[p];
          if (ped->GetID() == pedID) {
               return ped;
          }
     }

     return NULL;
}

Transition* Building::GetTransitionByUID(int uid) const
{
     //eventually
     map<int, Transition*>::const_iterator itr;
     for(itr = _transitions.begin(); itr != _transitions.end(); ++itr) {
          if (itr->second->GetUniqueID()== uid)
               return itr->second;
     }
     return NULL;
}


bool Building::SaveGeometry(const std::string &filename)
{
     std::stringstream geometry;

     //write the header
     geometry<< "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"<<endl;
     geometry<< "<geometry version=\"0.5\" caption=\"second life\" unit=\"m\"\n "
               " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n  "
               " xsi:noNamespaceSchemaLocation=\"http://134.94.2.137/jps_geoemtry.xsd\">"<<endl<<endl;

     //write the rooms
     geometry<<"<rooms>"<<endl;
     for (auto&& itroom : _rooms)
     {
          auto&& room=itroom.second;
          geometry<<"\t<room id =\""<<room->GetID()<<"\" caption =\""<<room->GetCaption()<<"\">"<<endl;
          for(auto&& itr_sub : room->GetAllSubRooms())
          {
               auto&& sub=itr_sub.second;
               const double* plane=sub->GetPlaneEquation();
               geometry<<"\t\t<subroom id =\""<<sub->GetSubRoomID()
                                  <<"\" closed=\""<<0
                                  <<"\" class=\""<<sub->GetType()
                                  <<"\" A_x=\""<<plane[0]
                                                       <<"\" B_y=\""<<plane[1]
                                                                            <<"\" C_z=\""<<plane[2]<<"\">"<<endl;


               for (auto&& wall : sub->GetAllWalls())
               {
                    const Point& p1=wall.GetPoint1();
                    const Point& p2=wall.GetPoint2();

                    geometry<<"\t\t\t<polygon caption=\"wall\" type=\""<<wall.GetType()<<"\">"<<endl
                              <<"\t\t\t\t<vertex px=\""<<p1._x<<"\" py=\""<<p1._y<<"\"/>"<<endl
                              <<"\t\t\t\t<vertex px=\""<<p2._x<<"\" py=\""<<p2._y<<"\"/>"<<endl
                              <<"\t\t\t</polygon>"<<endl;
               }

               if(sub->GetType()=="stair")
               {
                    const Point& up = ((Stair*)sub.get())->GetUp();
                    const Point& down = ((Stair*)sub.get())->GetDown();
                    geometry<<"\t\t\t<up px=\""<<up._x<<"\" py=\""<<up._y<<"\"/>"<<endl;
                    geometry<<"\t\t\t<down px=\""<<down._x<<"\" py=\""<<down._y<<"\"/>"<<endl;
               }

               geometry<<"\t\t</subroom>"<<endl;
          }

          //write the crossings
          geometry<<"\t\t<crossings>"<<endl;
          for (auto const& mapcross : _crossings)
          {
               Crossing* cross=mapcross.second;

               //only write the crossings in this rooms
               if(cross->GetRoom1()->GetID()!=room->GetID()) continue;

               const Point& p1=cross->GetPoint1();
               const Point& p2=cross->GetPoint2();

               geometry<<"\t<crossing id =\""<<cross->GetID()
                                  <<"\" subroom1_id=\""<<cross->GetSubRoom1()->GetSubRoomID()
                                  <<"\" subroom2_id=\""<<cross->GetSubRoom2()->GetSubRoomID()<<"\">"<<endl;

               geometry<<"\t\t<vertex px=\""<<p1._x<<"\" py=\""<<p1._y<<"\"/>"<<endl
                         <<"\t\t<vertex px=\""<<p2._x<<"\" py=\""<<p2._y<<"\"/>"<<endl
                         <<"\t</crossing>"<<endl;
          }
          geometry<<"\t\t</crossings>"<<endl;
          geometry<<"\t</room>"<<endl;
     }

     geometry<<"</rooms>"<<endl;

     //write the transitions
     geometry<<"<transitions>"<<endl;

     for (auto const& maptrans : _transitions)
     {
          Transition* trans=maptrans.second;
          const Point& p1=trans->GetPoint1();
          const Point& p2=trans->GetPoint2();
          int room2_id=-1;
          int subroom2_id=-1;
          if(trans->GetRoom2())
          {
               room2_id=trans->GetRoom2()->GetID();
               subroom2_id=trans->GetSubRoom2()->GetSubRoomID();
          }

          geometry<<"\t<transition id =\""<<trans->GetID()
                             <<"\" caption=\""<<trans->GetCaption()
                             <<"\" type=\""<<trans->GetType()
                             <<"\" room1_id=\""<<trans->GetRoom1()->GetID()
                             <<"\" subroom1_id=\""<<trans->GetSubRoom1()->GetSubRoomID()
                             <<"\" room2_id=\""<<room2_id
                             <<"\" subroom2_id=\""<<subroom2_id<<"\">"<<endl;

          geometry<<"\t\t<vertex px=\""<<p1._x<<"\" py=\""<<p1._y<<"\"/>"<<endl
                    <<"\t\t<vertex px=\""<<p2._x<<"\" py=\""<<p2._y<<"\"/>"<<endl
                    <<"\t</transition>"<<endl;

     }

     geometry<<"</transitions>"<<endl;
     geometry<<"</geometry>"<<endl;
     //write the routing file

     //cout<<endl<<geometry.str()<<endl;

     ofstream geofile (filename);
     if(geofile.is_open())
     {
          geofile<<geometry.str();
          Log->Write("INFO:\tfile saved to %s",filename.c_str());
     }
     else
     {
          Log->Write("ERROR:\tunable to save the geometry to %s",filename.c_str());
          return false;
     }

     return true;
}

#endif // _SIMULATOR
