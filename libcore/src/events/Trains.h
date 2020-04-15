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

class Trains
{
private:
    std::map<std::string, TrainType> _trainTypes;

public:
    Trains()               = default;
    ~Trains()              = default;
    Trains(const Trains &) = default;
    Trains(Trains &&)      = default;
    Trains & operator=(const Trains &) = default;
    Trains & operator=(Trains &&) = default;

    /**
     *
     * @param name
     * @param maxAgents
     * @param length
     * @param doors
     */
    void AddTrainType(std::string name, int maxAgents, float length, std::vector<Transition> doors);

    /**
     *
     * @param trainType
     */
    void AddTrainType(TrainType trainType);

    /**
     *
     * @param name
     * @return
     */
    std::optional<TrainType> GetTrainType(std::string name) const;

    const std::map<std::string, TrainType> & GetTrainTypes() const;
};
