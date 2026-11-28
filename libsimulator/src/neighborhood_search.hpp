// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "generic_agent.hpp"
#include "kernel.hpp"
#include "simulation_error.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

/// Finds the items within a radius of a point, by sorting them into square cells.
///
/// The cells span the bounding box of the items and are numbered column by column. `_entries`
/// holds all items sorted by cell, so the cells of one column that a query covers are one
/// contiguous range of entries. Where a cell's entries start is stored in one of two layouts:
/// - dense: `_cell_start` has an element for every cell of the bounding box, and cell `c` holds
///   the entries from `_cell_start[c]` up to (excluding) `_cell_start[c + 1]`.
/// - sparse: used when the bounding box has far more cells than there are items, e.g. for two
///   groups kilometers apart. Only occupied cells are stored: `_cell_keys` lists them in order,
///   and the i-th of them holds the entries from `_cell_start[i]` up to `_cell_start[i + 1]`.
///   A query finds the occupied cells of a column by binary search, which is slower than the
///   dense layout's direct lookup but needs memory only for the items.
/// Both layouts visit the items in the same order. Queries only read, so any number of threads
/// may run them at once, as long as nothing updates or adds in the meantime.
template <typename Value>
class NeighborhoodSearch
{
    /// An item with its position in plan stored next to it, so that a query tests distances
    /// without touching the item.
    struct Entry {
        Point position;
        const Value* item;
    };

    /// Column (x) and row (y) of a cell.
    struct Cell {
        std::int32_t column;
        std::int32_t row;
    };

    double _cell_size;
    /// Cell coordinates of the grid's first cell and its size in cells. No cells while empty.
    std::int32_t _first_column{0};
    std::int32_t _first_row{0};
    std::int32_t _column_count{0};
    std::int32_t _row_count{0};
    bool _sparse{false};
    /// Where the entries of every cell (dense) or occupied cell (sparse) start, plus one last
    /// element that is the number of entries.
    std::vector<std::uint32_t> _cell_start{0};
    /// Sparse only: the key of every occupied cell, ascending.
    std::vector<std::uint64_t> _cell_keys{};
    std::vector<Entry> _entries{};
    /// Items added since they were last sorted in. Searched one by one, so they are sorted in
    /// once there are more than `max_added`.
    std::vector<Entry> _added{};
    /// Scratch: the stored and added entries to sort in together, the cell index (dense) or the
    /// key and position (sparse) of every entry being sorted in. Kept to reuse their memory.
    std::vector<Entry> _unsorted{};
    std::vector<std::uint32_t> _unsorted_cells{};
    std::vector<std::pair<std::uint64_t, std::uint32_t>> _unsorted_keys{};

    /// Adding many items without an update, e.g. all agents before the first iteration, would
    /// otherwise search all added items in every query.
    static constexpr std::size_t max_added = 1000;
    /// The dense layout is used up to this many cells per item, plus `spare_dense_cells`.
    static constexpr std::size_t dense_cells_per_item = 4;
    static constexpr std::size_t spare_dense_cells = 65536;
    /// Cell coordinates are clamped to this, so that the size of the grid in cells fits an int32.
    static constexpr double max_cell_coordinate = 1e9;

    /// The cell coordinate of @p cells, clamped like that of a stored item.
    static double clamped_cell(double cells)
    {
        return std::clamp(std::floor(cells), -max_cell_coordinate, max_cell_coordinate);
    }

    static std::int32_t to_cell(double cells)
    {
        return static_cast<std::int32_t>(clamped_cell(cells));
    }

    Cell cell_of(const Point& pos) const
    {
        return {to_cell(pos.x() / _cell_size), to_cell(pos.y() / _cell_size)};
    }

    std::uint32_t cell_index(Cell cell) const
    {
        return static_cast<std::uint32_t>(cell.column - _first_column) *
                   static_cast<std::uint32_t>(_row_count) +
               static_cast<std::uint32_t>(cell.row - _first_row);
    }

