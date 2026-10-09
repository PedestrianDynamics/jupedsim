// SPDX-License-Identifier: LGPL-3.0-or-later
#include "FloorfieldRoutingEngine.hpp"

#include "Geometry/Geometry.hpp"
#include "SimulationError.hpp"

#include <boost/range/iterator_range.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace ff = jupedsim::floorfield;

namespace
{
constexpr double cell_size = 0.2;
/// Walls slow agents down within this distance, which keeps routes off them.
constexpr double wall_influence_radius = 0.5;

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

FloorfieldRoutingEngine::FloorfieldRoutingEngine(const Geometry& geometry)
    : _field(checked([&geometry] {
        return ff::new_multi_region_floorfield(
            to_floorfield(geometry), cell_size, wall_influence_radius);
    }))
{
}

Destination FloorfieldRoutingEngine::register_destination(const std::vector<AreaPiece>& pieces)
{
    std::vector<ff::AreaPiece> ff_pieces{};
    ff_pieces.reserve(pieces.size());
    for(const auto& piece : pieces) {
        ff_pieces.push_back({piece.region, to_ring(piece.polygon)});
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
