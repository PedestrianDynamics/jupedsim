#include <string>
#include <cstdio>
#include <cstdlib> 
#include <iostream>
#include <vector>

class EventManager {
private:
    std::vector<double> _event_times;
    std::vector<std::string> _event_values;
    std::string _projectFilename;
    std::string _projectRootDir;

public:
    EventManager();
    void readEventsXml();
    void listEvents();
    void Update_Events(double time);
    void readEventsTxt(std::string eventfile);
    void SetProjectFilename(const std::string &filename) ;
    void SetProjectRootDir(const std::string &filename);
};
