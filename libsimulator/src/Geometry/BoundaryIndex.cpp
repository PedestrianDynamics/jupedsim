// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/BoundaryIndex.hpp"

#include "CfgCgal.hpp"
#include "SimulationError.hpp"

#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangular_expansion_visibility_2.h>
#include <CGAL/intersections.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <optional>
#include <set>
#include <utility>
#include <variant>
#include <vector>

//==================================================================================================
// NaiveBoundaryIndex
//==================================================================================================
NaiveBoundaryIndex::NaiveBoundaryIndex(std::vector<SegmentGrid<>> regions_)
    : regions(std::move(regions_))
{
}

std::vector<LineSegment> NaiveBoundaryIndex::Query(const Location& loc, double maximum_distance)
{
    auto range = regions[loc.region()].LineSegmentsInDistanceTo(maximum_distance, loc.xy());
    return {std::begin(range), std::end(range)};
}

//==================================================================================================
// Shared portal visibility helpers
//==================================================================================================
namespace
{
/// Shorter than this a clipped piece is an artefact of the clip, not something an agent can
/// see, let alone be pushed away from.
constexpr double kMinPartLength = 1e-9;

/// How close to an end of a wall, as a fraction of its length, a cut has to land to count as
/// being that end. On anything of a building's size this is well under a micrometre.
constexpr double kSnapEps = 1e-9;

/// A seam as a sight ray needs it: where it stands and where crossing it leads.
struct SeamCandidate {
    LineSegment segment{};
    std::size_t neighbor{};
};

/// Everything one query can possibly touch, grouped by the region it belongs to.
///
/// Walls are pooled into one flat vector so that an index into it identifies a wall for the
/// rest of the query. Identity is per region and not per segment value: where a surface folds,
/// two regions can carry walls that fall on the exact same (x, y) segment, and the two are
/// still different walls with different things in front of them.
struct Candidates {
    std::vector<LineSegment> walls{};
    std::vector<std::vector<std::size_t>> wall_ids_of{};
    std::vector<std::vector<SeamCandidate>> seams_of{};
};

/// Collects the walls and seams @p radius around @p p, region by region.
///
/// The pruning is exact and not a first cut that something later has to make up for: every
/// point the answer may contain lies within @p radius of @p p, and whether such a point is
/// seen is settled by what the straight line from @p p to it crosses -- a line that never
/// leaves the disc. Geometry with no point in the disc can therefore neither be reported nor
/// hide anything that is.
Candidates CollectCandidates(const RegionGraph& g, Point p, double radius)
{
    Candidates candidates{};
    candidates.wall_ids_of.resize(boost::num_vertices(g));
    candidates.seams_of.resize(boost::num_vertices(g));

    for(const auto region : boost::make_iterator_range(boost::vertices(g))) {
        for(const auto& wall : g[region]->LineSegmentsInDistanceTo(radius, p)) {
            candidates.wall_ids_of[region].push_back(candidates.walls.size());
            candidates.walls.push_back(wall);
        }
        for(const auto edge : boost::make_iterator_range(boost::out_edges(region, g))) {
            const auto& seam = g[edge];
            if(seam.DistTo(p) <= radius) {
                candidates.seams_of[region].push_back(SeamCandidate{seam, boost::target(edge, g)});
            }
        }
    }
    return candidates;
}

/// A parameter within rounding distance of an end of a segment, snapped to that end.
///
/// A cut that close was made by geometry running through the segment's own endpoint, and
/// re-deriving that endpoint from an angle, or from a disc, must not be allowed to move it.
double SnapToEnd(double s)
{
    if(s < kSnapEps) {
        return 0.0;
    }
    return s > 1.0 - kSnapEps ? 1.0 : s;
}

/// Where @p x sits along @p ls, as the parameter of its projection onto it.
double ParameterAlong(const LineSegment& ls, Point x)
{
    const Point along{ls.p2.x - ls.p1.x, ls.p2.y - ls.p1.y};
    return Point{x.x - ls.p1.x, x.y - ls.p1.y}.ScalarProduct(along) / along.ScalarProduct(along);
}

/// The part of @p ls between the two parameters, or nothing when that is empty or too short
/// to mean anything.
std::optional<LineSegment> SubSegment(const LineSegment& ls, double from, double to)
{
    const Point along{ls.p2.x - ls.p1.x, ls.p2.y - ls.p1.y};
    if(to <= from || (to - from) * along.Norm() < kMinPartLength) {
        return std::nullopt;
    }
    // An untouched end stays the endpoint it was, rather than an arithmetic re-derivation of
    // it: a wall seen whole should come back as itself.
    const auto at = [&](double s) {
        if(s <= 0.0) {
            return ls.p1;
        }
        if(s >= 1.0) {
            return ls.p2;
        }
        return Point{ls.p1.x + s * along.x, ls.p1.y + s * along.y};
    };
    return LineSegment{at(from), at(to)};
}

/// The part of @p ls inside the closed disc, or nothing if it does not reach into it.
std::optional<LineSegment> ClipToDisc(const LineSegment& ls, Point center, double radius)
{
    const Point along{ls.p2.x - ls.p1.x, ls.p2.y - ls.p1.y};
    const Point to_start{ls.p1.x - center.x, ls.p1.y - center.y};
    const double a = along.ScalarProduct(along);
    if(a <= 0.0) {
        return std::nullopt;
    }
    const double b = 2.0 * to_start.ScalarProduct(along);
    const double c = to_start.ScalarProduct(to_start) - radius * radius;
    const double discriminant = b * b - 4.0 * a * c;
    if(discriminant < 0.0) {
        // The supporting line misses the circle, so no point of the segment is inside it
        // either: anything reaching in would have to cross the circle to get there.
        return std::nullopt;
    }
    const double root = std::sqrt(discriminant);
    return SubSegment(
        ls, std::max(0.0, (-b - root) / (2.0 * a)), std::min(1.0, (-b + root) / (2.0 * a)));
}

/// Turns what the portal recursion found -- per wall, the stretches of it that were seen, as
/// parameters along that wall -- into the answer Query() promises: the visible pieces
/// themselves. @p parts_of_wall is sorted in place.
///
/// One stretch of wall can arrive in several pieces that touch: split where the recursion's
/// own bookkeeping happened to cut the wall, or seen through two windows whose views meet on
/// it. Touching intervals are fused before clipping, so each piece that leaves here is
/// maximal. A cut within rounding distance of an end of the wall is that end: re-deriving an
/// endpoint from the recursion's arithmetic must not move it.
std::vector<LineSegment> AssembleAnswer(
    const Candidates& candidates,
    Point p,
    double maximum_distance,
    std::vector<std::vector<std::pair<double, double>>>& parts_of_wall)
{
    // Where the surface folds, two regions can carry walls on the same (x, y) segment, and
    // nothing says the mesh stored them the same way round: two candidates, one answer. The
    // orientation-blind key keeps a piece from appearing twice.
    std::set<std::pair<Point, Point>> emitted{};
    std::vector<LineSegment> result{};
    for(std::size_t id = 0; id < parts_of_wall.size(); ++id) {
        auto& intervals = parts_of_wall[id];
        if(intervals.empty()) {
            continue;
        }
        std::sort(std::begin(intervals), std::end(intervals));
        const auto& wall = candidates.walls[id];
        // The disc is cut against the WHOLE wall, not against each piece: where the disc only
        // grazes a wall the cut is decided by the last bits of a discriminant, and feeding it
        // re-derived piece endpoints turns the same tangency into an answer a few nanometres
        // long.
        const auto within_disc = ClipToDisc(wall, p, maximum_distance);
        if(!within_disc) {
            continue;
        }
        const double disc_from = ParameterAlong(wall, within_disc->p1);
        const double disc_to = ParameterAlong(wall, within_disc->p2);
        std::size_t i = 0;
        while(i < intervals.size()) {
            auto [from, to] = intervals[i];
            for(++i; i < intervals.size() && intervals[i].first <= to + kSnapEps; ++i) {
                to = std::max(to, intervals[i].second);
            }
            const auto part = SubSegment(
                wall, SnapToEnd(std::max(from, disc_from)), SnapToEnd(std::min(to, disc_to)));
            if(!part) {
                continue;
            }
            const auto key =
                part->p1 < part->p2 ? std::pair{part->p1, part->p2} : std::pair{part->p2, part->p1};
            if(emitted.insert(key).second) {
                result.push_back(*part);
            }
        }
    }
    return result;
}
} // namespace

