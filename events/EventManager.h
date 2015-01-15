/**
 * \file        EventManager.h
 * \date        Jul 4, 2014
 * \version     v0.6
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

#include <vector>
#include <string>

class Building;

extern OutputHandler* Log;

class EventManager
{
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
     long int _lastUpdateTime;

public:
     //constructor
     EventManager(Building *_b);
     void SetProjectFilename(const std::string &filename);
     void SetProjectRootDir(const std::string &filename);
     bool ReadEventsXml();
     void ListEvents();
     void ReadEventsTxt(double time);
     //Update
     void Update_Events(double time, double d);
     //Eventhandling
     void CloseDoor(int id);
     void OpenDoor(int id);
     void ChangeRouting(int id, const std::string& state);
     void GetEvent(char* c);
};
