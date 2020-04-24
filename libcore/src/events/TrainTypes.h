#pragma once

#include "geometry/Transition.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

struct TrainType {
    std::string _type;
    int _maxAgents;
    float _length;
    std::vector<Transition> _doors;
};

class TrainTypes
{
private:
    std::map<std::string, TrainType> _trainTypes;

public:
    TrainTypes()                   = default;
    ~TrainTypes()                  = default;
    TrainTypes(const TrainTypes &) = default;
    TrainTypes(TrainTypes &&)      = default;
    TrainTypes & operator=(const TrainTypes &) = default;
    TrainTypes & operator=(TrainTypes &&) = default;

    void AddTrainType(TrainType trainType);

    std::optional<TrainType> GetTrainType(std::string type) const;

    const std::map<std::string, TrainType> & GetTrainTypes() const;
};
