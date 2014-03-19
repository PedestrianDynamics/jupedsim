#include "EventManager.h"

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
    _deltaT=NULL;
    _eventCounter=0;
    _dynamic=false;
    //_file.open("../events/events.txt", ios::in);
    _file = fopen("../events/events.txt","r");
    if(!_file){
        cout << "INFO:\tDatei events.txt nicht gefunden. Dynamisches Eventhandling nicht moeglich." << endl;
    }
    else{
        cout << "INFO:\tDatei events.txt gefunden. Dynamisches Eventhandling moeglich." << endl;
        _dynamic=true;
    }
    //fclose(_file);
    //_file.close();
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

void EventManager::readEventsTxt(double time){
    rewind(_file);
    char cstring[256];
    int lines=0;
    do {
        lines++;
        fgets(cstring,20,_file);
        if(lines>_eventCounter){
            //cout << time << ": " << cstring << endl;
            printf("INFO:\tEvent zum Zeitpunkt %f findet statt: ",time);
            getTheEvent(cstring);
            _eventCounter++;
        }
    }while (feof(_file)==0);

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

    //zuerst muss die Reroutingzeit der Peds aktualisiert werden:
    _deltaT=d;
    vector<Pedestrian*> _allPedestrians=_building->GetAllPedestrians();
    int nSize = _allPedestrians.size();
    for(int p=0;p<nSize;p++){
        _allPedestrians[p]->UpdateReroutingTime();
        if(_allPedestrians[p]->IsReadyForRerouting()){
            _allPedestrians[p]->ClearMentalMap();
            _allPedestrians[p]->ResetRerouting();
        }
    }
    int i;
    for(i=0;i<_event_times.size();i++){
        if(fabs(_event_times[i]-time)<0.0000001){
            //Event findet statt
            printf("INFO:\t%f: Event zum Zeitpunkt %f findet statt: ",time,_event_times[i]);
            if(_event_states[i].compare("close")==0){
                closeDoor(_event_ids[i]);
            }
            else{
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
void EventManager::closeDoor(int id){
    //pruefen ob entsprechende Tuer schon zu ist, wenn nicht dann schliessen und neues Routing berechnen
    Transition *t=_building->GetTransition(id);
    if(t->IsOpen()){
        t->Close();
        cout << "Door " << id << " closed." << endl;
        changeRouting(id,"close");
    }
    else{
        cout << "Door " << id << " is already close yet." << endl;
    }

}

void EventManager::openDoor(int id){
    //pruefen ob entsprechende Tuer schon offen ist, wenn nicht dann oeffnen und neues Routing berechnen
    Transition *t=_building->GetTransition(id);
    if(!t->IsOpen()){
        t->Open();
        cout << "Door " << id << " opened." << endl;
        changeRouting(id,"open");
    }
    else{
        cout << "Door " << id << " is already open yet." << endl;
    }
}

void EventManager::changeRouting(int id, string state){
    RoutingEngine* routingEngine= _building->GetRoutingEngine();
    routingEngine->Init(_building);
    _building->InitPhiAllPeds(_deltaT);
    vector<Pedestrian*> _allPedestrians=_building->GetAllPedestrians();
    unsigned int nSize = _allPedestrians.size();
    //cout << nSize << endl;
    //for (int p = 0; p < nSize; ++p) {       !!!!!!so gehts, wenn alle Pedestrians sofort auf ein Ereignis reagieren.
      //  _allPedestrians[p]->ClearMentalMap();
    //}

    //Pedestrians sollen aber, damit es realitaetsnaeher wird, je nachdem wo sie stehen erst spaeter merken,
    //dass sich Tueren aendern.
    Transition *t = _building->GetTransition(id);
    for (int p = 0; p < nSize; ++p) {
        if(_allPedestrians[p]->GetExitIndex()==t->GetUniqueID()){
            double dist = _allPedestrians[p]->GetDistanceToNextTarget();
            if(dist>0.0&&dist<1.0){
                _allPedestrians[p]->ClearMentalMap();
            }
            else if(dist>=1.0&&dist<3.0){
                _allPedestrians[p]->RerouteIn(2.0);
            }
            else{
                _allPedestrians[p]->RerouteIn(5.0);
            }
        }
        else{
            _allPedestrians[p]->ClearMentalMap();
        }
    }
}

void EventManager::getTheEvent(char* c){
    int split = 0;
    string type = "";
    string id = "";
    string state = "";
    for(int i=0;i<20;i++){
        if(c[i]==NULL){
            break;
        }
        else if(c[i]==' '){
            split++;
        }
        else if(c[i]=='\n'){

        }
        else{
            if(split==0){
                type+=c[i];
            }
            else if(split==1){
                id+=c[i];
            }
            else if(split==2){
                state+=c[i];
            }
        }

    }
    if(state.compare("close")==0){
        closeDoor(atoi(id.c_str()));
    }
    else{
        openDoor(atoi(id.c_str()));
    }
}