//==================================================================================================
// PortalBoundaryIndex
//==================================================================================================
namespace
{
/// Exact arithmetic for the arrangement work. The visibility region of a point among segments
/// is made of constructed intersection points, and windows feed those points straight back in
/// as new segment endpoints one region later. With inexact constructions the second round
/// would not find the points of the first again; with EPECK everything stays consistent, and
/// the one rounding step is the conversion of the finished answer back to doubles.
using VisKernel = CGAL::Exact_predicates_exact_constructions_kernel;
using VisPoint = VisKernel::Point_2;
using VisSegment = VisKernel::Segment_2;
using VisArrangement = CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<VisKernel>>;

/// Triangular_expansion_visibility_2, and the choice is made on its source rather than its
/// advertising: its query triangulates EVERY edge of the attached arrangement as a constraint
/// and expands triangles from the query point, stopping at constraints -- the face handle is
/// only checked, never consulted. Segments floating loose in the query face (pruning leaves
/// plenty) are therefore just more constraints, where the concept documentation would only
/// promise faces whose every edge separates two faces. Rotational_sweep_visibility_2 walks
/// the query face's boundary circulators instead, which leaves the same question to how the
/// arrangement happens to file those segments -- as inner boundaries of the face -- so the
/// engine whose correctness is visible in its own code wins. The regularized flavour drops
/// zero-width needles, which the Query() contract allows to be omitted anyway. Both
/// engines want the query point in a BOUNDED face; the box below provides one.
using VisEngine = CGAL::Triangular_expansion_visibility_2<VisArrangement, CGAL::Tag_true>;

/// A recursion this deep means a sight line threading this many doorways inside one query
/// disc; no building does that, but a cycle in the window bookkeeping would.
constexpr int kMaxWindowDepth = 128;

VisPoint ToExact(Point p)
{
    return {p.x, p.y};
}

Point FromExact(const VisPoint& p)
{
    return {CGAL::to_double(p.x()), CGAL::to_double(p.y())};
}

/// One pending look into a region: through which piece of which seam, and how many windows
/// deep. The agent's own region is the one job without a window.
struct WindowJob {
    std::size_t region{};
    int depth{};
    /// The stretch of seam the look passes through, exact because it was constructed.
    std::optional<VisSegment> window{};
    /// The seam the window lies on, as the region it was seen from stores it.
    LineSegment seam_of_parent{};
    /// The region the window was seen from.
    std::size_t parent{};
};

/// An opaque segment of one job's arrangement, kept together with what it means, so pieces of
/// the visibility boundary can be traced back to the wall or seam they lie on.
struct JobSegment {
    VisSegment segment{}; ///< the whole wall or seam, for containment tests
    bool is_seam{};
    std::size_t wall_id{}; ///< into Candidates::walls, when a wall
    const SeamCandidate* seam{}; ///< when a seam
};

/// The part of a boundary piece whose sight lines from the agent pass through the window:
/// the piece cut to the wedge the window spans as seen from the agent's point.
///
/// A piece cannot simply be kept or dropped whole. The edge of the wedge -- the shadow a
/// window endpoint casts -- shows up as a vertex of the visibility region only where some
/// curve of the arrangement still ends at that window endpoint, and the far-side cut of the
/// entering region may have removed every such curve; a piece can then straddle the shadow.
/// The clip is exact: the wedge's edges run through constructed window endpoints, and
/// deciding just-inside against just-outside in doubles would tear pieces at exactly the
/// places the recursion has to agree with itself about.
std::optional<VisSegment>
ClipToWindowWedge(const VisPoint& q, const VisSegment& window, VisSegment piece)
{
    for(int i = 0; i < 2; ++i) {
        const VisPoint through = i == 0 ? window.source() : window.target();
        const VisPoint other = i == 0 ? window.target() : window.source();
        const VisKernel::Line_2 edge{q, through};
        // Never on the line: the window lies on a seam that does not run through the agent.
        const auto inside = edge.oriented_side(other);
        const auto side_a = edge.oriented_side(piece.source());
        const auto side_b = edge.oriented_side(piece.target());
        const bool out_a = side_a != inside && side_a != CGAL::ON_ORIENTED_BOUNDARY;
        const bool out_b = side_b != inside && side_b != CGAL::ON_ORIENTED_BOUNDARY;
        if(out_a && out_b) {
            return std::nullopt;
        }
        if(!out_a && !out_b) {
            continue;
        }
        const auto cut = CGAL::intersection(edge, piece);
        const auto* at = cut ? std::get_if<VisPoint>(&*cut) : nullptr;
        if(at == nullptr) {
            return std::nullopt; // grazing contact only
        }
        piece = VisSegment{out_a ? *at : piece.source(), out_b ? *at : piece.target()};
    }
    if(piece.source() == piece.target()) {
        return std::nullopt;
    }
    return piece;
}

/// Runs the visibility query of one (region, window) job: visible wall pieces go into
/// @p parts_of_wall as parameter intervals along their wall, visible seam pieces become new
/// jobs in @p pending.
void ProcessWindowJob(
    const Candidates& candidates,
    Point p,
    const VisPoint& q,
    double maximum_distance,
    const WindowJob& job,
    std::vector<std::vector<std::pair<double, double>>>& parts_of_wall,
    std::vector<WindowJob>& pending)
{
    // What this region's arrangement is made of, and what each segment stands for. A segment
    // whose supporting line runs through the agent is left out entirely: a straight sight
    // line meets a line through its own origin only at the origin, so such a segment can
    // neither block any ray nor be struck by one -- except the two rays running along the
    // line itself, a set of measure zero. Leaving it out changes nothing else, keeps the
    // query point off the arrangement's edges.
    std::vector<JobSegment> segments{};
    const auto keep = [&](const LineSegment& ls) -> std::optional<VisSegment> {
        const VisPoint a = ToExact(ls.p1);
        const VisPoint b = ToExact(ls.p2);
        if(CGAL::orientation(a, b, q) == CGAL::COLLINEAR) {
            return std::nullopt;
        }
        return VisSegment{a, b};
    };
    for(const auto id : candidates.wall_ids_of[job.region]) {
        if(const auto segment = keep(candidates.walls[id])) {
            segments.push_back(JobSegment{*segment, false, id, nullptr});
        }
    }
    for(const auto& seam : candidates.seams_of[job.region]) {
        if(const auto segment = keep(seam.segment)) {
            segments.push_back(JobSegment{*segment, true, 0, &seam});
        }
    }

    // The opaque input: this region's walls and seams -- except the seam the look enters
    // through, which goes in MINUS the window. The window is exactly the piece of this seam
    // the previous region's query proved visible, so a ray through it is unobstructed all the
    // way back to the agent by construction, and nothing of that path needs re-checking here.
    // The rest of the seam stays opaque: sight was shown to pass through the window and
    // nowhere else.
    //
    // Everything else is cut back to the FAR side of the seam's supporting line first. A
    // region only exists for a sight line once the line has entered it, and a straight line
    // crosses the seam's line exactly once -- at the window -- so everything of this region a
    // window ray can meet lies strictly beyond that line. What lies on the agent's side is
    // the region folding back over ground the ray already covered in other regions (the floor
    // below an upper storey, say), and left in it would throw phantom shadows.
    //
    // Pruning is safe here for the same reason the candidate pruning itself is exact: every
    // segment pruned from
    // the candidates lies wholly beyond the radius, so the false gap it leaves can only be
    // reached by sight lines already longer than the radius -- visibility within the disc is
    // unaffected, and everything is disc-clipped at the end anyway.
    std::optional<VisKernel::Line_2> window_line{};
    CGAL::Oriented_side agent_side{CGAL::ON_ORIENTED_BOUNDARY};
    if(job.window) {
        window_line =
            VisKernel::Line_2{ToExact(job.seam_of_parent.p1), ToExact(job.seam_of_parent.p2)};
        // Strictly off the line: a seam collinear with the agent never gets a window.
        agent_side = window_line->oriented_side(q);
    }
    const auto beyond_window = [&](const VisSegment& segment) -> std::optional<VisSegment> {
        if(!window_line) {
            return segment; // the agent's own region is walked whole
        }
        const auto side_a = window_line->oriented_side(segment.source());
        const auto side_b = window_line->oriented_side(segment.target());
        const bool near_a = side_a == agent_side;
        const bool near_b = side_b == agent_side;
        if(!near_a && !near_b) {
            return segment;
        }
        if(near_a && near_b) {
            return std::nullopt;
        }
        if((near_a ? side_b : side_a) == CGAL::ON_ORIENTED_BOUNDARY) {
            return std::nullopt; // reaches the line from the near side and no further
        }
        const auto cut = CGAL::intersection(*window_line, segment);
        const auto* at = cut ? std::get_if<VisPoint>(&*cut) : nullptr;
        // Unreachable: endpoints strictly on opposite sides always meet the line in a point.
        // Failing SAFE still matters -- an un-cut segment here is precisely the phantom
        // shadow this clip exists to prevent.
        assert(at != nullptr);
        if(at == nullptr) {
            return std::nullopt;
        }
        return VisSegment{*at, near_a ? segment.target() : segment.source()};
    };
    std::vector<VisSegment> curves{};
    curves.reserve(segments.size() + 4);
    [[maybe_unused]] int entering_found = 0;
    for(const auto& js : segments) {
        // Seams are stored once per direction, so the seam the window lies on appears here as
        // its own exact reverse, leading back to the region the look came from. The match is
        // bit-exact on purpose -- CreateRegionGraph builds both directions from the same mesh
        // points -- and the assert below keeps that coupling visible: a window whose seam is
        // not found again would stay silently bricked up.
        const bool entering = job.window && js.is_seam && js.seam->neighbor == job.parent &&
                              js.seam->segment.p1 == job.seam_of_parent.p2 &&
                              js.seam->segment.p2 == job.seam_of_parent.p1;
        if(entering) {
            entering_found += 1;
        }
        if(!entering) {
            if(const auto reachable = beyond_window(js.segment)) {
                curves.push_back(*reachable);
            }
            continue;
        }
        // Cut the window out: order its endpoints along the seam and keep what is left of the
        // seam on either side.
        const VisPoint a = js.segment.source();
        const VisPoint b = js.segment.target();
        const auto along = [&](const VisPoint& x) { return (x - a) * (b - a); };
        VisPoint first = job.window->source();
        VisPoint last = job.window->target();
        if(along(first) > along(last)) {
            std::swap(first, last);
        }
        if(a != first) {
            curves.push_back(VisSegment{a, first});
        }
        if(last != b) {
            curves.push_back(VisSegment{last, b});
        }
    }
    assert((!job.window || entering_found == 1) && "the window's seam was not found again");
    // A box around the whole disc closes the arrangement: the engine wants the query point in
    // a bounded face, and with pruning (or a window at the agent's back) the region's own
    // boundary does not provide one. Every point of the box lies beyond the radius, so
    // whatever the box hides is nothing the disc clip would have kept.
    const double half = maximum_distance + 1.0;
    const VisPoint corners[4] = {
        {p.x - half, p.y - half},
        {p.x + half, p.y - half},
        {p.x + half, p.y + half},
        {p.x - half, p.y + half}};
    for(int i = 0; i < 4; ++i) {
        curves.push_back(VisSegment{corners[i], corners[(i + 1) % 4]});
    }

    VisArrangement arrangement{};
    CGAL::insert(arrangement, std::begin(curves), std::end(curves));

    const CGAL::Arr_naive_point_location<VisArrangement> point_location{arrangement};
    const auto located = point_location.locate(q);
    const auto* face = std::get_if<VisArrangement::Face_const_handle>(&located);
    // The collinear drop above took every segment through the agent's point out of the
    // arrangement, so the point lies strictly inside a face.
    assert(face != nullptr && !(*face)->is_unbounded());
    if(face == nullptr) {
        return;
    }

    const VisEngine engine{arrangement};
    VisArrangement visibility{};
    const auto seen = engine.compute_visibility(q, *face, visibility);

    // Walk the boundary of the visibility region. Each edge is a piece of an input segment,
    // a piece of the box, or a stretch of a sight ray connecting them across free space; only
    // the first kind is an answer, and it is found by asking which input segment contains it.
    auto circulator = seen->outer_ccb();
    auto edge = circulator;
    do {
        const VisPoint& source = edge->source()->point();
        const VisPoint& target = edge->target()->point();
        if(CGAL::orientation(q, source, target) == CGAL::COLLINEAR) {
            continue; // runs along a sight ray: free space, not something seen
        }
        // Which input segment the piece lies on. Where a fold leaves a wall and a seam of the
        // SAME region on the same (x, y) segment the piece is credited to the SEAM: a seam
        // says the surface continues there, and crediting the wall instead would silently
        // delete everything behind it.
        // Two coincident seams of one region leading to DIFFERENT neighbours would have no
        // defensible winner at all; CreateRegionGraph does not produce that for a manifold
        // mesh, and the assert keeps the assumption visible.
        const JobSegment* on = nullptr;
        [[maybe_unused]] int seams_matched = 0;
        for(const auto& js : segments) {
            if(!js.segment.has_on(source) || !js.segment.has_on(target)) {
                continue;
            }
            if(js.is_seam) {
                seams_matched += 1;
                if(on == nullptr || !on->is_seam) {
                    on = &js;
                }
            } else if(on == nullptr) {
                on = &js;
            }
        }
        assert(seams_matched <= 1 && "coincident seams of one region: no defensible winner");
        if(on == nullptr) {
            continue; // a piece of the box
        }
        // Looking through a window means standing outside the region, and the face the agent
        // occupies then wraps around the region's OUTSIDE: rays that never pass the window
        // reach around the boundary -- past the ends of the seam, through where pruned
        // geometry would stand -- and strike walls and seams from behind. Those are no
        // sightings: the straight line from the agent never entered this region at all. What
        // separates a
        // sighting from such an accident is the window itself, so a piece counts exactly as
        // far as the straight lines from the agent to it pass through the window -- the wedge
        // clip. The agent's own region needs no such repair: there the agent is inside, and
        // its face reaches nothing its own sight lines do not reach.
        VisSegment piece{source, target};
        if(job.window) {
            const auto through = ClipToWindowWedge(q, *job.window, piece);
            if(!through) {
                continue;
            }
            piece = *through;
        }
        if(!on->is_seam) {
            const auto& wall = candidates.walls[on->wall_id];
            const VisPoint a = ToExact(wall.p1);
            const VisPoint b = ToExact(wall.p2);
            const auto length2 = (b - a) * (b - a);
            const double from = CGAL::to_double(((piece.source() - a) * (b - a)) / length2);
            const double to = CGAL::to_double(((piece.target() - a) * (b - a)) / length2);
            parts_of_wall[on->wall_id].emplace_back(std::min(from, to), std::max(from, to));
            continue;
        }
        // A visible piece of a seam is a window: everything behind it is seen, if at all,
        // through exactly this stretch. Nothing beyond a window is nearer than the window --
        // every sight line onward passes through it first -- so a window already out of reach
        // opens on nothing the disc clip would keep, and this prune is what ends the
        // recursion.
        const LineSegment window{FromExact(piece.source()), FromExact(piece.target())};
        if(window.DistTo(p) > maximum_distance) {
            continue;
        }
        if(Distance(window.p1, window.p2) < kMinPartLength) {
            continue; // a zero-width slit; measure zero may be omitted
        }
        assert(job.depth < kMaxWindowDepth && "portal window recursion runaway");
        if(job.depth >= kMaxWindowDepth) {
            continue; // the cap has to hold with the assert compiled out, too
        }
        pending.push_back(
            WindowJob{on->seam->neighbor, job.depth + 1, piece, on->seam->segment, job.region});
    } while(++edge != circulator);
}
} // namespace

