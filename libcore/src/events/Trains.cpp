#include "Trains.h"

#include "general/Logger.h"

void Trains::AddTrainType(
    std::string name,
    int maxAgents,
    float length,
    std::vector<Transition> doors)
{
    _trainTypes.emplace(name, TrainType{name, maxAgents, length, doors});
}

void Trains::AddTrainType(TrainType trainType)
{
    _trainTypes.emplace(trainType._type, trainType);
}

std::optional<TrainType> Trains::GetTrainType(std::string name) const
{
    if(_trainTypes.count(name) > 0) {
        return _trainTypes.at(name);
    } else {
        return std::nullopt;
    }
}
const std::map<std::string, TrainType> & Trains::GetTrainTypes() const
{
    return _trainTypes;
}
