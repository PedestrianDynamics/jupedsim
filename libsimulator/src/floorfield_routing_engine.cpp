// SPDX-License-Identifier: LGPL-3.0-or-later
#include "floorfield_routing_engine.hpp"

#include "geometry/geometry.hpp"
#include "simulation_error.hpp"

#include <boost/range/iterator_range.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace ff = jupedsim::floorfield;

namespace
{

template <typename Call>
auto checked(Call&& call)
{
    try {
        return call();
    } catch(const rust::Error& e) {
        throw SimulationError("Floor field: {}", e.what());
    }
}

ff::Ring to_ring(const Poly& poly)
{
    ff::Ring ring{};
    for(const auto& p : poly.container()) {
        ring.points.push_back({CGAL::to_double(p.x()), CGAL::to_double(p.y())});
    }
    return ring;
}

SpeedGrid to_speed_grid(const ff::GridInfo& grid, const rust::Vec<double>& values)
{
    return SpeedGrid{
        {grid.origin_x, grid.origin_y},
        grid.width,
        grid.height,
        grid.cell_size,
        std::vector<double>(values.begin(), values.end())};
}

rust::Slice<const double> to_slice(const std::vector<double>& values)
{
    return {values.data(), values.size()};
}

/// The region graph of @p geometry in the bridge's layout, which mirrors RegionGraph2D: regions
/// with their rings, and every directed edge as a seam through ring edge (ring, index) of its
/// source. The floor field checks it; this only copies.
ff::RegionGraph to_floorfield(const Geometry& geometry)
{
    const auto* graph = geometry.region_graph_2d();
    if(graph == nullptr) {
        throw SimulationError("Routing needs a geometry built from regions, not a raw mesh.");
    }
    const auto& g = *graph;
    ff::RegionGraph out{};
    for(const auto r : boost::make_iterator_range(boost::vertices(g))) {
        const PolyWithHoles& poly = g[r];
        ff::Region region{};
        region.outer = to_ring(poly.outer_boundary());
        for(const auto& hole : poly.holes()) {
            region.holes.push_back(to_ring(hole));
        }
        out.regions.push_back(std::move(region));
    }
    for(const auto e : boost::make_iterator_range(boost::edges(g))) {
        out.seams.push_back(
            {boost::source(e, g),
             boost::target(e, g),
             static_cast<std::uint32_t>(g[e].ring),
             static_cast<std::uint32_t>(g[e].index)});
    }
    return out;
}
} // namespace

FloorfieldRoutingEngine::FloorfieldRoutingEngine(
    const Geometry& geometry,
    FloorfieldOptions options)
    : _field(checked([&geometry, options] {
        return ff::new_multi_region_floorfield(
            to_floorfield(geometry), options.cell_size, options.wall_influence_radius);
    }))
{
}

Destination FloorfieldRoutingEngine::register_destination(const std::vector<AreaPiece>& pieces)
{
    std::vector<ff::AreaPiece> ff_pieces{};
    ff_pieces.reserve(pieces.size());
    for(const auto& piece : pieces) {
        ff_pieces.push_back({piece.region_id, to_ring(piece.polygon)});
    }
    const auto id = checked([&] {
        return _field->add_area_destination(
            rust::Slice<const ff::AreaPiece>{ff_pieces.data(), ff_pieces.size()});
    });
    return Destination{*this, id};
}

bool FloorfieldRoutingEngine::contains(const Location& where, std::size_t id)
{
    const auto xy = where.xy();
    return checked([&] { return _field->travel_time(where.region(), {xy.x, xy.y}, id); }) == 0.0;
}

Point FloorfieldRoutingEngine::get_orientation(const Location& from, std::size_t id)
{
    const auto xy = from.xy();
    const auto direction =
        checked([&] { return _field->region_direction(from.region(), {xy.x, xy.y}, id); });
    return {direction.x, direction.y};
}

TravelTimeGrid FloorfieldRoutingEngine::travel_times(std::size_t region, std::size_t id)
{
    return checked([&] {
        const auto grid = _field->region_grid(region);
        const auto values = _field->region_travel_times(region, id);
        const auto directions = _field->region_directions(region, id);
        std::vector<Point> points{};
        points.reserve(directions.size());
        for(const auto& d : directions) {
            points.emplace_back(d.x, d.y);
        }
        return TravelTimeGrid{
            {grid.origin_x, grid.origin_y},
            grid.width,
            grid.height,
            grid.cell_size,
            std::vector<double>(values.begin(), values.end()),
            std::move(points)};
    });
}

SpeedGrid FloorfieldRoutingEngine::default_speed(std::size_t region) const
{
    return checked([&] {
        return to_speed_grid(_field->region_grid(region), _field->region_default_speed(region));
    });
}

void FloorfieldRoutingEngine::set_default_speed(
    std::size_t region,
    const std::vector<double>& values)
{
    checked([&] { _field->set_region_default_speed(region, to_slice(values)); });
}

SpeedGrid FloorfieldRoutingEngine::speed(std::size_t region, std::size_t id) const
{
    return checked([&] {
        return to_speed_grid(_field->region_grid(region), _field->region_speed(region, id));
    });
}

bool FloorfieldRoutingEngine::has_speed_override(std::size_t region, std::size_t id) const
{
    return checked([&] { return _field->region_has_speed_override(region, id); });
}

void FloorfieldRoutingEngine::set_speed_override(
    std::size_t region,
    std::size_t id,
    const std::vector<double>& values)
{
    checked([&] { _field->set_region_speed_override(region, id, to_slice(values)); });
}

void FloorfieldRoutingEngine::clear_speed_override(std::size_t region, std::size_t id)
{
    checked([&] { _field->clear_region_speed_override(region, id); });
}
