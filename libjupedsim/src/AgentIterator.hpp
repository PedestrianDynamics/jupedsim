// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <functional>
#include <memory>

template <typename Model>
class AgentIterator
{
private:
    using Container = std::vector<Model>;
    Container& container{};
    typename Container::iterator iter{};

public:
    AgentIterator(Container& container_) : container(container_), iter(std::begin(container_)) {}
    ~AgentIterator() = default;

    Model* Next()
    {
        if(iter == std::end(container)) {
            return nullptr;
        }
        auto result = &*iter;
        ++iter;
        return result;
    }
};
