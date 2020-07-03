#include "TrainFileParser.h"

#include <Logger.h>

void TrainFileParser::ParseTrainTimeTable(
    EventManager & eventManager,
    Building & building,
    const std::map<std::string, TrainType> & trainTypes,
    const fs::path & trainTimeTableFile)
{
    TiXmlDocument docTTT(trainTimeTableFile.string());
    if(!docTTT.LoadFile()) {
        LOG_ERROR("{}", docTTT.ErrorDesc());
        LOG_ERROR("Could not parse the train timetable file.");
    }
    TiXmlElement * xTTT = docTTT.RootElement();
    if(!xTTT) {
        LOG_ERROR("Root element does not exist in TTT file.");
    }
    if(xTTT->ValueStr() != "train_time_table") {
        LOG_ERROR("Parsing train timetable file. Root element value is not 'train_time_table'.");
    }

    for(TiXmlElement * e = xTTT->FirstChildElement("train"); e;
        e                = e->NextSiblingElement("train")) {
        auto trainInfo = ParseTrainTimeTableNode(e, building, trainTypes);
        if(trainInfo.has_value()) {
            auto trainTimes = ParseTrainTimeTableTimes(e, trainInfo.value().trainID);
            if(trainTimes.has_value()) {
                auto [arrivalTime, departureTime] = trainTimes.value();

                LOG_INFO("Loading train time table NODE");
                LOG_INFO("ID: {}", trainInfo.value().trainID);
                LOG_INFO("Type: {}", trainInfo.value().trainType._type);
                LOG_INFO("track_id: {}", trainInfo.value().trackID);
                LOG_INFO("train_offset: {}", trainInfo.value().trainStartOffset);
                LOG_INFO("reversed: {}", trainInfo.value().reversed);
                LOG_INFO("arrival_time: {}", arrivalTime);
                LOG_INFO("departure_time: {}", departureTime);

                const auto & trainType = trainTypes.at(trainInfo.value().trainType._type);
                const TrainEventInfo eventInfo{
                    &building,
                    trainInfo.value().trainID,
                    trainInfo.value().trackID,
                    trainInfo.value().trainType,
                    trainInfo.value().trainStartOffset,
                    trainInfo.value().reversed};

                // Arriving train
                eventManager.AddEvent(std::make_unique<TrainArrivalEvent>(arrivalTime, eventInfo));

                // Departing train
                eventManager.AddEvent(
                    std::make_unique<TrainDepartureEvent>(departureTime, eventInfo));

                building.AddTrain(trainInfo.value().trainID, trainType);
            }
        }
    }
}

