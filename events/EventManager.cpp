/**
 * \file        EventManager.cpp
 * \date        Jul 4, 2014
 * \version     v0.5
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
 
#include "EventManager.h"

using namespace std;

/*******************
 Konstruktoren
 ******************/

EventManager::EventManager(Building *_b)
{
     _event_times=vector<double>();
     _event_types=vector<string>();
     _event_states=vector<string>();
     _event_ids=vector<int>();
     _projectFilename = "";
     _building = _b;
     _deltaT= 0; // NULL;
     _eventCounter=0;
     _dynamic=false;
     _file = fopen("../events/events.txt","r");
     if(!_file) {
          //Log->Write("INFO:\tDatei events.txt nicht gefunden. Dynamisches Eventhandling nicht moeglich.");
     } else {
          Log->Write("INFO:\tDatei events.txt gefunden. Dynamisches Eventhandling moeglich.");
          _dynamic=true;
     }
}

/*******************
 Dateien einlesen
 ******************/
void EventManager::SetProjectFilename(const std::string &filename)
{
     _projectFilename=filename;
}

void EventManager::SetProjectRootDir(const std::string &filename)
{
     _projectRootDir= filename;
}

void EventManager::readEventsXml()
{
     //get the geometry filename from the project file
     TiXmlDocument doc(_projectFilename);
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \t could not parse the project file. ");
          exit(EXIT_FAILURE);
     }

     Log->Write("INFO: \tParsing the event file");
     TiXmlElement* xMainNode = doc.RootElement();
     string eventfile="";
     if(xMainNode->FirstChild("events")) {
          eventfile=_projectRootDir+xMainNode->FirstChild("events")->FirstChild()->Value();
          Log->Write("INFO: \tevents <"+eventfile+">");
     }

     TiXmlDocument docEvent(eventfile);
     if(!docEvent.LoadFile()) {
          //Log->Write("INFO: \t%s",docEvent.ErrorDesc());
          //Log->Write("INFO: \t could not parse the event file. So no Events are found.");
          //exit(EXIT_FAILURE);
          return;
     }
     Log->Write("INFO: \tReading events\n ");

     TiXmlElement* xRootNode = docEvent.RootElement();
     if(!xRootNode) {
          Log->Write("ERROR:\tRoot element does not exist.");
          exit(EXIT_FAILURE);
     }

     if( xRootNode->ValueStr () != "JPScore" ) {
          Log->Write("ERROR:\tRoot element value is not 'JPScore'.");
          exit(EXIT_FAILURE);
     }

     TiXmlNode* xEvents = xRootNode->FirstChild("events");
     if(!xEvents) {
          Log->Write("ERROR:\tNo events found.");
          exit(EXIT_FAILURE);
     }

     for(TiXmlElement* e = xEvents->FirstChildElement("event"); e; e= e->NextSiblingElement("event")) {
          _event_times.push_back(atoi(e->Attribute("time")));
          _event_types.push_back(e->Attribute("type"));
          _event_states.push_back(e->Attribute("state"));
          _event_ids.push_back(atoi(e->Attribute("id")));
     }
     Log->Write("INFO: \tEvents were read\n");
}

void EventManager::listEvents()
{
     if(_event_times.size()==0) {
          Log->Write("INFO: \tNo events in the events.xml");
     } else {
          unsigned int i;
          char buf[10],buf2[10];
          for(i=0; i<_event_times.size(); i++) {
               sprintf(buf,"%f",_event_times[i]);
               sprintf(buf2,"%d",_event_ids[i]);
               Log->Write("INFO: \tAfter "+string(buf)+" sec: "+_event_types[i]+" "+string(buf2)+" "+_event_states[i]);
          }
     }

}

void EventManager::readEventsTxt(double time)
{
     rewind(_file);
     char cstring[256];
     int lines=0;
     do {
          fgets(cstring,30,_file);
          if(cstring[0]!='#') { // keine Kommentarzeile
               lines++;
               if(lines>_eventCounter) {
                    Log->Write("INFO:\tEvent: after %f sec: ",time);
                    getTheEvent(cstring);
                    _eventCounter++;
               }
          }
     } while (feof(_file)==0);
}

/***********
 Update
 **********/

