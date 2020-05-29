/**
 * Copyright (c) 2020 Forschungszentrum Jülich GmbH. All rights reserved.
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
 **/
#include "EventFileParser.h"

#include "events/DoorEvent.h"
#include "events/EventHelper.h"
#include "general/Macros.h"
#include "tinyxml.h"

#include <Logger.h>

void EventFileParser::ParseDoorEvents(EventManager & eventManager, const fs::path & eventFile)
{
    LOG_INFO("Parsing event file");
    TiXmlDocument docEvent(eventFile.string());
    if(!docEvent.LoadFile()) {
        LOG_ERROR("Cannot parse event file: {}", docEvent.ErrorDesc());
        return;
    }

    TiXmlElement * xRootNode = docEvent.RootElement();
    if(!xRootNode) {
        LOG_ERROR("DoorEvent file root element missing");
        return;
    }

    if(xRootNode->ValueStr() != "JPScore") {
        LOG_ERROR("DoorEvent file root element should be 'JPScore'");
        return;
    }

    TiXmlNode * xEvents = xRootNode->FirstChild("events");
    if(!xEvents) {
        LOG_ERROR("No events found");
        return;
    }

    for(TiXmlElement * e = xEvents->FirstChildElement("event"); e;
        e                = e->NextSiblingElement("event")) {
        // Read id and check for correct value
        int id;
        if(const char * attribute = e->Attribute("id"); attribute) {
            if(int value = xmltoi(attribute, -1);
               value > -1 && attribute == std::to_string(value)) {
                id = value;
            } else {
                LOG_ERROR("event: id is set but no integer");
                continue;
            }
        } else {
            LOG_ERROR("event: id required");
            continue;
        }

        // Read time of events
        double time;
        if(const char * attribute = e->Attribute("time"); attribute) {
            if(double value = xmltof(attribute, std::numeric_limits<double>::min()); value >= 0.) {
                time = value;
            } else {
                LOG_ERROR("event {:d}: time not set properly", id);
                continue;
            }
        } else {
            LOG_ERROR("event {:d}: time required", id);
            continue;
        }

        // Read state
        std::string state;
        if(const char * attribute = e->Attribute("state"); attribute) {
            std::string in = xmltoa(attribute, "");
            if(!in.empty()) {
                state = in;
            } else {
                LOG_ERROR("event {:d}: state not set properly", id);
                continue;
            }
        } else {
            LOG_ERROR("event {:d}: state required", id);
            continue;
        }

        if(auto action = EventHelper::StringToEventAction(state); !action.has_value()) {
            LOG_ERROR("event {:d}: unknown event {}", id, state);
        } else {
            eventManager.AddEvent(std::make_unique<DoorEvent>(id, time, action.value()));
        }
    }
    LOG_INFO("Events have been initialized");
}

