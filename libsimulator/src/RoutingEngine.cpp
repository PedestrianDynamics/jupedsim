// SPDX-License-Identifier: LGPL-3.0-or-later
#include "RoutingEngine.hpp"

#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Mesh.hpp"
#include "SimulationError.hpp"

#include <CGAL/Distance_3/Ray_3_Line_3.h>
#include <CGAL/IO/OFF/Scanner_OFF.h>
#include <CGAL/number_utils.h>
#include <algorithm>
#include <glm/geometric.hpp>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_mesh_area_criteria_2.h>
#include <CGAL/Delaunay_mesh_size_criteria_2.h>
#include <CGAL/Delaunay_mesher_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Triangulation_conformer_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/draw_triangulation_2.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <CGAL/point_generators_2.h> // for to_double()
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using Kernel = CGAL::Simple_cartesian<double>;
using Point_2 = Kernel::Point_2;


// for debuging
void export_triangles(const CDT& cdt, const std::string& filename)
{
    std::ofstream out(filename);
    if(!out) {
        std::cerr << "Cannot open file " << filename << " for writing." << std::endl;
        return;
    }

    for(auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit) {
        if(!fit->get_in_domain())
            continue;

        for(int i = 0; i < 3; ++i) {
            const auto& p = fit->vertex(i)->point();
            out << CGAL::to_double(p.x()) << "," << CGAL::to_double(p.y()) << " ";
        }
        out << "\n";
    }

    std::cout << "Exported triangles to " << filename << std::endl;
}

double splitLongConstraints(CDT& cdt, double threshold)
{
    std::set<Point_2> points_to_add;

    for(auto eit = cdt.finite_edges_begin(); eit != cdt.finite_edges_end(); ++eit) {
        if(!cdt.is_constrained(*eit))
            continue;

        auto face = eit->first;
        int index = eit->second;

        Point_2 p1 = face->vertex((index + 1) % 3)->point();
        Point_2 p2 = face->vertex((index + 2) % 3)->point();

        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double length_sq = dx * dx + dy * dy;

        if(length_sq > threshold * threshold) {
            Point_2 midpoint((p1.x() + p2.x()) / 2.0, (p1.y() + p2.y()) / 2.0);
            points_to_add.insert(midpoint);
        }
    }

    for(const auto& point : points_to_add) {
        try {
            cdt.insert(point);
        } catch(...) {
            std::cerr << "Error inserting point: " << point << std::endl;
        }
    }

    return points_to_add.size();
}

// Add centroids of large triangles
double addCentroidsOfLargeTriangles(CDT& cdt, double area_threshold)
{
    std::set<Point_2> centroids_to_add;

    for(auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit) {
        Point_2 p1 = fit->vertex(0)->point();
        Point_2 p2 = fit->vertex(1)->point();
        Point_2 p3 = fit->vertex(2)->point();

        // Compute signed area using the cross product
        double area =
            std::abs(
                (p2.x() - p1.x()) * (p3.y() - p1.y()) - (p3.x() - p1.x()) * (p2.y() - p1.y())) /
            2.0;

        if(area > area_threshold) {
            Point_2 centroid((p1.x() + p2.x() + p3.x()) / 3.0, (p1.y() + p2.y() + p3.y()) / 3.0);
            centroids_to_add.insert(centroid);
        }
    }

    for(const auto& centroid : centroids_to_add) {
        try {
            cdt.insert(centroid);
        } catch(const std::exception& e) {
            std::cerr << "Exception inserting centroid " << centroid << ": " << e.what()
                      << std::endl;
        }
    }

    return centroids_to_add.size();
}

void improveTriangulation(CDT& cdt)
{
    std::vector<Point_2> points_to_add;
    float area_threshold = 16.0;
    float length_threshold = 4.0;
    int res1, res2;
    int max_iterations = cdt.number_of_vertices();
    int count = 0;
    while(count++ < max_iterations) {
        res1 = splitLongConstraints(cdt, length_threshold);
        res2 = addCentroidsOfLargeTriangles(cdt, area_threshold);
        if(res1 == 0 && res2 == 0) {
            break; // No more points to add
        }
    }

    for(const auto& point : points_to_add) {
        cdt.insert(point);
    }
}

