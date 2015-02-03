/**
 * \file        EventManager.cpp
 * \date        Jul 4, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <math.h>
#include <stdio.h>
#include "../pedestrian/Pedestrian.h"
#include "../pedestrian/Knowledge.h"
#include "../mpi/LCGrid.h"
#include "../geometry/Building.h"
#include "../geometry/SubRoom.h"
#include "../geometry/Transition.h"
#include "../geometry/Point.h"
#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"
#include "../IO/IODispatcher.h"
#include "../routing/RoutingEngine.h"
#include "../routing/GlobalRouter.h"
#include "../routing/QuickestPathRouter.h"
#include "../routing/MeshRouter.h"
#include "../routing/DummyRouter.h"
#include "../routing/SafestPathRouter.h"
#include "../routing/CognitiveMapRouter.h"
#include "EventManager.h"

using namespace std;

/*******************
 constructors
 ******************/

EventManager::EventManager(Building *_b)
{
     _event_times = vector<double>();
     _event_types = vector<string>();
     _event_states = vector<string>();
     _event_ids = vector<int>();
     _building = _b;
     _eventCounter = 0;
     _dynamic = false;
     _lastUpdateTime = 0;
     //_deltaT = 0;
     _projectFilename=_building->GetProjectFilename();
     _projectRootDir=_building->GetProjectRootDir();
     _updateFrequency =1 ;//seconds
     _updateRadius =2;//meters
     _file = fopen("../events/events.txt", "r");
     if (!_file) {
          Log->Write("INFO:\tFiles 'events.txt' missing. "
                    "Realtime interaction with the simulation not possible.");
     } else {
          Log->Write("INFO:\tFile 'events.txt' will be monitored for new events.");
          _dynamic = true;
     }

     //save the first graph
     CreateRoutingEngine(_b, true);
}

EventManager::~EventManager()
{
     if(_file)
          fclose(_file);
}

bool EventManager::ReadEventsXml()
{
     Log->Write("INFO: \tLooking for pre-defined events in other files");
     //get the geometry filename from the project file
     TiXmlDocument doc(_projectFilename);
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \t could not parse the project file.");
          return false;
     }

     TiXmlElement* xMainNode = doc.RootElement();
     string eventfile = "";
     if (xMainNode->FirstChild("events_file")) {
          eventfile = _projectRootDir
                    + xMainNode->FirstChild("events_file")->FirstChild()->Value();
          Log->Write("INFO: \tevents <" + eventfile + ">");
     } else {
          Log->Write("INFO: \tNo events found");
          return true;
     }

     Log->Write("INFO: \tParsing the event file");
     TiXmlDocument docEvent(eventfile);
     if (!docEvent.LoadFile()) {
          Log->Write("ERROR: \t%s", docEvent.ErrorDesc());
          Log->Write("ERROR: \t could not parse the event file.");
          return false;
     }

     TiXmlElement* xRootNode = docEvent.RootElement();
     if (!xRootNode) {
          Log->Write("ERROR:\tRoot element does not exist.");
          return false;
     }

     if (xRootNode->ValueStr() != "JPScore") {
          Log->Write("ERROR:\tRoot element value is not 'JPScore'.");
          return false;
     }

     TiXmlNode* xEvents = xRootNode->FirstChild("events");
     if (!xEvents) {
          Log->Write("ERROR:\tNo events found.");
          return false;
     }
     _updateFrequency = xmltoi(xEvents->ToElement()->Attribute("update_time"), 1);
     _updateRadius = xmltoi(xEvents->ToElement()->Attribute("update_radius"), 2);

     string color=xmltoa(xEvents->ToElement()->Attribute("agents_color_by_knowledge"), "false");
     if(color=="true")
          Pedestrian::SetColorMode(BY_KNOWLEDGE);

     //Pedestrian::SetColorMode(BY_SPOTLIGHT);

     for (TiXmlElement* e = xEvents->FirstChildElement("event"); e;
               e = e->NextSiblingElement("event")) {
          _event_times.push_back(atoi(e->Attribute("time")));
          _event_types.push_back(e->Attribute("type"));
          _event_states.push_back(e->Attribute("state"));
          _event_ids.push_back(atoi(e->Attribute("id")));
     }
     Log->Write("INFO: \tEvents were initialized");
     return true;
}