void EventFileParser::ParseSchedule(EventManager & eventManager, const fs::path & scheduleFile)
{
    LOG_INFO("Parsing schedule file");
    TiXmlDocument docSchedule(scheduleFile.string());
    if(!docSchedule.LoadFile()) {
        LOG_ERROR("Cannot parse schedule file: {}", docSchedule.ErrorDesc());
        return;
    }

    TiXmlElement * xRootNode = docSchedule.RootElement();
    if(!xRootNode) {
        LOG_ERROR("Schedule file root element missing");
        return;
    }

    if(xRootNode->ValueStr() != "JPScore") {
        LOG_ERROR("Schedule file root element is not 'JPScore'");
        return;
    }

    // Read groups
    TiXmlNode * xGroups = xRootNode->FirstChild("groups");
    if(!xGroups) {
        LOG_ERROR("No groups found in schedule file");
        return;
    }

    std::map<int, int> groupMaxAgents;
    std::map<int, std::vector<int>> groupDoor;

    for(TiXmlElement * e = xGroups->FirstChildElement("group"); e;
        e                = e->NextSiblingElement("group")) {
        // Read id and check for correct value
        int id;
        if(const char * attribute = e->Attribute("id"); attribute) {
            if(int value = xmltoi(attribute, -1);
               value > -1 && attribute == std::to_string(value)) {
                id = value;
            } else {
                LOG_ERROR("schedule group: id is set but no integer");
                continue;
            }
        } else {
            LOG_ERROR("schedule group: id required");
            continue;
        }

        std::vector<int> member;
        for(TiXmlElement * xmember = e->FirstChildElement("member"); xmember;
            xmember                = xmember->NextSiblingElement("member")) {
            int tId = std::stoi(xmember->Attribute("t_id"));
            member.push_back(tId);
        }
        groupDoor[id] = member;
    }

    // Read times
    TiXmlNode * xTimes = xRootNode->FirstChild("times");
    if(!xTimes) {
        LOG_ERROR("No times found");
        return;
    }

    for(TiXmlElement * e = xTimes->FirstChildElement("time"); e;
        e                = e->NextSiblingElement("time")) {
        // Read id and check for correct value
        int id;
        if(const char * attribute = e->Attribute("group_id"); attribute) {
            if(int value = xmltoi(attribute, -1);
               value > -1 && attribute == std::to_string(value)) {
                id = value;
            } else {
                LOG_ERROR("schedule times: group_id is set but no integer");
                continue;
            }
        } else {
            LOG_ERROR("schedule times: group_id required");
            continue;
        }

        int closingTime;
        if(const char * attribute = e->Attribute("closing_time"); attribute) {
            if(int value = xmltoi(attribute, -1); value > 0 && attribute == std::to_string(value)) {
                closingTime = value;
            } else {
                LOG_ERROR("schedule times {:d}: closing_time is not set properly", id);
                continue;
            }
        } else {
            LOG_ERROR("schedule times {:d}: closing_time required", id);
            continue;
        }

        bool resetDoor = false;
        if(const char * attribute = e->Attribute("reset"); attribute) {
            std::string in = xmltoa(attribute, "");
            if(!in.empty()) {
                std::string resetString = in;
                std::transform(
                    resetString.begin(), resetString.end(), resetString.begin(), ::tolower);
                resetDoor = (resetString == "true");
            } else {
                LOG_ERROR("schedule times {:d}: reset has no proper input, set to false", id);
            }
        }

        std::vector<double> timeOpen;
        std::vector<double> timeClose;
        std::vector<double> timeReset;

        for(TiXmlElement * time = e->FirstChildElement("t"); time;
            time                = time->NextSiblingElement("t")) {
            double openingTime = xmltof(time->Attribute("t"), -1.);
            if(openingTime < 0) {
                LOG_ERROR("schedule times {:d}: t has no proper input", id);
                continue;
            }
            if(resetDoor) {
                timeReset.push_back(openingTime);
            } else {
                timeOpen.push_back(openingTime);
            }

            timeClose.push_back(openingTime + closingTime);
        }

        for(auto door : groupDoor[id]) {
            for(auto open : timeOpen) {
                eventManager.AddEvent(
                    std::make_unique<DoorEvent>(door, open, EventAction::DOOR_OPEN));
            }

            for(auto close : timeClose) {
                eventManager.AddEvent(
                    std::make_unique<DoorEvent>(door, close, EventAction::DOOR_TEMP_CLOSE));
            }

            for(auto reset : timeReset) {
                eventManager.AddEvent(
                    std::make_unique<DoorEvent>(door, reset, EventAction::DOOR_RESET_USAGE));
            }
        }
    }
}

std::map<int, int> EventFileParser::ParseMaxAgents(const fs::path & scheduleFile)
{
    LOG_INFO("Parsing schedule file for max agents");
    TiXmlDocument docSchedule(scheduleFile.string());
    if(!docSchedule.LoadFile()) {
        LOG_ERROR("Cannot parse schedule file: {}", docSchedule.ErrorDesc());
        return std::map<int, int>();
    }

    TiXmlElement * xRootNode = docSchedule.RootElement();
    if(!xRootNode) {
        LOG_ERROR("Schedule file root element missing");
        return std::map<int, int>();
    }

    if(xRootNode->ValueStr() != "JPScore") {
        LOG_ERROR("Schedule file root element is not 'JPScore'");
        return std::map<int, int>();
    }

    // Read groups
    TiXmlNode * xGroups = xRootNode->FirstChild("groups");
    if(!xGroups) {
        LOG_ERROR("No groups found in schedule file");
        return std::map<int, int>();
    }

    std::map<int, int> doorMaxAgents;

    for(TiXmlElement * e = xGroups->FirstChildElement("group"); e;
        e                = e->NextSiblingElement("group")) {
        int maxAgents;
        if(const char * attribute = e->Attribute("max_agents"); attribute) {
            if(int value = xmltoi(attribute, -1); value > 0 && attribute == std::to_string(value)) {
                maxAgents = value;
            } else {
                LOG_ERROR("schedule group: max_agents is set but no integer");
                continue;
            }
        } else {
            // if no max_agents set, or not set correctly continue
            continue;
        }

        for(TiXmlElement * xmember = e->FirstChildElement("member"); xmember;
            xmember                = xmember->NextSiblingElement("member")) {
            int transID;
            if(const char * attribute = e->Attribute("t"); attribute) {
                if(int value = xmltoi(attribute, -1);
                   value >= 0 && attribute == std::to_string(value)) {
                    transID = value;
                } else {
                    LOG_ERROR("schedule times: trans_id not set properly");
                    continue;
                }
            } else {
                LOG_ERROR("schedule times: trans_id required");
                continue;
            }

            doorMaxAgents[transID] = maxAgents;
        }
    }

    return doorMaxAgents;
}
