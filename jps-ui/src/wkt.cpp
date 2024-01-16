// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "wkt.hpp"
#include "aabb.hpp"

#include <OpenGL/OpenGL.h>
#include <geos_c.h>
#include <glm/glm.hpp>

#include <fstream>

#include <CGAL/partition_2.h>
#include <GLFW/glfw3.h>
#include <exception>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/draw_triangulation_2.h>
#include <CGAL/mark_domain_in_triangulation.h>
using K = CGAL::Exact_predicates_exact_constructions_kernel;
using Vb = CGAL::Triangulation_vertex_base_with_info_2<unsigned int, K>;
template <class Gt, class Fb = CGAL::Constrained_triangulation_face_base_2<Gt>>
class MyFace : public Fb
{
    bool in{false};
    typedef Fb Base;
    typedef typename Fb::Triangulation_data_structure TDS;

public:
    using Fb::Fb;
    template <typename TDS2>
    struct Rebind_TDS {
        typedef typename Fb::template Rebind_TDS<TDS2>::Other Fb2;
        typedef MyFace<Gt, Fb2> Other;
    };
    void set_in_domain(bool v) { in = v; }
    bool get_in_domain() const { return in; }
};
using TDS = CGAL::Triangulation_data_structure_2<Vb, MyFace<K>>;
using Itag = CGAL::Exact_predicates_tag;
using CDT = CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>;
using Point = K::Point_2;

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

std::tuple<std::vector<glm::vec2>, std::vector<unsigned int>, std::vector<unsigned int>>
partition_polygon(const GEOSGeometry* g)
{
    assert(GEOSGeomTypeId(g) == GEOS_POLYGON);
    auto out = std::make_tuple(
        std::vector<glm::vec2>{}, std::vector<unsigned int>{}, std::vector<unsigned int>{});
    CDT cdt{};
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

    for(auto&& face : cdt.finite_face_handles()) {
        if(!face->get_in_domain()) {
            continue;
        }
        std::get<1>(out).push_back(face->vertex(0)->info());
        std::get<1>(out).push_back(face->vertex(1)->info());
        std::get<1>(out).push_back(face->vertex(2)->info());
    }

    for(auto&& edge : cdt.finite_edges()) {
        if(edge.first->get_in_domain() || edge.first->neighbor(edge.second)->get_in_domain()) {
            std::get<2>(out).push_back(edge.first->vertex(CDT::cw(edge.second))->info());
            std::get<2>(out).push_back(edge.first->vertex(CDT::ccw(edge.second))->info());
        }
    }

    return out;
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
                triangles.emplace_back(partition_polygon(g));
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

    vaos.resize(sequences.size() + triangles.size() * 2);
    vbos.resize(sequences.size() + triangles.size() * 3);
    glGenVertexArrays(sequences.size() + triangles.size() * 2, vaos.data());
    glGenBuffers(sequences.size() + triangles.size() * 3, vbos.data());

    for(size_t index = 0; index < sequences.size(); ++index) {
        glBindVertexArray(vaos[index]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[index]);
        glBufferData(
            GL_ARRAY_BUFFER,
            sequences[index].size() * sizeof(glm::vec2),
            reinterpret_cast<const GLvoid*>(sequences[index].data()),
            GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
        glEnableVertexAttribArray(0);
    }
    for(size_t index = 0; index < triangles.size(); ++index) {
        glBindVertexArray(vaos[sequences.size() + index]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[sequences.size() + index * 3]);
        glBufferData(
            GL_ARRAY_BUFFER,
            std::get<0>(triangles[index]).size() * sizeof(glm::vec2),
            reinterpret_cast<const GLvoid*>(std::get<0>(triangles[index]).data()),
            GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[sequences.size() + index * 3 + 2]);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            std::get<2>(triangles[index]).size() * sizeof(unsigned int),
            reinterpret_cast<const GLvoid*>(std::get<2>(triangles[index]).data()),
            GL_STATIC_DRAW);
    }
    for(size_t index = 0; index < triangles.size(); ++index) {
        glBindVertexArray(vaos[sequences.size() + triangles.size() + index]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[sequences.size() + index * 3]);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[sequences.size() + index * 3 + 1]);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            std::get<1>(triangles[index]).size() * sizeof(unsigned int),
            reinterpret_cast<const GLvoid*>(std::get<1>(triangles[index]).data()),
            GL_STATIC_DRAW);
    }
}

DrawableGEOS::~DrawableGEOS()
{
}

void DrawableGEOS::Draw(Shader& shader) const
{
    shader.Activate();

    shader.SetUniform("color", glm::vec4(0.0f, 0.0f, 0.75f, 1.0f));
    for(size_t index = 0; index < triangles.size(); ++index) {
        glBindVertexArray(vaos[index + sequences.size() + triangles.size()]);
        glDrawElements(
            GL_TRIANGLES, std::get<1>(triangles[index]).size(), GL_UNSIGNED_INT, nullptr);
    }

    shader.SetUniform("color", glm::vec4(255.0f / 255.0f, 0.0f, 0.0f, 1.0f));
    for(size_t index = 0; index < triangles.size(); ++index) {
        glBindVertexArray(vaos[index + sequences.size()]);
        glDrawElements(GL_LINES, std::get<2>(triangles[index]).size(), GL_UNSIGNED_INT, nullptr);
    }

    shader.SetUniform("color", glm::vec4(255.0f / 255.0f, 1.0f, 1.0f, 1.0f));
    for(size_t index = 0; index < sequences.size(); ++index) {
        glBindVertexArray(vaos[index]);
        glDrawArrays(GL_LINE_STRIP, 0, sequences[index].size());
    }
}

const GEOSGeometry* read_wkt(std::filesystem::path file)
{
    std::ifstream in(file);
    const std::string wkt((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    GEOSWKTReader* reader = GEOSWKTReader_create();
    GEOSGeometry* geom_a = GEOSWKTReader_read(reader, wkt.c_str());
    GEOSWKTReader_destroy(reader);
    return geom_a;
}

AABB DrawableGEOS::initBounds(const GEOSGeometry* geo)
{
    double xmin, ymin, xmax, ymax;
    GEOSGeom_getExtent(geo, &xmin, &ymin, &xmax, &ymax);
    return AABB{{xmin, ymin}, {xmax, ymax}};
}