void EventManager::ListEvents()
{
     for (unsigned int i = 0; i < _event_times.size(); i++)
     {
          Log->Write("INFO: \tAfter %.2f sec, %s door %d", _event_times[i],
                    _event_states[i].c_str(), _event_ids[i]);
     }
}

void EventManager::ReadEventsTxt(double time)
{
     rewind(_file);
     char cstring[256];
     int lines = 0;
     do {
          if (fgets(cstring, 30, _file) == NULL) {
               Log->Write("WARNING: \tCould not read the event file");
               return;
          }
          if (cstring[0] != '#') {// skip comments
               lines++;
               if (lines > _eventCounter) {
                    Log->Write("INFO:\tEvent: after %.2f sec: ", time);
                    GetEvent(cstring);
                    _eventCounter++;
               }
          }
     } while (feof(_file) == 0);
}

/***********
 Update
 **********/
bool EventManager::UpdateAgentKnowledge(Building* _b)
{
//#pragma omp parallel
     for(auto&& ped:_b->GetAllPedestrians())
     {
          for (auto&& door: _b->GetAllTransitions())
          {
               if(door.second->DistTo(ped->GetPos())<0.5)//distance to door to register its state
               {
                    //actualize the information about the newly closed door
                    if(door.second->IsOpen()==false)
                    {
                         ped->AddKnownClosedDoor(door.first, Pedestrian::GetGlobalTime());
                         ped->SetNewEventFlag(true);

                    }
               }
          }
     }

     //collect the peds that are allowed to forward the information.
     vector<Pedestrian*> informant;
     for(auto&& ped:_b->GetAllPedestrians())
     {
          if (ped->GetNewEventFlag())
               informant.push_back(ped);
     }


     for(auto&& ped1:informant)
     {
          vector<Pedestrian*> neighbourhood;
          _b->GetGrid()->GetNeighbourhood(ped1,neighbourhood);
          for(auto&& ped2:neighbourhood)
          {
               if( (ped1->GetPos()-ped2->GetPos()).Norm()<_updateRadius)
               {
                    //maybe same room and subroom ?
                    if(_b->IsVisible(ped1->GetPos(),ped2->GetPos()))
                    {
                         MergeKnowledge(ped1, ped2);  //ped1->SetSpotlight(true);
                         ped2->SetNewEventFlag(true);
                    }
               }
          }
     }

     //TODO: what happen when they all have the new event flag ? reset maybe?
     if(informant.size()==_b->GetAllPedestrians().size())
     {

          for(auto&& ped:_b->GetAllPedestrians())
               ped->SetNewEventFlag(false);
     }

     // information speed to fast
//     for(auto&& ped1:_b->GetAllPedestrians())
//     {
//          vector<Pedestrian*> neighbourhood;
//          _b->GetGrid()->GetNeighbourhood(ped1,neighbourhood);
//          for(auto&& ped2:neighbourhood)
//          {
//               if( (ped1->GetPos()-ped2->GetPos()).Norm()<_updateRadius)
//               {
//                    //maybe same room and subroom ?
//                    if(_b->IsVisible(ped1->GetPos(),ped2->GetPos()))
//                    MergeKnowledge(ped1, ped2);  //ped1->SetSpotlight(true);
//               }
//          }
//     }

     //update the routers based on the configurations
     //#pragma omp parallel
     for(auto&& ped:_b->GetAllPedestrians())
     {
          if(UpdateRoute(ped)==false)
          {
               //Clear the memory and attempt to reroute
               //this can happen if all doors are known to be closed
               ped->ClearKnowledge();
               Log->Write("ERROR: \t clearing ped knowledge");
               if(UpdateRoute(ped)==false)
               {
                    Log->Write("ERROR: \t cannot reroute the pedestrian. unknown problem");
                    //return false;
                    exit(EXIT_FAILURE);
               }
          }
     }
     return true;
}