PortalBoundaryIndex::PortalBoundaryIndex(std::unique_ptr<RegionGraph> graph) : g(std::move(graph))
{
}

std::vector<LineSegment> PortalBoundaryIndex::Query(const Location& loc, double maximum_distance)
{
    const Point p = loc.xy();
    const VisPoint q = ToExact(p);
    const auto candidates = CollectCandidates(*g, p, maximum_distance);

    std::vector<std::vector<std::pair<double, double>>> parts_of_wall(candidates.walls.size());
    std::vector<WindowJob> pending{};
    pending.push_back(WindowJob{loc.region(), 0});
    while(!pending.empty()) {
        const WindowJob job = std::move(pending.back());
        pending.pop_back();
        ProcessWindowJob(candidates, p, q, maximum_distance, job, parts_of_wall, pending);
    }

    // One stretch of wall can arrive here in several pieces that touch -- split where the box
    // happened to cross the wall, or seen through two windows whose views meet on it -- so the
    // assembly fuses them before clipping.
    return AssembleAnswer(candidates, p, maximum_distance, parts_of_wall);
}

//==================================================================================================
// Factories
//==================================================================================================
std::unique_ptr<BoundaryIndex>
MakeNaiveBoundaryIndex(const SurfaceMesh& mesh, const RegionSplit& region_split)
{
    auto boundaries = CreatePerRegionSegmentGrids(mesh, region_split);
    return std::make_unique<NaiveBoundaryIndex>(std::move(boundaries));
}

