// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/Geometry2D.hpp"
#include "Geometry/SegmentGrid.hpp"
#include "Geometry/WallMerge.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <boost/container/small_vector.hpp>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>

/// The walls a query found, whichever way it had to look for them.
///
/// A polygon-built geometry answers from one grid and hands out a slice of it. A mesh-built
/// one may have to look in several regions -- the agent's own and whatever a seam brings
/// within reach -- and then the same wall can turn up twice, because a wall running past a
/// region boundary belongs to both. Both cases have to come back as the same type, since a
/// model asks one question and gets segments either way.
///
/// Nothing is copied in either case: the pieces point into storage that outlives the query.
class WallRange : public std::ranges::view_interface<WallRange>
{
public:
    /// One region's contribution, already narrowed by that region's grid. The filtering is
    /// left where it belongs; what is added here is only the stitching and the recognising.
    using Span = SegmentGrid<MergedWall>::LineSegmentRange;
    using Spans = boost::container::small_vector<Span, 4>;

    WallRange() = default;

    /// The polygon path: one slice, nothing to stitch, nothing to recognise.
    explicit WallRange(Geometry2D::LineSegmentRange flat) : _flat(flat), _isFlat(true) {}

    /// The mesh path.
    explicit WallRange(Spans spans) : _spans(std::move(spans)) {}

    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = LineSegment;
        using difference_type = std::ptrdiff_t;
        using pointer = const LineSegment*;
        using reference = const LineSegment&;

        Iterator() = default;
        Iterator(const WallRange* range, bool at_end);

        const LineSegment& operator*() const;
        Iterator& operator++();
        Iterator operator++(int)
        {
            auto before = *this;
            ++*this;
            return before;
        }
        bool operator==(const Iterator& other) const;

    private:
        /// Park on the next wall worth handing out, crossing span boundaries as needed.
        void settle();
        bool worth_handing_out();

        const WallRange* _range{nullptr};
        DistanceQueryIterator<LineSegment> _flat{};
        std::size_t _span{0};
        DistanceQueryIterator<MergedWall> _wall{};
        /// Walls already handed out. Only filled when there is more than one span, so the
        /// common case of an agent well inside its region pays nothing for it.
        boost::container::small_vector<std::uint32_t, 16> _seen{};
    };

    Iterator begin() const { return Iterator{this, false}; }
    Iterator end() const { return Iterator{this, true}; }

private:
    Geometry2D::LineSegmentRange _flat{{}, {}};
    Spans _spans{};
    bool _isFlat{false};
};