bool EventManager::UpdateRoute(Pedestrian* ped)
{
     //create the key as string.
     //map are sorted by default
     string key= ped->GetKnowledgeAsString();
     //get the router engine corresponding to the actual configuration
     bool status=true;
     if (_eventEngineStorage.count(key)>0)
     {
          RoutingEngine* engine=_eventEngineStorage[key];
          //retrieve the old strategy
          RoutingStrategy strategy=ped->GetRouter()->GetStrategy();
          //retrieve the new router
          Router*rout =engine->GetRouter(strategy);
          //check for validity
          ped->SetRouter(rout);
          //clear all previous routes
          ped->ClearMentalMap();
          //ped->ClearKnowledge();
          //overwrite/update the pedestrian router
          if(!rout) status= false;
     }
     else
     {
          Log->Write("WARNING: \t unknown configuration <%s>", key.c_str());
          Log->Write("WARNING: \t  [%d] router available", _eventEngineStorage.size());
          //Log->Write("       : \t trying to create");
          //CreateRoutingEngine(_building);
          status= false;
     }
     return status;
}

void EventManager::MergeKnowledge(Pedestrian* p1, Pedestrian* p2)
{
     auto const &  old_info1=p1->GetKnownledge();
     auto const &   old_info2=p2->GetKnownledge();
     map<int, Knowledge> merge_info;

     //collect the most recent knowledge
     for (auto&& info1 : old_info1)
     {
          merge_info[info1.first] = info1.second;
     }

     for (auto&& info2:old_info2)
     {
          //update infos according to a newest time
          if(merge_info.count(info2.first)>0)
          {
               if(info2.second.GetTime()>merge_info[info2.first].GetTime())
               {
                    merge_info[info2.first]=info2.second;
               }
          }
          else //the info was not present, just add
          {
               merge_info[info2.first]=info2.second;
          }
     }

     //synchronize the knowledge
     p1->ClearKnowledge();
     p2->ClearKnowledge();
     for (auto&& info : merge_info)
     {
          p1->AddKnownClosedDoor(info.first,info.second.GetTime());
          p2->AddKnownClosedDoor(info.first,info.second.GetTime());
     }
}

void EventManager::ProcessEvent()
{
     if (_event_times.size() == 0) return;

     int current_time = Pedestrian::GetGlobalTime();

     if ( (current_time != _lastUpdateTime) &&
               ((current_time % _updateFrequency) == 0))
     {
          //update knowledge about closed doors
          //share the information between the pedestrians
          UpdateAgentKnowledge(_building);
          //cout<<"updating buildling..."<<endl;
          //actualize based on the new knowledge
          _lastUpdateTime = current_time;
          //cout<<"updating..."<<currentTime<<endl<<endl;
     }

     //update the building state
     // the time is needed as double
     double current_time_d = Pedestrian::GetGlobalTime();
     for (unsigned int i = 0; i < _event_times.size(); i++)
     {
          if (fabs(_event_times[i] - current_time_d) < J_EPS_EVENT) {
               //Event with current time stamp detected
               Log->Write("INFO:\tEvent: after %.2f sec: ", current_time_d);
               if (_event_states[i].compare("close") == 0) {
                    CloseDoor(_event_ids[i]);
               } else {
                    OpenDoor(_event_ids[i]);
               }
          }
     }

     if (_dynamic)
          ReadEventsTxt(current_time);
}


