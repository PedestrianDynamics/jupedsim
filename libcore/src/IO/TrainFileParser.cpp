#include "TrainFileParser.h"

#include "general/Logger.h"

std::map<int, TrainTimeTable>
TrainFileParser::ParseTrainTimetable(const fs::path & trainTimeTableFile)
{
    std::map<int, TrainTimeTable> trainTimeTable;
    TiXmlDocument docTTT(trainTimeTableFile.string());
    if(!docTTT.LoadFile()) {
        LOG_ERROR("{}", docTTT.ErrorDesc());
        LOG_ERROR("Could not parse the train timetable file.");
        return trainTimeTable;
    }
    TiXmlElement * xTTT = docTTT.RootElement();
    if(!xTTT) {
        LOG_ERROR("Root element does not exist in TTT file.");
        return trainTimeTable;
    }
    if(xTTT->ValueStr() != "train_time_table") {
        LOG_ERROR("Parsing train timetable file. Root element value is not 'train_time_table'.");
        return trainTimeTable;
    }
    for(TiXmlElement * e = xTTT->FirstChildElement("train"); e;
        e                = e->NextSiblingElement("train")) {
        auto TTT = ParseTrainTimeTableNode(e);

        if(TTT) {
            if(trainTimeTable.find(TTT.value().id) != std::end(trainTimeTable)) {
                LOG_WARNING("Duplicate id for train time table found [{}]", TTT.value().id);
                continue;
            }
            trainTimeTable.emplace(TTT.value().id, TTT.value());
        }
    }
    return trainTimeTable;
}

std::optional<TrainTimeTable> TrainFileParser::ParseTrainTimeTableNode(TiXmlElement * node)
{
    //TODO check these values for correctness e.g. arrival < departure
    LOG_INFO("Loading train time table NODE");
    std::string caption = xmltoa(node->Attribute("caption"), "-1");
    int id              = xmltoi(node->Attribute("id"), -1);
    std::string type    = xmltoa(node->Attribute("type"), "-1");
    int room_id         = xmltoi(node->Attribute("room_id"), -1);
    int subroom_id      = xmltoi(node->Attribute("subroom_id"), -1);
    int platform_id     = xmltoi(node->Attribute("platform_id"), -1);
    float track_start_x = xmltof(node->Attribute("track_start_x"), -1);
    float track_start_y = xmltof(node->Attribute("track_start_y"), -1);
    float track_end_x   = xmltof(node->Attribute("track_end_x"), -1);
    float track_end_y   = xmltof(node->Attribute("track_end_y"), -1);

    float train_start_x = xmltof(node->Attribute("train_start_x"), -1);
    float train_start_y = xmltof(node->Attribute("train_start_y"), -1);
    float train_end_x   = xmltof(node->Attribute("train_end_x"), -1);
    float train_end_y   = xmltof(node->Attribute("train_end_y"), -1);

    float arrival_time   = xmltof(node->Attribute("arrival_time"), -1);
    float departure_time = xmltof(node->Attribute("departure_time"), -1);
    LOG_INFO("Train time table:");
    LOG_INFO("ID: {}", id);
    LOG_INFO("Type: {}", type);
    LOG_INFO("room_id: {}", room_id);
    LOG_INFO("subroom_id: {}", subroom_id);
    LOG_INFO("platform_id: {}", platform_id);
    LOG_INFO("track_start: [{:.2f}, {:.2f}]", track_start_x, track_start_y);
    LOG_INFO("track_end: [{:.2f}, {:.2f}]", track_end_x, track_end_y);
    LOG_INFO("arrival_time: {:.2f}", arrival_time);
    LOG_INFO("departure_time: {:.2f}", departure_time);
    Point track_start(track_start_x, track_start_y);
    Point track_end(track_end_x, track_end_y);
    Point train_start(train_start_x, train_start_y);
    Point train_end(train_end_x, train_end_y);
    return TrainTimeTable{
        id,
        type,
        room_id,
        subroom_id,
        arrival_time,
        departure_time,
        track_start,
        track_end,
        train_start,
        train_end,
        platform_id,
        false,
        false,
    };
}

std::map<std::string, TrainType> TrainFileParser::ParseTrainType(const fs::path & trainTypeFile)
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
        if(trainType) {
            if(trainTypes.find(trainType.value().type) != std::end(trainTypes)) {
                LOG_WARNING("Duplicate type for train found [{}]", trainType.value().type);
                continue;
            }
            trainTypes.insert(std::make_pair(trainType.value().type, trainType.value()));
        }
    }
    return trainTypes;
}

std::optional<TrainType> TrainFileParser::ParseTrainTypeNode(TiXmlElement * node)
{
    LOG_INFO("Loading train type");
    std::string type = xmltoa(node->Attribute("type"), "-1");
    int agents_max   = xmltoi(node->Attribute("agents_max"), -1);
    float length     = xmltof(node->Attribute("length"), -1);
    Transition t;
    std::vector<Transition> doors;

    for(TiXmlElement * xDoor = node->FirstChildElement("door"); xDoor;
        xDoor                = xDoor->NextSiblingElement("door")) {
        int D_id = xmltoi(xDoor->Attribute("id"), -1);
        float x1 = xmltof(xDoor->FirstChildElement("vertex")->Attribute("px"), -1);
        float y1 = xmltof(xDoor->FirstChildElement("vertex")->Attribute("py"), -1);
        float x2 = xmltof(xDoor->LastChild("vertex")->ToElement()->Attribute("px"), -1);
        float y2 = xmltof(xDoor->LastChild("vertex")->ToElement()->Attribute("py"), -1);
        Point start(x1, y1);
        Point end(x2, y2);
        float outflow = xmltof(xDoor->Attribute("outflow"), -1);
        float dn      = xmltoi(xDoor->Attribute("dn"), -1);
        t.SetID(D_id);
        t.SetCaption(type + std::to_string(D_id));
        t.SetPoint1(start);
        t.SetPoint2(end);
        t.SetOutflowRate(outflow);
        t.SetDN(dn);
        doors.push_back(t);
    }
    LOG_INFO("type: {}", type);
    LOG_INFO("capacity: {}", agents_max);
    LOG_INFO("number of doors: {}", doors.size());
    for(auto d : doors) {
        LOG_INFO(
            "Door ({}): {} | {}", d.GetID(), d.GetPoint1().toString(), d.GetPoint2().toString());
    }

    return TrainType{
        type,
        agents_max,
        length,
        doors,
    };
}
