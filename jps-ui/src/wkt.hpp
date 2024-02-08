// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "aabb.hpp"
#include "shader.hpp"

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/draw_triangulation_2.h>
#include <CGAL/mark_domain_in_triangulation.h>

#include <geos_c.h>

#include <array>
#include <filesystem>
#include <memory>
#include <vector>

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

class DrawableGEOS
{
private:
    std::vector<std::vector<glm::vec2>> sequences{};
    AABB bounds{};
    CDT cdt{};

public:
    explicit DrawableGEOS(const GEOSGeometry* geo);
    ~DrawableGEOS();
    const AABB& Bounds() const { return bounds; }
    const CDT& tri() const { return cdt; }

private:
    AABB initBounds(const GEOSGeometry* geo);
    void partition_polygon(const GEOSGeometry* g);
};

const GEOSGeometry* read_wkt(const std::string& data);
const GEOSGeometry* read_wkt(const std::filesystem::path& file);
