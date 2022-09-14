/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ConvexPolygon.hpp"
#include "UniqueID.hpp"

#include <set>
#include <string>
#include <vector>

struct Area {
    using Id = uint32_t;
    Id id;
    std::set<std::string> labels;
    ConvexPolygon polygon;

    Area() = default;
    Area(Area::Id id, std::set<std::string>&& labels, ConvexPolygon&& polygon);
};

using Areas = std::map<Area::Id, Area>;

class AreasBuilder
{
    std::vector<Area> _areas;

public:
    AreasBuilder() = default;
    ~AreasBuilder() = default;
    AreasBuilder(const AreasBuilder& other) = delete;
    AreasBuilder& operator=(const AreasBuilder& other) = delete;
    AreasBuilder(AreasBuilder&& other) = delete;
    AreasBuilder& operator=(AreasBuilder&& other) = delete;

    AreasBuilder& AddArea(
        Area::Id id,
        const std::vector<Point>& lineLoop,
        const std::vector<std::string>& labels);

    Areas Build();
};
