#pragma once

#include "general/Configuration.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <map>
#include <memory>
#include <tuple>

std::multimap<size_t, std::unique_ptr<Pedestrian>>
CreateAllPedestrians(Configuration * configuration, Building * building);

template <typename T>
std::vector<typename T::mapped_type> extract(T & mm, typename T::key_type key)
{
    auto [from, to] = mm.equal_range(key);
    std::vector<typename T::mapped_type> values{};
    values.reserve(std::distance(from, to));
    std::transform(
        from, to, std::back_inserter(values), [](auto && kv) { return std::move(kv.second); });
    mm.erase(from, to);
    return values;
}