////////////////////////////////////////////////////////////////////////////////
// NavMeshRoutingEngine
////////////////////////////////////////////////////////////////////////////////
RoutingEngine::RoutingEngine()
{
}

RoutingEngine::RoutingEngine(const PolyWithHoles& poly)
{
    cdt.insert_constraint(
        poly.outer_boundary().vertices_begin(), poly.outer_boundary().vertices_end(), true);
    for(const auto& p : poly.holes()) {
        cdt.insert_constraint(p.vertices_begin(), p.vertices_end(), true);
    }

    CGAL::mark_domain_in_triangulation(cdt);
    improveTriangulation(cdt);
    CGAL::make_conforming_Delaunay_2(cdt);
    CGAL::mark_domain_in_triangulation(cdt);

    mesh = std::make_unique<Mesh>(cdt);
    export_triangles(cdt, "triangle_cdt.txt");
}

std::unique_ptr<RoutingEngine> RoutingEngine::Clone() const
{
    auto clone = std::make_unique<RoutingEngine>();
    clone->cdt = cdt;
    clone->mesh = mesh->Clone();
    return clone;
}

Point RoutingEngine::ComputeWaypoint(Point currentPosition, Point destination)
{
    return ComputeAllWaypoints(currentPosition, destination)[1];
}

struct SearchState {
    double g_value{};
    double h_value{};
    CDT::Face_handle id{};
    SearchState* parent{};

    double f_value() const { return g_value + h_value; }
    bool parents_contain(CDT::Face_handle ancestor_id) const
    {
        const SearchState* pivot = this;
        while(pivot != nullptr) {
            if(pivot->id == ancestor_id) {
                return true;
            }
            pivot = pivot->parent;
        }
        return false;
    }
    std::vector<CDT::Face_handle> path() const
    {
        std::vector<CDT::Face_handle> p{};
        p.reserve(16);
        const SearchState* pivot = this;
        while(pivot != nullptr) {
            p.emplace_back(pivot->id);
            pivot = pivot->parent;
        }
        std::reverse(std::begin(p), std::end(p));
        return p;
    }
};

bool CompareSearchStatesGt(
    const std::shared_ptr<SearchState>& a,
    const std::shared_ptr<SearchState>& b)
{
    return a->f_value() > b->f_value();
}

double length_of_path(const std::vector<Point>& path)
{
    double segment_sum{};
    for(size_t index = 1; index < path.size(); ++index) {
        segment_sum += (Distance(path[index - 1], path[index]));
    }
    return segment_sum;
}

