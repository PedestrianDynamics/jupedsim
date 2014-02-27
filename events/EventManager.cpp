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
    _event_times=vector<int>();
    _event_values=vector<string>();
}

void readEventsXml(string eventfile){
    Log->Write("INFO: \tLoading and parsing the event file <%s>",eventfile.c_str());

    TiXmlDocument doc(eventfile);
    if(!doc.LoadFile()){
	Log->Write("EROOR: \t%s",doc.ErrorDesc());
	Log->Write("ERROR: \t could not parse the event file");
	exit(EXIT_FAILURE);
    }

    TiXmlElement* xMainNdoe = doc.RootElement();
    if(!xMainNode){
	Log->Write("ERROR:\tRoot element does not exist.");
	exit(EXIT_FAILURE);
    }
    
    TiXmlNode* xEvents = xMainNode->FirstChild("JPScore");
    if(!xEvents){
	Log->Write("ERROR:\tNo events found.");
	exit(EXIT_FAILURE);
    }
    
    for(TiXmlElement* e = xEvents->FirstChildElement("event"); e; e= e->NextSiblingElement("event")){
	_event_times.push_back(atoi(e->Attribute("time")));
	string tmp=e->Attribute("type"))+"-");
	tmp=tmp+e->Attribute("id"))+"-";
	tmp=tmp+=e->Attribute("state"));
	_event_values.push_back(tmp);
    }
}
