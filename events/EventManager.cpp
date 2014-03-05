#include <cstdio>
#include <cstdlib> 
#include <iostream>
#include <string>
#include "EventManager.h"
#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"

using namespace std;

/*******************
 Konstruktoren
 ******************/

EventManager::EventManager(){
    _event_times=vector<double>();
    _event_values=vector<string>();
    _projectFilename = "";
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
        string tmp=e->Attribute("type");
        tmp=tmp+"-";
        tmp=tmp+e->Attribute("id")+"-";
        tmp=tmp+=e->Attribute("state");
        _event_values.push_back(tmp);
    }
    printf("INFO: \tEvents were read\n");
}

void EventManager::listEvents(){
    if(_event_times.size()==0&&_event_values.size()==0){
        //Log->Write("INFO: \tNo events in the events.xml");
        printf("INFO: \tNo events in the events.xml\n");
    }
    else if(_event_times.size()!=_event_values.size()){
        //Log->Write("ERROR: \tThe event.xml is irregular.");
        printf("ERROR: \tThe event.xml is irregular.\n");
    }
    else{
        int i;
        for(i=0;i<_event_times.size();i++){
           // Log->Write("INFO: \tEvent "+(i+1)+" after "+_event_times[i]+" sec.: "+_event_values[i]);

            cout <<"INFO: \tEvent "<<i+1<<" after "<< _event_times[i] << "sec.: "<<_event_values[i] <<endl;
        }
    }

}

/***********
 Update
 **********/

void EventManager::Update_Events(double time){
  
}
