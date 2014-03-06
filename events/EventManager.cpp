#include <cstdio>
#include <cstdlib> 
#include <iostream>
#include <string>
#include <math.h>
#include "EventManager.h"
#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"

using namespace std;

/*******************
 Konstruktoren
 ******************/

EventManager::EventManager(Building *_b){
    _event_times=vector<double>();
    _event_types=vector<string>();
    _event_states=vector<string>();
    _event_ids=vector<int>();
    _projectFilename = "";
    _building = _b;
}

/*******************
 Dateien einlesen
 ******************/
void EventManager::SetProjectFilename(const std::string &filename){
    _projectFilename=filename;
}

void EventManager::SetProjectRootDir(const std::string &filename){
    _projectRootDir= filename;
}

void EventManager::readEventsXml(){
    printf("INFO: \tReading events\n ");
    //get the geometry filename from the project file
    TiXmlDocument doc(_projectFilename);
    if (!doc.LoadFile()){
        //Log->Write("ERROR: \t%s", doc.ErrorDesc());
        //Log->Write("ERROR: \t could not parse the project file");
        printf("ERROR: \t%s\nRROR: \t could not parse the project file\n",doc.ErrorDesc());
        exit(EXIT_FAILURE);
    }

    //Log->Write("INFO: \tParsing the event file");
    printf("INFO: \tParsing the event file\n");
    TiXmlElement* xMainNode = doc.RootElement();
    string eventfile="";
    if(xMainNode->FirstChild("events")){
        eventfile=_projectRootDir+xMainNode->FirstChild("events")->FirstChild()->Value();
        //Log->Write("INFO: \tevents <"+eventfile+">");
        //printf("INFO: \t events <%s>\n", eventfile);
    }

    TiXmlDocument docEvent(eventfile);
    if(!docEvent.LoadFile()){
        //Log->Write("EROOR: \t%s",docEvent.ErrorDesc());
        //Log->Write("ERROR: \t could not parse the event file");
        printf("EROOR: \t%s\nERROR: \t could not parse the event file\n",docEvent.ErrorDesc());
        exit(EXIT_FAILURE);
    }

    TiXmlElement* xRootNode = docEvent.RootElement();
    if(!xRootNode){
        //Log->Write("ERROR:\tRoot element does not exist.");
        printf("ERROR:\tRoot element does not exist.\n");
        exit(EXIT_FAILURE);
    }

    if( xRootNode->ValueStr () != "JPScore" ) {
        //Log->Write("ERROR:\tRoot element value is not 'geometry'.");
        printf("ERROR:\tRoot element value is not 'JPScore'.");
        exit(EXIT_FAILURE);
    }

    TiXmlNode* xEvents = xRootNode->FirstChild("events");
    if(!xEvents){
        //Log->Write("ERROR:\tNo events found.");
        printf("ERROR:\tNo events found.\n");
        exit(EXIT_FAILURE);
    }
    
    for(TiXmlElement* e = xEvents->FirstChildElement("event"); e; e= e->NextSiblingElement("event")){
        _event_times.push_back(atoi(e->Attribute("time")));
        _event_types.push_back(e->Attribute("type"));
        _event_states.push_back(e->Attribute("state"));
        _event_ids.push_back(atoi(e->Attribute("id")));
    }
    printf("INFO: \tEvents were read\n");
}

void EventManager::listEvents(){
    if(_event_times.size()==0){
        //Log->Write("INFO: \tNo events in the events.xml");
        printf("INFO: \tNo events in the events.xml\n");
    }
    else{
        int i;
        for(i=0;i<_event_times.size();i++){
           // Log->Write("INFO: \tEvent "+(i+1)+" after "+_event_times[i]+" sec.: "+_event_values[i]);

            cout <<"INFO: \tEvent "<<i+1<<" after "<< _event_times[i] << "sec.: "<< _event_types[i] << " " << _event_states[i] << " " << _event_ids[i] << endl;
        }
    }

}

/***********
 Update
 **********/

void EventManager::Update_Events(double time, double d){
    //1. pruefen ob in _event_times der zeitstempel time zu finden ist. Wenn ja zu 2. sonst zu 3.
    //2. Event aus _event_times und _event_values verarbeiten (Tuere schliessen/oeffnen, neues Routing)
    //   Dann pruefen, ob eine neue Zeile in der .txt Datei steht
    //3. .txt Datei auf neue Zeilen pruefen. Wenn es neue gibt diese Events verarbeiten ( Tuere schliessen/oeffnen,
    //   neues Routing) ansonsten fertig
    int i;
    for(i=0;i<_event_times.size();i++){
        if(fabs(_event_times[i]-time)<0.0000001){
            //Event findet statt
            printf("%f: Event zum Zeitpunkt %f findet statt: ",time,_event_times[i]);
            cout << _event_types[i] << " " << _event_ids[i] << " " << _event_states[i] << endl;
            if(_event_states[i].compare("close")==0){
                closeDoor(_event_ids[i]);
            }
            else{
                openDoor(_event_ids[i]);
            }
        }
    }
}

/***************
 Eventhandling
 **************/
void EventManager::closeDoor(int id){
    //pruefen ob entsprechende Tuer schon zu ist, wenn nicht dann schliessen und neues Routing berechnen
    Transition *t=_building->GetTransition(id);
    if(t->IsOpen()){
        t->Close();
        cout << "Door " << id << " closed." << endl;
        changeRouting();
    }

}

void EventManager::openDoor(int id){
    //pruefen ob entsprechende Tuer schon offen ist, wenn nicht dann oeffnen und neues Routing berechnen
    Transition *t=_building->GetTransition(id);
    if(!t->IsOpen()){
        t->Open();
        cout << "Door " << id << " opened." << endl;
        changeRouting();
    }
}

void EventManager::changeRouting(){


}
