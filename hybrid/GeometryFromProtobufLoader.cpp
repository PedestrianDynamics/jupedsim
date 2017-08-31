//
// Created by Gregor Lämmel on 30/03/16.
//

#include <hybridsim.pb.h>
#include "GeometryFromProtobufLoader.h"
#include "../geometry/Building.h"
#include "../geometry/SubRoom.h"

GeometryFromProtobufLoader::GeometryFromProtobufLoader(const Configuration* configuration)
        :_configuration(configuration)
{

}

void GeometryFromProtobufLoader::LoadBuilding(Building* building)
{
     //todo: what happens if any of these  methods failed (return false)? throw exception ?
     if (!LoadGeometry(building)) {
          Log->Write("ERROR:\t could not load the geometry!");
          exit(EXIT_FAILURE);
     }
     if (!LoadRoutingInfo(building)) {
          Log->Write("ERROR:\t could not load extra routing information!");
          exit(EXIT_FAILURE);
     }
     if (!LoadTrafficInfo(building)) {
           Log->Write("ERROR:\t could not load extra traffic information!");
           exit(EXIT_FAILURE);
     }

}

bool GeometryFromProtobufLoader::LoadTrafficInfo(Building* building)
{
     Log->Write("WARNING:\traffic_constraints not yet implemented in proto file [gl march '16]");
     return true;
}

bool GeometryFromProtobufLoader::LoadGeometry(Building* building)
{
     const hybridsim::Environment& environment = _configuration->GetScenario()->environment();
     building->SetCaption("remote_building");
     for (const hybridsim::Room& pbRoom : environment.room()) {
          Room* room = new Room();
          room->SetID(pbRoom.id());
          room->SetCaption(pbRoom.caption());
          Log->Write("WARNING:\tzpos not yet implemented in proto file [gl march '16]");
          double position = 0.0; //TODO zpos not yet implemented in proto file [gl march '16]
          room->SetZPos(position);
          for (hybridsim::Subroom pbSubroom : pbRoom.subroom()) {
//            std::string subroom_id = pbSubroom.id();
               std::string type = pbSubroom.class_();

               //get the equation of the plane if any
               Log->Write("WARNING:\tA_x, B_y, C_z not yet implemented in proto file [gl march '16]");
               double A_x = 0.0; //TODO: not yet implemented in proto file [gl march '16]
               double B_y = 0.0; //TODO: not yet implemented in proto file [gl march '16]
               double C_z = 0.0; //TODO: not yet implemented in proto file [gl march '16]

               SubRoom* subroom = nullptr;

               //TODO: not yet implemented in proto file [gl march '16]
               Log->Write(
                       "WARNING:\tstairs, escalators, and idle_escalators not yet implemented in proto file [gl march '16]");
               if (type=="stair" || type=="escalator" || type=="idle_escalator") {
//                if(xSubRoom->FirstChildElement("up")==NULL) {
//                    Log->Write("ERROR:\t the attribute <up> and <down> are missing for the " + type);
//                    Log->Write("ERROR:\t check your geometry file");
//                    return false;
//                }
//                double up_x = xmltof( xSubRoom->FirstChildElement("up")->Attribute("px"), 0.0);
//                double up_y = xmltof( xSubRoom->FirstChildElement("up")->Attribute("py"), 0.0);
//                double down_x = xmltof( xSubRoom->FirstChildElement("down")->Attribute("py"), 0.0);
//                double down_y = xmltof( xSubRoom->FirstChildElement("down")->Attribute("py"), 0.0);
//                subroom = new Stair();
//                ((Stair*)subroom)->SetUp(Point(up_x,up_y));
//                ((Stair*)subroom)->SetDown(Point(down_x,down_y));
                    Log->Write("WARNING:\t(stairs, escalators, and idle_escalators not yet implemented");
                    exit(-1);
               }
               else {
                    //normal subroom or corridor
                    subroom = new NormalSubRoom();

               }
               subroom->SetType(type);
               subroom->SetPlanEquation(A_x, B_y, C_z);
               subroom->SetRoomID(room->GetID());
               subroom->SetSubRoomID(pbSubroom.id());
               for (hybridsim::Polygon polygon : pbSubroom.polygon()) {
                    for (int i = 0; i<polygon.coordinate_size()-1; i++) {

                         double x0 = polygon.coordinate(i).x();
                         double y0 = polygon.coordinate(i).y();
                         double x1 = polygon.coordinate(i+1).x();
                         double y1 = polygon.coordinate(i+1).y();
                         subroom->AddWall(Wall(Point(x0, y0), Point(x1, y1)));
                    }
               }

               //TODO: implement obstacles in proto file [gl march '16]
               Log->Write("WARNING:\tobstacles not yet implemented in proto file [gl march '16]");

               room->AddSubRoom(subroom);
          }

          //TODO: implement crossings in proto file [gl march '16]
          Log->Write("WARNING:\tcrossings not yet implemented in proto file [gl march '16]");

          building->AddRoom(room);
     }
     for (hybridsim::Transition tr : environment.transition()) {
//          building->GetTr

          //FIXME: HACK only add transition if reverse transition not yet exist
          //FIXME: Only works for neighboring rooms with one and only one common door
          //FIXME: This needs to be fixed upstream (SUMO side) [GL Apr '16]
          int room1_id = tr.room1_id();
          int room2_id = tr.room2_id();
          bool found = false;
          for (auto&& existing : building->GetAllTransitions()) {
               if (existing.second->GetRoom2()!=0 && existing.second->GetRoom1()->GetID()==room2_id
                       && existing.second->GetRoom2()->GetID()==room1_id) {
                    found = true;
                    break;
               }
          }
          if (found) {
               continue;
          }

//

          Transition* t = new Transition();
          t->SetID(tr.id());
          t->SetCaption(tr.caption());
          t->SetPoint1(Point(tr.vert1().x(), tr.vert1().y()));
          t->SetPoint2(Point(tr.vert2().x(), tr.vert2().y()));
          t->SetType(tr.type());
          if (tr.room1_id()!=-1 && tr.subroom1_id()!=-1) {
               Room* room = building->GetRoom(tr.room1_id());
               SubRoom* subroom = room->GetSubRoom(tr.subroom1_id());
               room->AddTransitionID(t->GetUniqueID());
               t->SetRoom1(room);
               t->SetSubRoom1(subroom);
               subroom->AddTransition(t);
          }

          if (tr.room2_id()!=-1 && tr.subroom2_id()!=-1) {
               Room* room = building->GetRoom(tr.room2_id());
               SubRoom* subroom = room->GetSubRoom(tr.subroom2_id());
               room->AddTransitionID(t->GetUniqueID());
               t->SetRoom2(room);
               t->SetSubRoom2(subroom);
               subroom->AddTransition(t);
          }
          building->AddTransition(t);
     }

     for (hybridsim::Hline pbHline : environment.hline()) {
          int id = pbHline.id();
          int room_id = pbHline.room_id();
          int subroom_id = pbHline.subroom_id();
          double x1 = pbHline.vert1().x();
          double y1 = pbHline.vert1().y();
          double x2 = pbHline.vert2().x();
          double y2 = pbHline.vert2().y();

          Room* room = building->GetRoom(room_id);
          SubRoom* subroom = room->GetSubRoom(subroom_id);

          Hline* h = new Hline();
          h->SetID(id);
          h->SetPoint1(Point(x1, y1));
          h->SetPoint2(Point(x2, y2));
          h->SetRoom1(room);
          h->SetSubRoom1(subroom);

          if (building->AddHline(h)) {
               subroom->AddHline(h);
               //h is freed in building
          }
          else {
               delete h;
          }
     }
     Log->Write("INFO: \tLoading building file successful!!!\n");
     return true;
}

