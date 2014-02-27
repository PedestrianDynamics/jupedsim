#include <cstdio>
#include <cstdlib> 
#include <iostream>
#include <string>
#include <vector>

class EventManager {
private:
    std::vector<int> _event_times;
    std::vector<string> _event_values;

public:
    EventManager();
    readEventsXml(string eventfile);
};