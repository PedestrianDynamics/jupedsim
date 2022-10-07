/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <GenericAgent.hpp>
#include <memory>

class AgentIterator
{
private:
    using Container = std::vector<GenericAgent>;
    const Container& container{};
    typename Container::const_iterator iter{};

public:
    AgentIterator(const std::vector<GenericAgent>& container)
        : container(container), iter(std::begin(container))
    {
    }
    ~AgentIterator() = default;

    const GenericAgent* Next()
    {
        if(iter == std::end(container)) {
            return nullptr;
        }
        return &*(iter++);
    }
};
