#include <string>
#include <cstdio>
#include <cstdlib> 
#include <iostream>
#include <vector>
#include "../geometry/Building.h"
#include "../geometry/Transition.h"

class EventManager {
private:
    std::vector<double> _event_times;
    std::vector<std::string> _event_types;
    std::vector<std::string> _event_states;
    std::vector<int> _event_ids;
    std::string _projectFilename;
    std::string _projectRootDir;
    Building *_building;
    double _deltaT;

public:
    //Konstruktor
    EventManager(Building *_b);
    //Dateien einlesen
    void SetProjectFilename(const std::string &filename) ;
    void SetProjectRootDir(const std::string &filename);
    void readEventsXml();
    void listEvents();
    void readEventsTxt(std::string eventfile);
    //Update
    void Update_Events(double time, double d);
    //Eventhandling
    void closeDoor(int id);
    void openDoor(int id);
    void changeRouting(int id, std::string state);


};