    /// Orders cells column by column, like the dense numbering.
    std::uint64_t cell_key(Cell cell) const
    {
        const auto column = std::int64_t{cell.column} - _first_column;
        const auto row = std::int64_t{cell.row} - _first_row;
        return (static_cast<std::uint64_t>(column) << 32) | static_cast<std::uint64_t>(row);
    }

    /// Replaces the stored entries with the entries @p to_entry makes of @p elements, sorted into
    /// the cells so that their order within a cell is kept.
    template <typename Range, typename ToEntry>
    void sort_in(const Range& elements, ToEntry to_entry)
    {
        const auto count = std::size(elements);
        _entries.resize(count);
        if(count == 0) {
            _column_count = 0;
            _row_count = 0;
            _sparse = false;
            _cell_start.assign(1, 0);
            return;
        }

        // The grid spans the bounding box of the entries' cells.
        Cell low{
            std::numeric_limits<std::int32_t>::max(), std::numeric_limits<std::int32_t>::max()};
        Cell high{
            std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::min()};
        for(const auto& element : elements) {
            const auto position = to_entry(element).position;
            if(!std::isfinite(position.x()) || !std::isfinite(position.y())) {
                throw SimulationError(
                    "Cannot search neighbors of position ({}, {})", position.x(), position.y());
            }
            const Cell cell = cell_of(position);
            low = {std::min(low.column, cell.column), std::min(low.row, cell.row)};
            high = {std::max(high.column, cell.column), std::max(high.row, cell.row)};
        }
        _first_column = low.column;
        _first_row = low.row;
        _column_count = high.column - low.column + 1;
        _row_count = high.row - low.row + 1;
        const auto cell_count = static_cast<std::size_t>(_column_count) * _row_count;
        _sparse = cell_count > dense_cells_per_item * count + spare_dense_cells;
        if(_sparse) {
            sort_in_sparse(elements, to_entry);
        } else {
            sort_in_dense(elements, to_entry, cell_count);
        }
    }

    /// Sorts with a counting sort over all cells.
    template <typename Range, typename ToEntry>
    void sort_in_dense(const Range& elements, ToEntry to_entry, std::size_t cell_count)
    {
        _cell_keys.clear();
        // Count the entries of every cell, then sum up: `_cell_start[c]` is where cell c ends.
        _cell_start.assign(cell_count + 1, 0);
        _unsorted_cells.resize(std::size(elements));
        std::size_t index = 0;
        for(const auto& element : elements) {
            const auto cell = cell_index(cell_of(to_entry(element).position));
            _unsorted_cells[index++] = cell;
            ++_cell_start[cell];
        }
        for(std::size_t cell = 1; cell <= cell_count; ++cell) {
            _cell_start[cell] += _cell_start[cell - 1];
        }
        // Fill every cell from its end, last entry first: this keeps the entries' order within a
        // cell and leaves `_cell_start[c]` where cell c begins.
        auto element = std::rbegin(elements);
        for(std::size_t reverse = std::size(elements); reverse > 0; --reverse, ++element) {
            const auto slot = --_cell_start[_unsorted_cells[reverse - 1]];
            _entries[slot] = to_entry(*element);
        }
    }

    /// Sorts by cell key, then lists the occupied cells.
    template <typename Range, typename ToEntry>
    void sort_in_sparse(const Range& elements, ToEntry to_entry)
    {
        _unsorted_keys.clear();
        std::uint32_t index = 0;
        for(const auto& element : elements) {
            _unsorted_keys.emplace_back(cell_key(cell_of(to_entry(element).position)), index++);
        }
        // Equal keys are ordered by position in `elements`, which keeps the order within a cell.
        std::sort(std::begin(_unsorted_keys), std::end(_unsorted_keys));
        _cell_keys.clear();
        _cell_start.clear();
        for(std::uint32_t slot = 0; slot < _unsorted_keys.size(); ++slot) {
            const auto [key, position] = _unsorted_keys[slot];
            _entries[slot] = to_entry(elements[position]);
            if(_cell_keys.empty() || _cell_keys.back() != key) {
                _cell_keys.push_back(key);
                _cell_start.push_back(slot);
            }
        }
        _cell_start.push_back(static_cast<std::uint32_t>(_unsorted_keys.size()));
    }

public:
    explicit NeighborhoodSearch(double cell_size) : _cell_size(cell_size) {};