bool GeometryFromProtobufLoader::LoadRoutingInfo(Building* building)
{
     const hybridsim::Scenario* scenario = _configuration->GetScenario();

     for (hybridsim::Goal pbGoal : scenario->goal()) {
          Goal* goal = new Goal();
          goal->SetId(pbGoal.id());
          goal->SetCaption(pbGoal.caption());
          goal->SetIsFinalGoal(pbGoal.final());

          //TODO: it seems so that goal in JPS may comprise of multiple polygons, if so adapt proto file accordingly [gl march '16]
          Log->Write(
                  "WARNING:\tit seems so that goal in JPS may comprise of multiple polygons, if so adapt proto file accordingly [gl march '16]");
          for (int i = 0; i<pbGoal.p().coordinate_size()-1; i++) {
               double x0 = pbGoal.p().coordinate(i).x();
               double y0 = pbGoal.p().coordinate(i).y();
               double x1 = pbGoal.p().coordinate(i+1).x();
               double y1 = pbGoal.p().coordinate(i+1).y();
               goal->AddWall(Wall(Point(x0, y0), Point(x1, y1)));
          }
          if (!goal->ConvertLineToPoly()) {
               return false;
          }
          building->AddGoal(goal);
          _configuration->GetRoutingEngine()->AddFinalDestinationID(goal->GetId());
     }


     //TODO: implement routes in proto file [gl march '16]
     Log->Write("WARNING:\troutes not yet implemented in proto file [gl march '16]");

     Log->Write("INFO:\tdone with loading extra routing information");
     return true;;
}




