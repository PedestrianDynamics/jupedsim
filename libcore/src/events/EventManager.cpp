/**
 * \file        EventManager.cpp
 * \date        Jul 4, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
#include "EventManager.h"

#include "Event.h"
#include "general/Logger.h"
#include "geometry/SubRoom.h"
#include "mpi/LCGrid.h"
#include "pedestrian/Knowledge.h"
#include "pedestrian/Pedestrian.h"

#include <cmath>
#include <map>
#include <string>
#include <tinyxml.h>

EventManager::EventManager(Configuration * config, Building * _b)
{
    _config   = config;
    _building = _b;
}

bool EventManager::ReadEventsXml()
{
    LOG_INFO("Looking for pre-defined events in other files");
    //get the geometry filename from the project file
    TiXmlDocument doc(_config->GetProjectFile().string());
    if(!doc.LoadFile()) {
        LOG_ERROR("Cannot parse project file: {}", doc.ErrorDesc());
        return false;
    }

    TiXmlElement * xMainNode = doc.RootElement();

    fs::path eventfile{};
    if(xMainNode->FirstChild("events_file")) {
        eventfile = _config->GetProjectRootDir() /
                    xMainNode->FirstChild("events_file")->FirstChild()->Value();
        LOG_INFO("events<{}>", eventfile.string());
    } else if(
        xMainNode->FirstChild("header") &&
        xMainNode->FirstChild("header")->FirstChild("events_file")) {
        eventfile =
            _config->GetProjectRootDir() /
            xMainNode->FirstChild("header")->FirstChild("events_file")->FirstChild()->Value();
        LOG_INFO("events<{}>", eventfile.string());
    } else {
        LOG_INFO("No events found");
        return true;
    }

    LOG_INFO("Parsing event file");
    TiXmlDocument docEvent(eventfile.string());
    if(!docEvent.LoadFile()) {
        LOG_ERROR("Cannot parse event file: {}", docEvent.ErrorDesc());
        return false;
    }

    TiXmlElement * xRootNode = docEvent.RootElement();
    if(!xRootNode) {
        LOG_ERROR("Event file root element missing");
        return false;
    }

    if(xRootNode->ValueStr() != "JPScore") {
        LOG_ERROR("Event file root element should be 'JPScore'");
        return false;
    }

    TiXmlNode * xEvents = xRootNode->FirstChild("events");
    if(!xEvents) {
        LOG_ERROR("No events found");
        return false;
    }

    for(TiXmlElement * e = xEvents->FirstChildElement("event"); e;
        e                = e->NextSiblingElement("event")) {
        int id      = std::stoi(e->Attribute("id"));
        double zeit = std::stoi(e->Attribute("time"));
        std::string state(e->Attribute("state"));
        std::string type(e->Attribute("type"));
        _events.emplace_back(id, zeit, type, state);
    }
    LOG_INFO("Events have been initialized");

    return true;
}

void EventManager::ListEvents()
{
    for(const auto & event : _events) {
        LOG_INFO("{}", event.GetDescription());
    }
}

bool EventManager::ProcessEvent()
{
    bool eventProcessed = false;

    for(const auto & event : _events) {
        if(fabs(event.GetTime() - Pedestrian::GetGlobalTime()) < J_EPS_EVENT) {
            //Event with current time stamp detected
            LOG_INFO("Event: after {:.2f} sec", Pedestrian::GetGlobalTime());
            switch(event.GetAction()) {
                case EventAction::OPEN:
                    OpenDoor(event.GetId());
                    break;
                case EventAction::CLOSE:
                    CloseDoor(event.GetId());
                    break;
                case EventAction::TEMP_CLOSE:
                    TempCloseDoor(event.GetId());
                    break;
                case EventAction::RESET_USAGE:
                    ResetDoor(event.GetId());
                    break;
                case EventAction::NOTHING:
                    LOG_WARNING("Unknown event action in events. open, close, reset or "
                                "temp_close. Default: do nothing");
                    break;
            }
            eventProcessed = true;
        }
    }
    return eventProcessed;
}

void EventManager::CloseDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->Close(true);
    LOG_INFO("Closing door {}", id);
}

void EventManager::TempCloseDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->TempClose(true);
    LOG_INFO("Closing door {}", id);
}

void EventManager::OpenDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->Open(true);
    LOG_INFO("Opening door {}", id);
}

void EventManager::ResetDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->ResetDoorUsage();
    LOG_INFO("Resetting door usage {}", id);
}

bool EventManager::ReadSchedule()
{
    LOG_INFO("Reading schedule");

    //get the geometry filename from the project file
    TiXmlDocument doc(_config->GetProjectFile().string());
    if(!doc.LoadFile()) {
        LOG_ERROR("Cannot parse project file: {}", doc.ErrorDesc());
        return false;
    }

    TiXmlElement * xMainNode = doc.RootElement();

    fs::path scheduleFile = "";
    if(xMainNode->FirstChild("schedule_file")) {
        scheduleFile = _config->GetProjectRootDir() /
                       xMainNode->FirstChild("schedule_file")->FirstChild()->Value();
        LOG_INFO("events<{}>", scheduleFile.string());
    } else if(
        xMainNode->FirstChild("header") &&
        xMainNode->FirstChild("header")->FirstChild("schedule_file")) {
        scheduleFile =
            _config->GetProjectRootDir() /
            xMainNode->FirstChild("header")->FirstChild("schedule_file")->FirstChild()->Value();
        LOG_INFO("events<{}>", scheduleFile.string());
    } else {
        LOG_INFO("No events found");
        return true;
    }

    LOG_INFO("Parsing schedule file");
    TiXmlDocument docSchedule(scheduleFile.string());
    if(!docSchedule.LoadFile()) {
        LOG_ERROR("Cannot parse schedule file: {}", docSchedule.ErrorDesc());
        return false;
    }

    TiXmlElement * xRootNode = docSchedule.RootElement();
    if(!xRootNode) {
        LOG_ERROR("Schedule file root element missing");
        return false;
    }

    if(xRootNode->ValueStr() != "JPScore") {
        LOG_ERROR("Schedule file root element is not 'JPScore'");
        return false;
    }

    // Read groups
    TiXmlNode * xGroups = xRootNode->FirstChild("groups");
    if(!xGroups) {
        LOG_ERROR("No groups found in schedule file");
        return false;
    }

    std::map<int, int> groupMaxAgents;
    std::map<int, std::vector<int>> groupDoor;

    for(TiXmlElement * e = xGroups->FirstChildElement("group"); e;
        e                = e->NextSiblingElement("group")) {
        int id = std::stoi(e->Attribute("id"));

        int max_agents = std::numeric_limits<int>::min();
        if(e->Attribute("max_agents")) {
            max_agents = std::stoi(e->Attribute("max_agents"));
        }

        std::vector<int> member;
        for(TiXmlElement * xmember = e->FirstChildElement("member"); xmember;
            xmember                = xmember->NextSiblingElement("member")) {
            int tId = std::stoi(xmember->Attribute("t_id"));
            member.push_back(tId);
        }
        groupDoor[id]      = member;
        groupMaxAgents[id] = max_agents;
    }

    //Set max agents
    for(auto const [groupID, maxAgents] : groupMaxAgents) {
        if(maxAgents > 0) {
            for(int transID : groupDoor.at(groupID)) {
                _building->GetTransition(transID)->SetMaxDoorUsage(maxAgents);
            }
        }
    }

    // Read times
    TiXmlNode * xTimes = xRootNode->FirstChild("times");
    if(!xTimes) {
        LOG_ERROR("No times found");
        return false;
    }

    for(TiXmlElement * e = xTimes->FirstChildElement("time"); e;
        e                = e->NextSiblingElement("time")) {
        int id           = std::stoi(e->Attribute("group_id"));
        int closing_time = std::stoi(e->Attribute("closing_time"));

        std::string resetString;
        if(e->Attribute("reset")) {
            resetString = e->Attribute("reset");
            std::transform(resetString.begin(), resetString.end(), resetString.begin(), ::tolower);
        }
        bool resetDoor = (resetString == "true");

        std::vector<int> timeOpen;
        std::vector<int> timeClose;
        std::vector<int> timeReset;

        for(TiXmlElement * time = e->FirstChildElement("t"); time;
            time                = time->NextSiblingElement("t")) {
            int t = std::stoi(time->Attribute("t"));
            if(resetDoor) {
                timeReset.push_back(t);
            } else {
                timeOpen.push_back(t);
            }
            timeClose.push_back(t + closing_time);
        }

        for(auto door : groupDoor[id]) {
            for(auto open : timeOpen) {
                Event event(door, open, "door", "open");
                _events.push_back(event);
            }

            for(auto close : timeClose) {
                Event event(door, close, "door", "temp_close");
                _events.push_back(event);
            }

            for(auto reset : timeReset) {
                Event event(door, reset, "door", "reset");
                _events.push_back(event);
            }
        }
    }
    LOG_INFO("Schedule initialized");
    return true;
}
