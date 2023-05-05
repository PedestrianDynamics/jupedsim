/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <iterator>
#include <optional>
#include <vector>

template <typename T>
std::optional<size_t> IndexInVector(const std::vector<T>& vec, const T& item)
{
    const auto find_iter = std::find(std::begin(vec), std::end(vec), item);
    if(find_iter == std::end(vec)) {
        return {};
    }
    return std::distance(std::begin(vec), find_iter);
}
