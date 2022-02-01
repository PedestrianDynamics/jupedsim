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
#include "EventFileParser.hpp"

#include "general/Macros.hpp"
#include "tinyxml.h"

#include <Logger.hpp>
#include <chrono>
#include <iterator>
#include <stdexcept>

namespace EventFileParser
{
ParsingError::ParsingError(const char * msg) : std::runtime_error(msg){};
ParsingError::ParsingError(const std::string & msg) : std::runtime_error(msg){};
} // namespace EventFileParser

std::vector<DoorEvent> EventFileParser::ParseDoorEvents(const fs::path & eventFile)
{
    LOG_INFO("Parsing event file");
    TiXmlDocument docEvent(eventFile.string());
    if(!docEvent.LoadFile()) {
        throw ParsingError(fmt::format(
            "Error parsing xml({},{}): {}",
            docEvent.ErrorRow(),
            docEvent.ErrorCol(),
            docEvent.ErrorDesc()));
    }

    TiXmlElement * xRootNode = docEvent.RootElement();
    if(!xRootNode) {
        throw ParsingError("<DoorEvent> root element not present xml");
    }

    if(xRootNode->ValueStr() != "JPScore") {
        throw ParsingError("DoorEvent file root element should be 'JPScore'");
    }

    std::vector<DoorEvent> events{};
    const auto * xml_events = xRootNode->FirstChild("events");
    if(xml_events == nullptr) {
        return events;
    }

    for(const auto * xml_event = xml_events->FirstChildElement("event"); xml_event != nullptr;
        xml_event              = xml_event->NextSiblingElement("event")) {
        // Read id and check for correct value
        int id;
        if(const auto * attribute = xml_event->Attribute("id"); attribute) {
            if(int value = xmltoi(attribute, -1);
               value > -1 && attribute == std::to_string(value)) {
                id = value;
            } else {
                throw ParsingError("Event: Attribute 'id' not a positive integer");
            }
        } else {
            throw ParsingError("Event: Attribute 'id' required");
        }

        // Read time of events
        double time;
        if(const auto * attribute = xml_event->Attribute("time"); attribute) {
            if(double value = xmltof(attribute, std::numeric_limits<double>::min()); value >= 0.) {
                time = value;
            } else {
                throw ParsingError("Event: Attribute 'time' not a positive floating point value");
            }
        } else {
            throw ParsingError("Event: Attribute 'time' required");
        }

        // Read state
        DoorEvent::Type event_type{};
        if(const auto * attribute = xml_event->Attribute("state"); attribute) {
            std::string in = xmltoa(attribute, "");
            try {
                event_type = from_string<DoorEvent::Type>(in);
            } catch(const std::exception & ex) {
                throw ParsingError("Event: Attribute 'state' is not a valid value");
            }
        } else {
            throw ParsingError("Event: Attribute 'state' required");
        }
        events.push_back(
            {std::chrono::duration_cast<std::chrono::nanoseconds>(
                 std::chrono::duration<double>(time)),
             id,
             event_type});
    }
    return events;
}

std::vector<DoorEvent> EventFileParser::ParseSchedule(const fs::path & scheduleFile)
{
    LOG_INFO("Parsing schedule file");
    TiXmlDocument doc(scheduleFile.string());
    if(!doc.LoadFile()) {
        throw ParsingError(fmt::format(
            "Error parsing xml({},{}): {}", doc.ErrorRow(), doc.ErrorCol(), doc.ErrorDesc()));
    }

    const auto * xRootNode = doc.RootElement();
    if(!xRootNode) {
        throw ParsingError("Schedule file root element missing");
    }

    if(xRootNode->ValueStr() != "JPScore") {
        throw ParsingError("Schedule file root element is not 'JPScore'");
    }

    // Read groups
    const auto * xGroups = xRootNode->FirstChild("groups");
    if(!xGroups) {
        throw ParsingError("No groups found in schedule file");
    }

    std::map<int, int> groupMaxAgents;
    std::map<int, std::vector<int>> groupDoor;
    std::vector<DoorEvent> events;

    for(const auto * e = xGroups->FirstChildElement("group"); e;
        e              = e->NextSiblingElement("group")) {
        // Read id and check for correct value
        int id;
        if(const char * attribute = e->Attribute("id"); attribute) {
            if(int value = xmltoi(attribute, -1);
               value > -1 && attribute == std::to_string(value)) {
                id = value;
            } else {
                throw ParsingError("schedule group: id is set but no integer");
            }
        } else {
            throw ParsingError("schedule group: id required");
        }

        std::vector<int> member;
        for(const auto * xmember = e->FirstChildElement("member"); xmember;
            xmember              = xmember->NextSiblingElement("member")) {
            int tId = std::stoi(xmember->Attribute("t_id"));
            member.push_back(tId);
        }
        groupDoor[id] = member;
    }

    // Read times
    const auto * xTimes = xRootNode->FirstChild("times");
    if(!xTimes) {
        throw ParsingError("No times found");
    }

    for(const auto * e = xTimes->FirstChildElement("time"); e; e = e->NextSiblingElement("time")) {
        // Read id and check for correct value
        int id;
        if(const char * attribute = e->Attribute("group_id"); attribute) {
            if(int value = xmltoi(attribute, -1);
               value > -1 && attribute == std::to_string(value)) {
                id = value;
            } else {
                throw ParsingError("schedule times: group_id is set but no integer");
            }
        } else {
            throw ParsingError("schedule times: group_id required");
        }

        int closingTime;
        if(const auto * attribute = e->Attribute("closing_time"); attribute) {
            if(int value = xmltoi(attribute, -1); value > 0 && attribute == std::to_string(value)) {
                closingTime = value;
            } else {
                throw ParsingError(
                    fmt::format("schedule times {:d}: closing_time is not set properly", id));
            }
        } else {
            throw ParsingError(fmt::format("schedule times {:d}: closing_time required", id));
        }

        bool resetDoor = false;
        if(const auto * attribute = e->Attribute("reset"); attribute) {
            std::string in = xmltoa(attribute, "");
            if(!in.empty()) {
                std::string resetString = in;
                std::transform(
                    resetString.begin(), resetString.end(), resetString.begin(), ::tolower);
                resetDoor = (resetString == "true");
            } else {
                throw ParsingError(fmt::format(
                    "schedule times {:d}: reset has no proper input, set to false", id));
            }
        }

        std::vector<double> timeOpen;
        std::vector<double> timeClose;
        std::vector<double> timeReset;

        for(const auto * time = e->FirstChildElement("t"); time;
            time              = time->NextSiblingElement("t")) {
            double openingTime = xmltof(time->Attribute("t"), -1.);
            if(openingTime < 0) {
                throw ParsingError(fmt::format("schedule times {:d}: t has no proper input", id));
            }
            if(resetDoor) {
                timeReset.push_back(openingTime);
            } else {
                timeOpen.push_back(openingTime);
            }

            timeClose.push_back(openingTime + closingTime);
        }

        for(auto door : groupDoor[id]) {
            for(auto time : timeOpen) {
                events.push_back(
                    {std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::duration<double>(time)),
                     door,
                     DoorEvent::Type::OPEN});
            }

            for(auto time : timeClose) {
                events.push_back(
                    {std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::duration<double>(time)),
                     door,
                     DoorEvent::Type::CLOSE});
            }

            for(auto time : timeReset) {
                events.push_back(
                    {std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::duration<double>(time)),
                     door,
                     DoorEvent::Type::RESET});
            }
        }
    }
    return events;
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
