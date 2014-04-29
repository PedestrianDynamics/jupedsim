#include <string>
#include <cstdio>
#include <cstdlib> 
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <stdio.h>
#include "../geometry/Building.h"
#include "../geometry/Transition.h"
#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"
#include "../IO/IODispatcher.h"
#include "../routing/RoutingEngine.h"
#include "../pedestrian/Pedestrian.h"

extern OutputHandler* Log;

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
  FILE *_file;
  bool _dynamic;
  int _eventCounter;

 public:
  //Konstruktor
  EventManager(Building *_b);
  //Dateien einlesen
  void SetProjectFilename(const std::string &filename) ;
  void SetProjectRootDir(const std::string &filename);
  void readEventsXml();
  void listEvents();
  void readEventsTxt(double time);
  //Update
  void Update_Events(double time, double d);
  //Eventhandling
  void closeDoor(int id);
  void openDoor(int id);
  void changeRouting(int id, std::string state);
  void getTheEvent(char* c);
};