std::optional<TrainEventInfo> TrainFileParser::ParseTrainTimeTableNode(
    TiXmlElement * node,
    Building & building,
    const std::map<std::string, TrainType> & trainTypes)
{
    // Read ID and check if correct value
    int id = std::numeric_limits<int>::min();
    if(const char * attribute = node->Attribute("id"); attribute) {
        if(int value = xmltoi(attribute, std::numeric_limits<int>::min());
           value > -1 && attribute == std::to_string(value)) {
            id = value;
        } else {
            LOG_WARNING("id {}: input for id should be positive integer. Skip entry", attribute);
            return std::nullopt;
        }
    } else {
        LOG_WARNING("no ID given. Skip entry.");
        return std::nullopt;
    }

    // Read track_id and check if correct value
    int trackID = std::numeric_limits<int>::min();
    if(const char * attribute = node->Attribute("track_id"); attribute) {
        if(int value = xmltoi(attribute, std::numeric_limits<int>::min());
           value > -1 && attribute == std::to_string(value)) {
            trackID = value;
        } else {
            LOG_WARNING(
                "id {}: input for track_id should be positive integer but is {}. Skip entry.",
                id,
                attribute);
            return std::nullopt;
        }
    } else {
        LOG_WARNING("id {}: input for track_id not found. Skip entry.", id);
        return std::nullopt;
    }

    // Read train type and check if correct value
    std::string type;
    if(const char * attribute = node->Attribute("type"); attribute) {
        if(std::string value = xmltoa(attribute, ""); !value.empty()) {
            type = value;
        } else {
            LOG_WARNING("id {}: input for type could not be parsed {}. Skip entry.", id, attribute);
            return std::nullopt;
        }
    } else {
        LOG_WARNING("id {}: input for type not found. Skip entry.", id);
        return std::nullopt;
    }
    // Check if train exists
    if(trainTypes.find(type) == std::end(trainTypes)) {
        LOG_ERROR(
            "Train {:2d}: the train type '{}' could not be found. Please check your "
            "train type "
            "file.",
            id,
            type);
        return std::nullopt;
    }

    // Read train type and check if correct value
    double trainOffset = -std::numeric_limits<double>::infinity();
    if(const char * attribute = node->Attribute("train_offset"); attribute) {
        if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
           value >= 0.) {
            trainOffset = value;
        } else {
            LOG_WARNING(
                "id {}: input for train_offset should be non-negative {}. Skip entry.", id, value);
            return std::nullopt;
        }
    } else {
        LOG_WARNING("id {}: input for train_offset not found. Skip entry.", id);
        return std::nullopt;
    }

    // Read from_end and check if correct value
    bool reversed = false;
    if(const char * attribute = node->Attribute("reversed"); attribute) {
        std::string in = xmltoa(attribute, "false");
        std::transform(in.begin(), in.end(), in.begin(), ::tolower);

        if(in == "false") {
            reversed = false;
        } else if(in == "true") {
            reversed = true;
        } else {
            reversed = false;
            LOG_WARNING(
                "id {}: input for reversed should be true or false, but is {}. Use default: "
                "false.",
                id,
                in);
        }
    } else {
        reversed = false;
    }

    return TrainEventInfo{&building, id, trackID, trainTypes.at(type), trainOffset, reversed};
}

std::optional<std::tuple<double, double>>
TrainFileParser::ParseTrainTimeTableTimes(TiXmlElement * node, int trainID)
{
    // Read arrival_time and check if correct value
    double arrivalTime = -std::numeric_limits<double>::infinity();
    if(const char * attribute = node->Attribute("arrival_time"); attribute) {
        if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
           value >= 0.) {
            arrivalTime = value;
        } else {
            LOG_WARNING(
                "id {}: input for arrival_time should be non-negative {}. Skip entry.",
                trainID,
                value);
            return std::nullopt;
        }
    } else {
        LOG_WARNING("id {}: input for arrival_time not found. Skip entry.", trainID);
        return std::nullopt;
    }

    // Read departure_time and check if correct value
    double departureTime = -std::numeric_limits<double>::infinity();
    if(const char * attribute = node->Attribute("departure_time"); attribute) {
        if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
           value >= 0.) {
            departureTime = value;
        } else {
            LOG_WARNING(
                "id {}: input for departure_time should be non-negative {}. Skip entry.",
                trainID,
                value);
            return std::nullopt;
        }
    } else {
        LOG_WARNING("id {}: input for departure_time not found. Skip entry.", trainID);
    }

    if(arrivalTime >= departureTime) {
        LOG_WARNING(
            "id {}: train departure {} is scheduled before train arrival {}. Skip entry.",
            trainID,
            departureTime,
            arrivalTime);
        return std::nullopt;
    }

    return std::make_tuple(arrivalTime, departureTime);
}