void EventManager::Update_Events(double time )
{
     //1. pruefen ob in _event_times der zeitstempel time zu finden ist. Wenn ja zu 2. sonst zu 3.
     //2. Event aus _event_times und _event_values verarbeiten (Tuere schliessen/oeffnen, neues Routing)
     //   Dann pruefen, ob eine neue Zeile in der .txt Datei steht
     //3. .txt Datei auf neue Zeilen pruefen. Wenn es neue gibt diese Events verarbeiten ( Tuere schliessen/oeffnen,
     //   neues Routing) ansonsten fertig

     //_deltaT = d;
     const vector<Pedestrian*>& _allPeds = _building->GetAllPedestrians();

     //zuerst muss geprueft werden, ob die Peds, die die neuen Infos schon haben sie an andere Peds weiter-
     //leiten muessen (wenn diese sich in der naechsten Umgebung befinden)
     //int currentTime = _allPeds[0]->GetGlobalTime();
     int currentTime = Pedestrian::GetGlobalTime();

     if ( (currentTime != _lastUpdateTime) &&
               ((currentTime % _updateFrequency) == 0))
     {
          for (unsigned int p1 = 0; p1 < _allPeds.size(); p1++) {
               Pedestrian* ped1 = _allPeds[p1];
               if (ped1->GetNewEventFlag()) {
                    int rID = ped1->GetRoomID();
                    int srID = ped1->GetSubRoomID();

                    for (unsigned int p2 = 0; p2 < _allPeds.size(); p2++) {
                         Pedestrian* ped2 = _allPeds[p2];
                         //same room and subroom
                         if (rID == ped2->GetRoomID()
                                   && srID == ped2->GetSubRoomID()) {
                              if (!ped2->GetNewEventFlag()
                                        && ped2->GetReroutingTime()
                                        > 2.0) {
                                   //wenn der Pedestrian die neuen Infos noch nicht hat und eine Reroutingtime von > 2 Sekunden hat, pruefen ob er nah genug ist
                                   double dist= (ped1->GetPos()-ped2->GetPos()).Norm();

                                   if (dist <= _updateRadius) { // wenn er nah genug (weniger als 2m) ist, Info weitergeben (Reroutetime auf 2 Sek)
                                        //ped->RerouteIn(2.0);
                                        ped2->RerouteIn(0.0);
                                   }
                              }
                         }
                    }
               }
          }
          _lastUpdateTime = currentTime;
          //cout<<"updating..."<<currentTime<<endl<<endl;
     }

     //dann muss die Reroutingzeit der Peds, die die neuen Infos noch nicht haben, aktualisiert werden:
     for (unsigned int p1 = 0; p1 < _allPeds.size(); p1++) {
          Pedestrian* ped1 = _allPeds[p1];
          ped1->UpdateReroutingTime();
          if (ped1->IsReadyForRerouting()) {
               ped1->ClearMentalMap();
               ped1->ResetRerouting();
               ped1->SetNewEventFlag(true);
               ped1->SetSpotlight(true);
          }
     }

     //Events finden
     for (unsigned int i = 0; i < _event_times.size(); i++) {
          if (fabs(_event_times[i] - time) < J_EPS_EVENT) {
               //Event with current time stamp detected
               Log->Write("INFO:\tEvent: after %.2f sec: ", time);
               if (_event_states[i].compare("close") == 0) {
                    CloseDoor(_event_ids[i]);
               } else {
                    OpenDoor(_event_ids[i]);
               }
          }
     }

     if (_dynamic)
          ReadEventsTxt(time);
}

/***************
 Event handling
 **************/
//close the door if it was open and relaunch the routing procedure
void EventManager::CloseDoor(int id)
{
     Transition *t = _building->GetTransition(id);
     if (t->IsOpen()) {
          t->Close();
          Log->Write("INFO:\tClosing door %d ", id);
          ChangeRouting(id, "close");
     } else {
          Log->Write("WARNING: \tdoor %d is already close", id);
     }

}

//open the door if it was open and relaunch the routing procedure
void EventManager::OpenDoor(int id)
{
     Transition *t = _building->GetTransition(id);
     if (!t->IsOpen()) {
          t->Open();
          Log->Write("INFO:\tOpening door %d ", id);
          ChangeRouting(id, "open");
     } else {
          Log->Write("WARNING: \tdoor %d is already open", id);
     }
}

void EventManager::ChangeRouting(int id, const std::string& state)
{

     //Pedestrians sollen, damit es realitaetsnaeher wird, je nachdem wo sie stehen erst spaeter(abh. von der
     //Entfernung zur Tuer) merken, dass sich Tueren aendern. Oder sie bekommen die Info von anderen Pedestrians
     Transition *t = _building->GetTransition(id);
     //Abstand der aktuellen Position des Pedestrians zur entsprechenden Tuer: Tuer als Linie sehen und mit
     //DistTo(ped.GetPos()) den Abstand messen. Reroutezeit dann aus Entfernung und Geschwindigkeit berechnen.

     for(auto&& ped:_building->GetAllPedestrians())
     {
          //if(_allPedestrians[p]->GetExitIndex()==t->GetUniqueID()){
          ped->SetNewEventFlag(false);
          double dist = t->DistTo(ped->GetPos());
          const Point& v = ped->GetV();
          double norm = v.Norm();
          if (norm == 0.0) {
               norm = 0.01;
          }
          double time = dist / norm;
          if (time < 1.0) {
               ped->ClearMentalMap();
               ped->ResetRerouting();
               ped->SetNewEventFlag(true);
          } else {
               ped->RerouteIn(time);
          }
     }

     //Create and save a graph corresponding to the actual state of the building.
     if(CreateRoutingEngine(_building)==false)
     {
          Log->Write("ERROR: \tcannot create a routing engine with the new event");
     }
}

