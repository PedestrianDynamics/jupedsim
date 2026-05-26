// SPDX-License-Identifier: LGPL-3.0-or-later
#include "RoutingEngine.hpp"

#include "CfgCgal.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Mesh.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Distance_2/Point_2_Segment_2.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <CGAL/number_utils.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

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
    mesh = std::make_unique<Mesh>(cdt);
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

    SearchState(double g, double h, CDT::Face_handle _id, SearchState* _parent)
        : g_value(g), h_value(h), id(_id), parent(_parent)
    {
    }

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

struct FaceHandleHash {
    std::size_t operator()(CDT::Face_handle fh) const noexcept
    {
        // Face_handle is an iterator, therefore use "&*"-pattern to get the address.
        return std::hash<decltype(&*fh)>{}(&*fh);
    }
};

bool CompareSearchStatesGt(const SearchState* a, const SearchState* b)
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

    // Hold all search states inside a deque which never invalidates pointers and grows within O(1)
    std::deque<SearchState> all_search_states{};
    std::vector<SearchState*> open_states{};
    all_search_states.emplace_back(0.0, Distance(currentPosition, destination), from, nullptr);
    open_states.push_back(&all_search_states.back());

    std::unordered_map<CDT::Face_handle, SearchState*, FaceHandleHash> closed_states{};

    std::vector<Point> path{};
    double path_length = std::numeric_limits<double>::infinity();

    while(!open_states.empty()) {
        std::pop_heap(open_states.begin(), open_states.end(), CompareSearchStatesGt);
        SearchState* current_state = open_states.back();
        open_states.pop_back();
        closed_states.emplace(current_state->id, current_state);

        if(current_state->f_value() >= path_length) {
            // This search node's f-value already exceeds our path's length, and since the f-value
            // is underestimation of the path length the exact path cannot be shorter than what we
            // have
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

            // The shared edge between `current_state->id` and `target` is the edge
            // opposite vertex `idx` of the CURRENT face. CGAL's neighbor indexing is
            // not symmetric: the index of `target` in current's neighbor list differs
            // from the index of `current` in target's neighbor list, so querying
            // `cdt.segment(target, idx)` returns an unrelated edge of `target` and
            // produces bogus g/h values that mis-rank successors in A*.
            const auto edge = cdt.segment(current_state->id, idx);

            // For all remaining nodes compute g/h values
            // The h-value is the distance between the goal and the closest point on the edge
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
            // is always g(s) + zero as we assume point size agents (for now)
            const double g_value_2 = current_state->g_value + 0;

            //  Another lower bound value for g(s′) is g(s)+(h(s)−h(s′)), or the parent state’s
            //  g-value plus the difference between its h-value and that of the child state.
            //  This is an underestimate because the Euclidean distance metric used for the
            //  heuristic is consistent.
            const double g_value_3 = current_state->g_value + current_state->h_value - h_value;

            const double g_value = std::max(g_value_1, std::max(g_value_2, g_value_3));

            // Evaluate every route that reaches the destination inline so that all
            // candidate routes have their funnel computed — not just the first one
            // (minimum-f_value) to arrive.  The closed_states guard would otherwise
            // block all subsequent routes from being evaluated.
            if(target == to) {
                // g_value + h_value is f_value which is a lower bound and therefore needs
                // to be smaller than current path length to be a good candidate.
                if(g_value + h_value < path_length) {
                    // Unlike in A* this is only a first candidate solution
                    // Now compute the actual path length via funnel algorithm
                    // store path and length if this variant is the shortest found so far
                    const SearchState dest_state{g_value, h_value, to, current_state};
                    const auto vertex_ids = dest_state.path();
                    const auto found_path =
                        straightenPath(currentPosition, destination, vertex_ids);
                    const double found_path_length = length_of_path(found_path);
                    if(found_path_length < path_length) {
                        path = found_path;
                        path_length = found_path_length;
                    }
                }
                continue;
            }

            // NOTE(kkratz): Clang16 seems to be confused with capturing a structured binding
            // and emits a warning when capturing 'target' directly. As of C++20 this SHOULD(TM)
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
                if(auto* s = *iter; s->g_value > g_value) {
                    s->g_value = g_value;
                    s->parent = current_state;
                    // As the g_value got modified, the heap needs to be entirely remade.
                    std::make_heap(open_states.begin(), open_states.end(), CompareSearchStatesGt);
                }
            } else if(auto iter = closed_states.find(target); iter != closed_states.end()) {
                auto* s = iter->second;
                if(s->g_value > g_value) {
                    s->g_value = g_value;
                    s->parent = current_state;
                    open_states.push_back(s);
                    std::push_heap(open_states.begin(), open_states.end(), CompareSearchStatesGt);
                    closed_states.erase(s->id);
                }
            } else {
                all_search_states.emplace_back(g_value, h_value, target, current_state);
                open_states.push_back(&all_search_states.back());
                std::push_heap(open_states.begin(), open_states.end(), CompareSearchStatesGt);
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
    // TODO(kkratz): Remove the 0.2m edge width adjustment and replace this with p[roper
    // arc-paths from the "Efficient Triangulation-Based Pathfinding" publication
    const size_t portalCount = path.size();

    // This is the actual simple stupid funnel algorithm
    auto apex = from;
    auto portal_left = from;
    auto portal_right = from;

    size_t index_apex{0};
    size_t index_left{0};
    size_t index_right{0};

    const auto get_edge = [this](const auto& a, const auto& b) {
        for(int idx = 0; idx < 3; ++idx) {
            if(a->neighbor(idx) == b) {
                const auto s = cdt.segment(a, idx);
                const auto src = s.source();
                const auto tgt = s.target();
                return LineSegment{
                    {CGAL::to_double(src.x()), CGAL::to_double(src.y())},
                    {CGAL::to_double(tgt.x()), CGAL::to_double(tgt.y())}};
            }
        }
        throw SimulationError("Internal Error");
    };

    std::vector<Point> waypoints{from};
    // This is an overestimation but IMO preferable to repeated allocations.
    // Ideally we replace this with something w.o. allocations
    waypoints.reserve(path.size() + 1);
    for(size_t index_portal = 1; index_portal <= portalCount; ++index_portal) {
        const auto face_from = path[index_portal - 1];
        const auto portal = index_portal < portalCount ? get_edge(face_from, path[index_portal]) :
                                                         LineSegment(to, to);

        const auto line_segment_left = portal.p2;
        const auto line_segment_right = portal.p1;
        const auto line_segment_direction = (line_segment_right - line_segment_left).Normalized();
        const auto candidate_left = line_segment_left + (line_segment_direction * 0.2);
        const auto candidate_right = line_segment_right - (line_segment_direction * 0.2);

        if(triarea2d(apex, portal_right, candidate_right) <= 0.0) {
            if(apex == portal_right || triarea2d(apex, portal_left, candidate_right) > 0.0) {
                portal_right = candidate_right;
                index_right = index_portal;
            } else {
                waypoints.emplace_back(portal_left);
                apex = portal_left;
                index_apex = index_left;
                portal_left = apex;
                portal_right = apex;
                index_left = index_apex;
                index_right = index_apex;
                index_portal = index_apex;
                continue;
            }
        }
        if(triarea2d(apex, portal_left, candidate_left) >= 0.0) {
            if(apex == portal_left || triarea2d(apex, portal_right, candidate_left) < 0.0) {
                portal_left = candidate_left;
                index_left = index_portal;
            } else {
                waypoints.emplace_back(portal_right);
                apex = portal_right;
                index_apex = index_right;
                portal_left = apex;
                portal_right = apex;
                index_left = index_apex;
                index_right = index_apex;
                index_portal = index_apex;
                continue;
            }
        }
    }
    waypoints.emplace_back(to);
    return waypoints;
}
