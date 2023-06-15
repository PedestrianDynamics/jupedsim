/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <functional>
#include <memory>

template <typename Model, typename ApiAgentType>
class AgentIterator
{
private:
    using Container = std::vector<Model>;
    const Container& container{};
    typename Container::const_iterator iter{};
    using MapFunc = std::function<ApiAgentType(Model)>;
    MapFunc mapFunc{};
    ApiAgentType item{};

public:
    AgentIterator(const Container& container_, MapFunc mapFunc_)
        : container(container_), iter(std::begin(container_)), mapFunc(mapFunc_)
    {
    }
    ~AgentIterator() = default;

    const ApiAgentType* Next()
    {
        if(iter == std::end(container)) {
            return nullptr;
        }
        item = mapFunc(*iter);
        ++iter;
        return &item;
    }
};