std::unique_ptr<BoundaryIndex>
MakePortalBoundaryIndex(const SurfaceMesh& mesh, const RegionSplit& region_split)
{
    auto g = CreateRegionGraph(mesh, region_split);
    return std::make_unique<PortalBoundaryIndex>(std::move(g));
}

//==================================================================================================
// Supporting code
//==================================================================================================
std::tuple<SurfaceMesh::Face_index, SurfaceMesh::Halfedge_index>
IncidentFaceAndHalfedge(const SurfaceMesh& mesh, SurfaceMesh::Edge_index e)
{
    auto h = mesh.halfedge(e, 0);
    if(mesh.is_border(h)) {
        h = mesh.opposite(h);
    }
    if(mesh.is_border(h)) {
        throw SimulationError("Input mesh contains edges without incident faces.");
    }
    return {mesh.face(h), h};
}

std::vector<SegmentGrid<>>
CreatePerRegionSegmentGrids(const SurfaceMesh& mesh, const RegionSplit& region_split)
{
    std::vector<std::vector<LineSegment>> elements{};
    elements.resize(region_split.count);

    for(const auto edge : mesh.edges()) {
        if(mesh.is_border(edge)) {
            const auto [f, he] = IncidentFaceAndHalfedge(mesh, edge);
            const auto region_id = region_split.region[f];
            const auto v0 = mesh.point(mesh.source(he));
            const auto v1 = mesh.point(mesh.target(he));
            elements.at(region_id).emplace_back(Point(v0.x(), v0.y()), Point(v1.x(), v1.y()));
            continue;
        }
    }
    std::vector<SegmentGrid<>> boundaries{};
    boundaries.reserve(region_split.count);
    std::transform(
        std::begin(elements),
        std::end(elements),
        std::back_inserter(boundaries),
        [](const auto& e) { return SegmentGrid(e); });
    return boundaries;
}