std::vector<Point> RoutingEngine::ComputeAllWaypoints(Point currentPosition, Point destination)
{
    const auto from_pos = CDT::Point{currentPosition.x, currentPosition.y};
    const auto to_pos = CDT::Point{destination.x, destination.y};
    const auto from = find_face(from_pos);
    const auto to = find_face(to_pos);

    if(from == to) {
        return std::vector<Point>{currentPosition, destination};
    }

    using SearchStatePtr = std::shared_ptr<SearchState>;

    std::vector<SearchStatePtr> open_states{};
    open_states.emplace_back(
        new SearchState{0.0, Distance(currentPosition, destination), from, nullptr});

    std::map<CDT::Face_handle, SearchStatePtr> closed_states{};

    std::vector<Point> path{};
    double path_length = std::numeric_limits<double>::infinity();

    while(!open_states.empty()) {
        std::make_heap(std::rbegin(open_states), std::rend(open_states), CompareSearchStatesGt);
        auto current_state = open_states.back();
        open_states.pop_back();
        closed_states.insert(std::make_pair(current_state->id, current_state));

        if(current_state->id == to) {
            // Unlike in A* this is only a first candidate solution
            // Now compute the actual path length via funnel algorithm
            // store path and length if this variant is the shortest found so far
            const auto vertex_ids = current_state->path();
            const auto found_path = straightenPath(currentPosition, destination, vertex_ids);
            const double found_path_length = length_of_path(found_path);
            if(found_path_length < path_length) {
                path = found_path;
                path_length = found_path_length;
            }
        }

        if(current_state->f_value() >= path_length) {
            // This search nodes f-value already excedes our paths length, and since the f-value is
            // underestimation of the path length the excat path cannot be shorter than what we have
            return path;
        }

        // Generate successors
        for(int idx = 0; idx < 3; ++idx) {
            const auto target = current_state->id->neighbor(idx);
            if(!target->get_in_domain()) {
                // Not a neighboring triangle.
                continue;
            }
            // Do not add search nodes for nodes already in the ancestor list of this path
            if(current_state->parents_contain(target)) {
                continue;
            }

            // Skip successors for nodes already in the closed list
            if(closed_states.contains(target)) {
                continue;
            }

            const auto edge = cdt.segment(target, idx);

            // For all remaining nodes compute g/h values
            // The h-value is the distance between the goal and the closts point on the edge
            // between the current triangle and this successor
            const double h_value = sqrt(CGAL::to_double((CGAL::squared_distance(to_pos, edge))));

            // The g-value is the maximum of:

            // "The first and simplest is the distance between the start point and the closest
            // point to it on the entry edge of the corresponding triangle."
            const double g_value_1 =
                sqrt(CGAL::to_double((CGAL::squared_distance(from_pos, edge))));

            // The second is g(s) plus the distance between the triangles associated with s and
            // s′. We assume that the g-value of s is a lower bound, and so we wish to add the
            // shortest such distance to achieve another lower bound. Again, we take this
            // measurement using the edges by which the triangles were first reached by search.
            // Since the triangles are adjacent, this is the distance of moving through the
            // triangle associated with s. In Theorem 4.2.12, we proved that the shortest
            // distance between two edges in a triangle was an arc path around the vertex shared
            // by these edges. Thus, if the entry edges of the triangles corresponding to s′ and
            // s form an angle θ, this estimate is calculated as g(s) + rθ. NOTE: Right now this
            // is always g(s) + zero as we asume point size agents (for now)
            const double g_value_2 = current_state->g_value + 0;

            //  Another lower bound value for g(s′) is g(s)+(h(s)−h(s′)), or the parent state’s
            //  g-value plus the difference between its h-value and that of the child state.
            //  This is an underes- timate because the Euclidean distance metric used for the
            //  heuristic is consistent.
            const double g_value_3 = current_state->g_value + current_state->h_value - h_value;

            const double g_value = std::max(g_value_1, std::max(g_value_2, g_value_3));

            // NOTE(kkratz): Clang16 seems to be confused with capturing a structured binding
            // and emits a warnign when capturing 'target' directly As of C++20 this SHOULD(TM)
            // be valid code but see:
            // https://stackoverflow.com/questions/46114214/lambda-implicit-capture-fails-with-variable-declared-from-structured-binding
            auto t2 = target;

            // TODO(kkratz): replace this find on unsorted vector with something with a better
            // runtime
            if(auto iter = std::find_if(
                   std::begin(open_states),
                   std::end(open_states),
                   [t2](const auto& s) { return s->id == t2; });
               iter != std::end(open_states)) {
                if(auto& s = *iter; s->g_value > g_value) {
                    s->g_value = g_value;
                    s->parent = current_state.get();
                }

            } else if(auto iter = closed_states.find(target); iter != std::end(closed_states)) {
                if(auto& [_, s] = *iter; s->g_value > g_value) {
                    s->g_value = g_value;
                    s->parent = current_state.get();
                    open_states.push_back(s);
                    closed_states.erase(s->id);
                }
            } else {
                open_states.emplace_back(
                    new SearchState{g_value, h_value, target, current_state.get()});
            }
        }
    }

    return path;
}

