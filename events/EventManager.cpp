#include <cstdio>
#include <cstdlib> 
#include <iostream>
#include <string>
#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"

using namespace std;

void readEventsXml(string eventfile){
    int event_count=0;

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
	event_times[event_count]=atoi(e->Attribute("time"));
	event_values[event_count]=e->Attribute("type"))+"-";
	event_values[event_count]=e->Attribute("id"))+"-";
	event_values[event_count]=e->Attribute("state"));
	event_count++;
    }
}