#include "TrainTypes.h"

#include "general/Logger.h"

void TrainTypes::AddTrainType(TrainType trainType)
{
    _trainTypes.emplace(trainType._type, trainType);
}

std::optional<TrainType> TrainTypes::GetTrainType(std::string name) const
{
    if(_trainTypes.count(name) > 0) {
        return _trainTypes.at(name);
    } else {
        return std::nullopt;
    }
}
const std::map<std::string, TrainType> & TrainTypes::GetTrainTypes() const
{
    return _trainTypes;
}
