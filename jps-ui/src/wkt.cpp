// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "wkt.hpp"
#include "aabb.hpp"

#include <geos_c.h>
#include <glm/glm.hpp>

#include <fstream>

#include <CGAL/partition_2.h>
#include <exception>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>
namespace GeosHelper
{
unsigned int GEOSCoordSeqGetDimensions(const GEOSCoordSequence* seq)
{
    unsigned int val;
    if(GEOSCoordSeq_getDimensions(seq, &val) == 0) {
        throw std::runtime_error("Error reading dimension");
    }
    return val;
}

unsigned int GEOSCoordSeqGetSize(const GEOSCoordSequence* seq)
{
    unsigned int val;
    if(GEOSCoordSeq_getSize(seq, &val) == 0) {
        throw std::runtime_error("Error reading size");
    }
    return val;
}

template <typename T>
auto GEOSCoordSeqGetXY(const GEOSCoordSequence* seq, unsigned int index) -> T
{
    double x{};
    double y{};
    if(GEOSCoordSeq_getXY(seq, index, &x, &y) == 0) {
        throw std::runtime_error("Error reading coordinate");
    };
    return {x, y};
}
} // namespace GeosHelper

void add_linear_ring(
    const GEOSGeometry* g,
    std::tuple<std::vector<glm::vec2>, std::vector<unsigned int>, std::vector<unsigned int>>& out)
{
}

void DrawableGEOS::partition_polygon(const GEOSGeometry* g)
{
    assert(GEOSGeomTypeId(g) == GEOS_POLYGON);
    auto out = std::make_tuple(
        std::vector<glm::vec2>{}, std::vector<unsigned int>{}, std::vector<unsigned int>{});
    unsigned int vertex_index{};

    const auto ring = GEOSGetExteriorRing(g);
    const auto seq = GEOSGeom_getCoordSeq(ring);
    assert(GeosHelper::GEOSCoordSeqGetDimensions(seq) == 2);
    const auto coord_count = GeosHelper::GEOSCoordSeqGetSize(seq);
    assert(coord_count >= 4);
    std::get<0>(out).push_back(GeosHelper::GEOSCoordSeqGetXY<glm::vec2>(seq, 0));
    CDT::Vertex_handle vh = cdt.insert(GeosHelper::GEOSCoordSeqGetXY<Point>(seq, 0));
    vh->info() = vertex_index++;
    CDT::Vertex_handle vh_prev = vh;
    CDT::Vertex_handle first = vh;
    for(unsigned int index = 1; index < coord_count - 1; ++index) {
        std::get<0>(out).push_back(GeosHelper::GEOSCoordSeqGetXY<glm::vec2>(seq, index));
        CDT::Vertex_handle vh = cdt.insert(GeosHelper::GEOSCoordSeqGetXY<Point>(seq, index));
        vh->info() = vertex_index++;
        cdt.insert_constraint(vh_prev, vh);
        vh_prev = vh;
    }
    cdt.insert_constraint(vh_prev, first);

    const auto count_rings = GEOSGetNumInteriorRings(g);
    for(int index = 0; index < count_rings; ++index) {
        const auto ring = GEOSGetInteriorRingN(g, index);
        const auto seq = GEOSGeom_getCoordSeq(ring);
        assert(GeosHelper::GEOSCoordSeqGetDimensions(seq) == 2);
        const auto coord_count = GeosHelper::GEOSCoordSeqGetSize(seq);
        assert(coord_count >= 4);
        std::get<0>(out).push_back(GeosHelper::GEOSCoordSeqGetXY<glm::vec2>(seq, 0));
        CDT::Vertex_handle vh = cdt.insert(GeosHelper::GEOSCoordSeqGetXY<Point>(seq, 0));
        vh->info() = vertex_index++;
        CDT::Vertex_handle vh_prev = vh;
        CDT::Vertex_handle first = vh;
        for(unsigned int index = 1; index < coord_count - 1; ++index) {
            std::get<0>(out).push_back(GeosHelper::GEOSCoordSeqGetXY<glm::vec2>(seq, index));
            CDT::Vertex_handle vh = cdt.insert(GeosHelper::GEOSCoordSeqGetXY<Point>(seq, index));
            vh->info() = vertex_index++;
            cdt.insert_constraint(vh_prev, vh);
            vh_prev = vh;
        }
        cdt.insert_constraint(vh_prev, first);
    }

    CGAL::mark_domain_in_triangulation(cdt);
};

DrawableGEOS::DrawableGEOS(const GEOSGeometry* geo) : bounds(initBounds(geo))
{
    auto read_sequence = [](auto seq) {
        unsigned int dim{};
        GEOSCoordSeq_getDimensions(seq, &dim);
        if(dim != 2) {
            throw std::runtime_error("Can only work with 2D coordinates");
        }
        unsigned int size{};
        GEOSCoordSeq_getSize(seq, &size);
        std::vector<glm::vec2> sequence_data{};
        sequence_data.reserve(size);
        double x{};
        double y{};
        for(unsigned int index = 0; index < size; ++index) {
            GEOSCoordSeq_getXY(seq, index, &x, &y);
            sequence_data.emplace_back(x, y);
        }
        return sequence_data;
    };
    std::vector<const GEOSGeometry*> stack{geo};

    while(!stack.empty()) {
        auto g = stack.back();
        stack.pop_back();
        switch(GEOSGeomTypeId(g)) {
            case GEOS_POINT:
                // We do not draw points atm.
                break;
            case GEOS_LINESTRING:
            case GEOS_LINEARRING:
                sequences.emplace_back(read_sequence(GEOSGeom_getCoordSeq(g)));
                break;
            case GEOS_POLYGON: {
                stack.push_back(GEOSGetExteriorRing(g));
                const auto count = GEOSGetNumInteriorRings(g);
                for(int index = 0; index < count; ++index) {
                    stack.push_back(GEOSGetInteriorRingN(g, index));
                }
                partition_polygon(g);
                break;
            }
            case GEOS_MULTIPOINT:
                // We do not draw points atm.
                break;
            case GEOS_MULTILINESTRING:
            case GEOS_MULTIPOLYGON:
            case GEOS_GEOMETRYCOLLECTION: {
                const auto count = GEOSGetNumGeometries(g);
                for(int index = 0; index < count; ++index) {
                    stack.push_back(GEOSGetGeometryN(g, index));
                }
                break;
            }
            default:
                throw std::runtime_error("Internal error");
        }
    }
}

DrawableGEOS::~DrawableGEOS()
{
}

const GEOSGeometry* read_wkt(const std::string& data)
{
    GEOSWKTReader* reader = GEOSWKTReader_create();
    GEOSGeometry* geom_a = GEOSWKTReader_read(reader, data.c_str());
    GEOSWKTReader_destroy(reader);
    return geom_a;
}

const GEOSGeometry* read_wkt(const std::filesystem::path& file)
{
    std::ifstream in(file);
    const std::string wkt((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    return read_wkt(wkt);
}

AABB DrawableGEOS::initBounds(const GEOSGeometry* geo)
{
    double xmin, ymin, xmax, ymax;
    GEOSGeom_getExtent(geo, &xmin, &ymin, &xmax, &ymax);
    return AABB{{xmin, ymin}, {xmax, ymax}};
}
