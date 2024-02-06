// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <algorithm>
#include <iterator>
#include <optional>

template <typename Container>
std::optional<size_t> IndexInContainer(const Container& c, const typename Container::value_type& v)
{
    size_t idx = 0;
    for(const auto& e : c) {
        if(e == v) {
            return idx;
        }
        ++idx;
    }
    return {};
}

template <typename Container>
bool Contains(const Container& c, const typename Container::value_type& v)
{
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}
