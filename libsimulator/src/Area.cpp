#include "Area.hpp"
#include "ConvexPolygon.hpp"

#include <fmt/format.h>

#include <stdexcept>

Area::Area(Area::Id id, std::set<std::string>&& labels, ConvexPolygon&& polygon)
    : id(id), labels(std::move(labels)), polygon(std::move(polygon))
{
}

AreasBuilder& AreasBuilder::AddArea(
    Area::Id id,
    const std::vector<Point>& lineLoop,
    const std::vector<std::string>& labels)
{
    _areas.emplace_back(
        id, std::set<std::string>(labels.begin(), labels.end()), ConvexPolygon(lineLoop));
    return *this;
}

Areas AreasBuilder::Build()
{
    Areas areas{};
    for(auto& area : _areas) {
        if(areas.count(area.id) != 0) {
            throw std::runtime_error(fmt::format("Duplicated area id found, id: {}", area.id));
        } else {
            areas.insert(std::make_pair(area.id, std::move(area)));
        }
    }
    return areas;
}