void EventManager::GetEvent(char* c)
{
     int split = 0;
     string type = "";
     string id = "";
     string state = "";
     for (int i = 0; i < 20; i++) {
          if (!c[i]) {
               break;
          } else if (c[i] == ' ') {
               split++;
          } else if (c[i] == '\n') {

          } else {
               if (split == 0) {
                    type += c[i];
               } else if (split == 1) {
                    id += c[i];
               } else if (split == 2) {
                    state += c[i];
               }
          }
     }
     if (state.compare("close") == 0) {
          CloseDoor(atoi(id.c_str()));
     } else {
          OpenDoor(atoi(id.c_str()));
     }
}

bool EventManager::CreateRoutingEngine(Building* _b, int first_engine)
{
     std::vector<int> closed_doors;

     for(auto&& t:_b->GetAllTransitions())
     {
          if(t.second->IsOpen()==false)
               closed_doors.push_back(t.second->GetID());
     }
     std::sort(closed_doors.begin(), closed_doors.end());

     //create the key as string.
     string key="";
     for(int door:closed_doors)
     {
          if(key.empty())
               key.append(std::to_string(door));
          else
               key.append(":"+std::to_string(door));
     }

     //the first (default) engine was created in the simulation
     // collect the defined routers
     if(first_engine)
     {
          RoutingEngine* engine=_b->GetRoutingEngine();
          _eventEngineStorage[key]=engine;

          for(auto&& rout: engine->GetAvailableRouters())
          {
               _availableRouters.push_back(rout->GetStrategy());
          }
          Log->Write("INFO: \tAdding a new routing Engine with the key: "+key+"\n");
          return true;
     }

     // the engine was not created
     // create a new one with the actual configuration
     if (_eventEngineStorage.count(key)==0)
     {
          //std::shared_ptr<RoutingEngine> engine = std::shared_ptr<RoutingEngine>(new RoutingEngine());
          //engine.get()->Init(_b);
          //_eventEngineStorage[key]=engine.get();

          //populate the engine with the routers defined in the ini file
          //and initialize
          RoutingEngine* engine= new RoutingEngine();
          for(auto&& rout:_availableRouters)
          {
               engine->AddRouter(CreateRouter(rout));
          }

          if(engine->Init(_b)==false)
               return false;

          //save the configuration
          _eventEngineStorage[key]=engine;
          Log->Write("INFO: \tAdding a new routing Engine with the key: "+key+"\n");
     }
     else
     {
          Log->Write("INFO: \tA routing already exits with the key: "+key+"\n");
     }

     return true;
}

Router * EventManager::CreateRouter(const RoutingStrategy& strategy)
{
     Router * rout=nullptr;

     switch(strategy)
     {
     case ROUTING_LOCAL_SHORTEST:
          rout = new GlobalRouter(ROUTING_LOCAL_SHORTEST, ROUTING_LOCAL_SHORTEST);
          break;

     case ROUTING_GLOBAL_SHORTEST:
          rout = new GlobalRouter(ROUTING_GLOBAL_SHORTEST, ROUTING_GLOBAL_SHORTEST);
          break;

     case ROUTING_QUICKEST:
          rout = new QuickestPathRouter(ROUTING_QUICKEST, ROUTING_QUICKEST);
          break;

     case ROUTING_NAV_MESH:
          rout = new MeshRouter(ROUTING_NAV_MESH, ROUTING_NAV_MESH);
          break;

     case ROUTING_DUMMY:
          rout = new DummyRouter(ROUTING_DUMMY, ROUTING_DUMMY);
          break;

     case ROUTING_SAFEST:
          rout = new SafestPathRouter(ROUTING_SAFEST, ROUTING_SAFEST);
          break;

     case ROUTING_COGNITIVEMAP:
          rout = new CognitiveMapRouter(ROUTING_COGNITIVEMAP, ROUTING_COGNITIVEMAP);
          break;

     default:
          Log->Write("ERROR: \twrong value for routing strategy [%d]!!!\n", strategy );
          break;

     }
     return rout;
}


