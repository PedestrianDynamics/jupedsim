// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <GenericAgent.hpp>

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

struct AgentIdIterator {
    using AgentIds = std::vector<GenericAgent::ID>;
    AgentIds ids;
    AgentIds::const_iterator iter;

    AgentIdIterator(AgentIds&& ids_) : ids(std::move(ids_)) { iter = std::begin(ids); }
};
