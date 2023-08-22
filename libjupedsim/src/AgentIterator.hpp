// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <functional>
#include <memory>

template <typename Model>
class AgentIterator
{
private:
    using Container = std::vector<Model>;
    const Container& container{};
    typename Container::const_iterator iter{};

public:
    AgentIterator(const Container& container_) : container(container_), iter(std::begin(container_))
    {
    }
    ~AgentIterator() = default;

    const Model* Next()
    {
        if(iter == std::end(container)) {
            return nullptr;
        }
        const auto result = &*iter;
        ++iter;
        return result;
    }
};