void EventManager::Update_Events(double time, double d)
{
     //1. pruefen ob in _event_times der zeitstempel time zu finden ist. Wenn ja zu 2. sonst zu 3.
     //2. Event aus _event_times und _event_values verarbeiten (Tuere schliessen/oeffnen, neues Routing)
     //   Dann pruefen, ob eine neue Zeile in der .txt Datei steht
     //3. .txt Datei auf neue Zeilen pruefen. Wenn es neue gibt diese Events verarbeiten ( Tuere schliessen/oeffnen,
     //   neues Routing) ansonsten fertig

     //zuerst muss die Reroutingzeit der Peds aktualisiert werden:
     _deltaT=d;
//     vector<Pedestrian*> _allPedestrians=_building->GetAllPedestrians();
//     int nSize = _allPedestrians.size();
//     for(int p=0; p<nSize; p++) {
//          _allPedestrians[p]->UpdateReroutingTime();
//          if(_allPedestrians[p]->IsReadyForRerouting()) {
//               _allPedestrians[p]->ClearMentalMap();
//               _allPedestrians[p]->ResetRerouting();
//          }
//     }

     for(unsigned i=0; i<_event_times.size(); i++) {
          if(fabs(_event_times[i]-time)<0.0000001) {
               //Event findet statt
               Log->Write("INFO:\tEvent: after %f sec: ",time);
               if(_event_states[i].compare("close")==0) {
                    closeDoor(_event_ids[i]);
               } else {
                    openDoor(_event_ids[i]);
               }
          }
     }
     if(_dynamic)
          readEventsTxt(time);
}

/***************
 Eventhandling
 **************/
void EventManager::closeDoor(int id)
{
     //pruefen ob entsprechende Tuer schon zu ist, wenn nicht dann schliessen und neues Routing berechnen
     Transition *t=_building->GetTransition(id);
     if(t->IsOpen()) {
          t->Close();
          Log->Write("\tDoor %d closed.",id);
          changeRouting(id,"close");
     } else {
          Log->Write("Door %d is already close yet.", id);
     }

}

void EventManager::openDoor(int id)
{
     //pruefen ob entsprechende Tuer schon offen ist, wenn nicht dann oeffnen und neues Routing berechnen
     Transition *t=_building->GetTransition(id);
     if(!t->IsOpen()) {
          t->Open();
          Log->Write("\tDoor %d opened.",id);
          changeRouting(id,"open");
     } else {
          Log->Write("Door %d is already open yet.", id);
     }
}

void EventManager::changeRouting(int id, string state)
{
     RoutingEngine* routingEngine= _building->GetRoutingEngine();
     routingEngine->Init(_building);
     _building->InitPhiAllPeds(_deltaT);
     vector<Pedestrian*> _allPedestrians=_building->GetAllPedestrians();
     unsigned int nSize = _allPedestrians.size();


     //clear the previous destinations
     // Method moved from Update_Events
     for(unsigned int p=0; p<nSize; p++) {
          _allPedestrians[p]->UpdateReroutingTime();
          if(_allPedestrians[p]->IsReadyForRerouting()) {
               _allPedestrians[p]->ClearMentalMap();
               _allPedestrians[p]->ResetRerouting();
          }
     }

     //Pedestrians sollen, damit es realitaetsnaeher wird, je nachdem wo sie stehen erst spaeter merken,
     //dass sich Tueren aendern.
     Transition *t = _building->GetTransition(id);
     //Abstand der aktuellen Position des Pedestrians zur entsprechenden Tuer: Tuer als Linie sehen und mit
     //DistTo(ped.GetPos()) den Abstand messen
     Line* l = new Line(t->GetPoint1(),t->GetPoint2());
     for (unsigned int p = 0; p < nSize; ++p) {
          //if(_allPedestrians[p]->GetExitIndex()==t->GetUniqueID()){
          double dist = l->DistTo(_allPedestrians[p]->GetPos());
          if(dist>0.0&&dist<0.5) {
               _allPedestrians[p]->ClearMentalMap();
          } else if(dist>=0.5&&dist<3.0) {
               _allPedestrians[p]->RerouteIn(1.0);
          } else {
               _allPedestrians[p]->RerouteIn(2.0);
          }
          //}
          //else{
          //  _allPedestrians[p]->ClearMentalMap();
          //}
     }
}

void EventManager::getTheEvent(char* c)
{
     int split = 0;
     string type = "";
     string id = "";
     string state = "";
     for(int i=0; i<20; i++) {
          if( ! c[i]) {
               break;
          } else if(c[i]==' ') {
               split++;
          } else if(c[i]=='\n') {

          } else {
               if(split==0) {
                    type+=c[i];
               } else if(split==1) {
                    id+=c[i];
               } else if(split==2) {
                    state+=c[i];
               }
          }

     }
     if(state.compare("close")==0) {
          closeDoor(atoi(id.c_str()));
     } else {
          openDoor(atoi(id.c_str()));
     }
}