std::unique_ptr<RegionGraph>
CreateRegionGraph(const SurfaceMesh& mesh, const RegionSplit& region_split)
{
    std::vector<std::vector<LineSegment>> elements{};
    elements.resize(region_split.count);
    std::vector<std::tuple<size_t, size_t, LineSegment>> seams{};
    seams.reserve(64);

    for(const auto edge : mesh.edges()) {
        if(mesh.is_border(edge)) {
            const auto [f, he] = IncidentFaceAndHalfedge(mesh, edge);
            const auto region_id = region_split.region[f];
            const auto v0 = mesh.point(mesh.source(he));
            const auto v1 = mesh.point(mesh.target(he));
            elements.at(region_id).emplace_back(Point(v0.x(), v0.y()), Point(v1.x(), v1.y()));
            continue;
        }
        const auto he0 = mesh.halfedge(edge, 0);
        const auto he0_region = region_split.region[mesh.face(he0)];
        const auto he1 = mesh.halfedge(edge, 1);
        const auto he1_region = region_split.region[mesh.face(he1)];
        if(he0_region != he1_region) {
            const auto v0 = mesh.point(mesh.source(he0));
            const auto v1 = mesh.point(mesh.target(he0));
            seams.emplace_back(
                he0_region, he1_region, LineSegment(Point(v0.x(), v0.y()), Point(v1.x(), v1.y())));
            seams.emplace_back(
                he1_region, he0_region, LineSegment(Point(v1.x(), v1.y()), Point(v0.x(), v0.y())));
        }
    }
    auto g = std::make_unique<RegionGraph>();
    for(auto&& e : elements) {
        auto v = boost::add_vertex(*g);
        (*g)[v] = std::make_unique<SegmentGrid<>>(e);
    }
    for(auto&& [src, dst, ls] : seams) {
        boost::add_edge(src, dst, ls, *g);
    }

    return g;
}