std::map<std::string, TrainType> TrainFileParser::ParseTrainTypes(const fs::path & trainTypeFile)
{
    std::map<std::string, TrainType> trainTypes;
    TiXmlDocument docTT(trainTypeFile.string());
    if(!docTT.LoadFile()) {
        LOG_ERROR("{}", docTT.ErrorDesc());
        LOG_ERROR("Could not parse the train type file.");
        return trainTypes;
    }
    TiXmlElement * xTT = docTT.RootElement();
    if(!xTT) {
        LOG_ERROR("Root element does not exist in TT file.");
        return trainTypes;
    }
    if(xTT->ValueStr() != "train_type") {
        LOG_ERROR("Parsing train type file. Root element value is not 'train_type'.");
        return trainTypes;
    }
    for(TiXmlElement * e = xTT->FirstChildElement("train"); e; e = e->NextSiblingElement("train")) {
        auto trainType = ParseTrainTypeNode(e);
        if(trainType.has_value()) {
            if(trainTypes.find(trainType.value()._type) != std::end(trainTypes)) {
                LOG_WARNING("Duplicate type for train found [{}]", trainType.value()._type);
                continue;
            }
            trainTypes.emplace(trainType.value()._type, trainType.value());
        }
    }
    return trainTypes;
}

std::optional<TrainType> TrainFileParser::ParseTrainTypeNode(TiXmlElement * node)
{
    LOG_INFO("Loading train type");

    std::string type = xmltoa(node->Attribute("type"), "NO_TYPE");
    if(type == "NO_TYPE") {
        LOG_WARNING("No train type name given. Use 'NO_TYPE' instead.");
    }
    LOG_INFO("type: {}", type);

    int agents_max = xmltoi(node->Attribute("agents_max"), std::numeric_limits<int>::max());
    if(agents_max == std::numeric_limits<int>::max()) {
        LOG_WARNING("No agents_max given. Set to default: {}.", agents_max);
    }
    LOG_INFO("max Agents: {}", agents_max);

    // Read length and check if correct value
    double length = -std::numeric_limits<double>::infinity();
    if(const char * attribute = node->Attribute("length"); attribute) {
        if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
           value >= 0.) {
            length = value;
        } else {
            LOG_WARNING("{}: input for length should be non-negative {}. Skip entry.", type, value);
            return std::nullopt;
        }
    } else {
        LOG_WARNING("{}: input for length not found. Skip entry.", type);
        return std::nullopt;
    }
    LOG_INFO("length: {}", length);


    std::vector<TrainDoor> doors;
    for(TiXmlElement * xDoor = node->FirstChildElement("door"); xDoor != nullptr;
        xDoor                = xDoor->NextSiblingElement("door")) {
        // Read distance and check if correct value
        double distance = -std::numeric_limits<double>::infinity();
        if(const char * attribute = xDoor->Attribute("distance"); attribute) {
            if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
               value >= 0.) {
                distance = value;
            } else {
                LOG_WARNING(
                    "{}: input for distance should be non-negative {}. Skip entry.", type, value);
                continue;
            }
        } else {
            LOG_WARNING("{}: input for distance not found. Skip entry.", type);
            continue;
        }

        // Read width and check if correct value
        double width = -std::numeric_limits<double>::infinity();
        if(const char * attribute = xDoor->Attribute("width"); attribute) {
            if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
               value > 0.) {
                width = value;
            } else {
                LOG_WARNING(
                    "{}: input for width should be non-negative {}. Skip entry.", type, value);
                continue;
            }
        } else {
            LOG_WARNING("{}: input for width not found. Skip entry.", type);
            continue;
        }

        // Read flow and check if correct value
        double flow = -std::numeric_limits<double>::infinity();
        if(const char * attribute = xDoor->Attribute("flow"); attribute) {
            if(double value = xmltof(attribute, -std::numeric_limits<double>::infinity());
               value > 0.) {
                flow = value;
            } else {
                LOG_WARNING(
                    "{}: input for flow should be >0 but is {:5.2}. Skip entry.", type, value);
                continue;
            }
        }

        doors.emplace_back(TrainDoor{distance, width, flow});
    }

    if(doors.empty()) {
        LOG_ERROR("Train {}: no doors given. Train will be ignored.", type);
        return std::nullopt;
    }

    LOG_INFO("number of doors: {}", doors.size());
    for(const auto & d : doors) {
        LOG_INFO(
            "Door:\tdistance: {:5.2f}\twidth: {:5.2f}\toutflow: {:5.2f}",
            d._distance,
            d._width,
            d._flow);
    }

    return TrainType{type, agents_max, length, doors};
}
