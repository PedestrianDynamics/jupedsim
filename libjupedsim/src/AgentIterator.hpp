#pragma once

#include <Agent.hpp>
#include <Simulation.hpp>
#include <memory>

class AgentIterator
{
private:
    using Container = std::vector<std::unique_ptr<Agent>>;
    const Container& container{};
    typename Container::const_iterator iter{};

public:
    AgentIterator(const std::vector<std::unique_ptr<Agent>>& container)
        : container(container), iter(std::begin(container))
    {
    }
    ~AgentIterator() = default;

    const Agent* Next()
    {
        if(iter == std::end(container)) {
            return nullptr;
        }
        return iter++->get();
    }
};
