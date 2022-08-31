#pragma once

#include <Pedestrian.hpp>
#include <Simulation.hpp>
#include <memory>

class AgentIterator
{
private:
    using Container = std::vector<std::unique_ptr<Pedestrian>>;
    const Container& container{};
    typename Container::const_iterator iter{};

public:
    AgentIterator(const std::vector<std::unique_ptr<Pedestrian>>& container)
        : container(container), iter(std::begin(container))
    {
    }
    ~AgentIterator() = default;

    const Pedestrian* Next()
    {
        if(iter == std::end(container)) {
            return nullptr;
        }
        return iter++->get();
    }
};