    /// Makes @p item visible to queries. It must stay where it is in memory until the next
    /// update.
    void add_agent(const Value& item)
    {
        _added.push_back({item.location.xy(), &item});
        if(_added.size() > max_added) {
            // Sorted in behind the stored entries, so within a cell the added items come last.
            _unsorted.assign(std::begin(_entries), std::end(_entries));
            _unsorted.insert(std::end(_unsorted), std::begin(_added), std::end(_added));
            _added.clear();
            sort_in(_unsorted, [](const Entry& entry) { return entry; });
        }
    }

    /// Sorts @p items into the cells, keeping their order within a cell. Replaces everything
    /// stored before, including added items.
    void update(const AgentContainer<Value>& items)
    {
        _added.clear();
        sort_in(items, [](const Value& item) { return Entry{item.location.xy(), &item}; });
    }

    /// Calls 'fn' for every item within 'radius' of 'pos'.
    template <std::invocable<const Value&> Fn>
    void for_each_in_range(Point pos, double radius, Fn&& fn) const
    {
        const auto radius_squared = radius * radius;
        const auto visit_all = [&](std::uint32_t begin, std::uint32_t end) {
            for(auto entry = begin; entry < end; ++entry) {
                if(distance_squared(_entries[entry].position, pos) <= radius_squared) {
                    fn(*_entries[entry].item);
                }
            }
        };

        if(_column_count > 0) {
            // The cells within reach, clipped to the grid. Clipped as doubles: for a large radius
            // the unclipped cells need not fit an int32.
            const double reach = std::ceil(radius / _cell_size);
            const double column = clamped_cell(pos.x() / _cell_size);
            const double row = clamped_cell(pos.y() / _cell_size);
            const double last_column = _first_column + _column_count - 1;
            const double last_row = _first_row + _row_count - 1;
            const double column_min = std::max<double>(column - reach, _first_column);
            const double column_max = std::min(column + reach, last_column);
            const double row_min = std::max<double>(row - reach, _first_row);
            const double row_max = std::min(row + reach, last_row);
            if(column_min <= column_max && row_min <= row_max) {
                const auto first = static_cast<std::int32_t>(column_min);
                const auto last = static_cast<std::int32_t>(column_max);
                const auto low = static_cast<std::int32_t>(row_min);
                const auto high = static_cast<std::int32_t>(row_max);
                if(_sparse) {
                    // The columns come in key order, so each search can start where the last
                    // one ended.
                    auto from = std::begin(_cell_keys);
                    for(std::int32_t x = first; x <= last; ++x) {
                        const auto begin =
                            std::lower_bound(from, std::end(_cell_keys), cell_key({x, low}));
                        from = std::upper_bound(begin, std::end(_cell_keys), cell_key({x, high}));
                        visit_all(
                            _cell_start[begin - std::begin(_cell_keys)],
                            _cell_start[from - std::begin(_cell_keys)]);
                    }
                } else {
                    for(std::int32_t x = first; x <= last; ++x) {
                        visit_all(
                            _cell_start[cell_index({x, low})],
                            _cell_start[cell_index({x, high}) + 1]);
                    }
                }
            }
        }
        for(const auto& entry : _added) {
            if(distance_squared(entry.position, pos) <= radius_squared) {
                fn(*entry.item);
            }
        }
    }
};