bool RoutingEngine::IsRoutable(Point p) const
{
    try {
        find_face({p.x, p.y});
    } catch(const SimulationError&) {
        return false;
    }
    return true;
}

void RoutingEngine::Update()
{
}

CDT::Face_handle RoutingEngine::find_face(K::Point_2 p) const
{
    const auto face = cdt.locate(p);
    if(face == nullptr || cdt.is_infinite(face) || !face->get_in_domain()) {
        throw SimulationError(
            "Point ({}, {}) is outside of accessible area",
            CGAL::to_double(p.x()),
            CGAL::to_double(p.y()));
    }
    return face;
}


std::vector<Point>
RoutingEngine::straightenPath(Point from, Point to, const std::vector<CDT::Face_handle>& path)
{
  // This is the actual simple stupid funnel algorithm
  //  https://cdn.aaai.org/AAAI/2006/AAAI06-148.pdf
  // arc-paths from the "Efficient Triangulation-Based Pathfinding" publication
    const size_t portalCount = path.size();
    const double radius = 0.2; // Object radius

    auto apex = from;
    auto portal_left = from;
    auto portal_right = from;

    const auto get_edge = [](const auto& a, const auto& b) {
        for(int idx = 0; idx < 3; ++idx) {
            if(a->neighbor(idx) == b) {
                auto v1 = a->vertex((idx + 1) % 3)->point();
                auto v2 = a->vertex((idx + 2) % 3)->point();
                return LineSegment{
                    {CGAL::to_double(v1.x()), CGAL::to_double(v1.y())},
                    {CGAL::to_double(v2.x()), CGAL::to_double(v2.y())}};
            }
        }
        throw SimulationError("Internal Error");
    };

    std::vector<Point> waypoints{from};
    // This is an over estimation but IMO preferable to repeadted allocations.
    // Ideally we replace this with something w.o. allocations
    waypoints.reserve(path.size() + 1);
    for(size_t index_portal = 1; index_portal <= portalCount; ++index_portal) {
        const auto face_from = path[index_portal - 1];
        const auto face_to = path[index_portal];
        if(face_from->neighbor(0) == face_to) {
        }

        const auto portal =
            index_portal < portalCount ? get_edge(face_from, face_to) : LineSegment(to, to);
  
        const auto p2 = portal.p2;
        const auto p1 = portal.p1;
        // Given edge: p1 to p2
        Point edge = (p2 - p1).Normalized();

        // Use offset_p1 and offset_p2 as left/right funnel legs
        // Efficient Triangulation-Based Pathfinding uses tangent points to circles around vertices.
        // We use the inward normal to the edge as the offset direction.
        Point normal = {-edge.y, edge.x};
        Point offset_p1 = p1 - normal * radius;
        Point offset_p2 = p2 + normal * radius;
        const auto candidate_left = offset_p2;
        const auto candidate_right = offset_p1;
      
        if(triarea2d(apex, portal_right, candidate_right) <= 0.0) {
            if(apex == portal_right || triarea2d(apex, portal_left, candidate_right) > 0.0) {
                portal_right = candidate_right;
            } else {
                waypoints.emplace_back(portal_left);
                apex = portal_left;
                portal_left = apex;
                portal_right = apex;
                continue;
            }
        }
        if(triarea2d(apex, portal_left, candidate_left) >= 0.0) {
            if(apex == portal_left || triarea2d(apex, portal_right, candidate_left) < 0.0) {
                portal_left = candidate_left;
            } else {
                waypoints.emplace_back(portal_right);
                apex = portal_right;
                portal_left = apex;
                portal_right = apex;
                continue;
            }
        }
    }

    if (waypoints.empty() || waypoints.back() != to) {
      waypoints.emplace_back(to);
    }
    
    return waypoints;
}
