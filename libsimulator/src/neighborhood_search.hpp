// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "generic_agent.hpp"
#include "kernel.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

/// Finds the items within a radius of a point, by sorting them into square cells.
///
/// The cells span the bounding box of the items and are stored compressed: `_entries` holds all
/// items sorted by cell, and cell `c` holds the entries from `_cell_start[c]` up to (excluding)
/// `_cell_start[c + 1]`. Cells are numbered column by column, so the cells of one column that a
/// query covers are one contiguous range of entries. Queries only read, so any number of
/// threads may run them at once, as long as nothing updates or adds in the meantime.
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
    /// One more than there are cells; the last element is the number of entries.
    std::vector<std::uint32_t> _cell_start{0};
    std::vector<Entry> _entries{};
    /// Items added since the last update. Searched one by one until the next update sorts them
    /// in.
    std::vector<Entry> _added{};
    /// Scratch for update(): the cell index of every item. Kept to reuse its memory.
    std::vector<std::uint32_t> _item_cells{};

    Cell cell_of(const Point& pos) const
    {
        return {
            static_cast<std::int32_t>(std::floor(pos.x() / _cell_size)),
            static_cast<std::int32_t>(std::floor(pos.y() / _cell_size))};
    }

    std::uint32_t cell_index(Cell cell) const
    {
        return static_cast<std::uint32_t>(cell.column - _first_column) *
                   static_cast<std::uint32_t>(_row_count) +
               static_cast<std::uint32_t>(cell.row - _first_row);
    }

public:
    explicit NeighborhoodSearch(double cell_size) : _cell_size(cell_size) {};

    /// Makes @p item visible to queries until the next update. It must stay where it is in
    /// memory until then.
    void add_agent(const Value& item) { _added.push_back({item.location.xy(), &item}); }

    /// Sorts @p items into the cells with a counting sort, keeping their order within a cell.
    /// Replaces everything stored before, including added items.
    void update(const AgentContainer<Value>& items)
    {
        _added.clear();
        _entries.resize(items.size());
        if(items.empty()) {
            _column_count = 0;
            _row_count = 0;
            _cell_start.assign(1, 0);
            return;
        }

        // The grid spans the bounding box of the items' cells.
        Cell low = cell_of(items.front().location.xy());
        Cell high = low;
        for(const auto& item : items) {
            const Cell cell = cell_of(item.location.xy());
            low = {std::min(low.column, cell.column), std::min(low.row, cell.row)};
            high = {std::max(high.column, cell.column), std::max(high.row, cell.row)};
        }
        _first_column = low.column;
        _first_row = low.row;
        _column_count = high.column - low.column + 1;
        _row_count = high.row - low.row + 1;
        const auto cell_count = static_cast<std::size_t>(_column_count) * _row_count;

        // Count the items of every cell, then sum up: `_cell_start[c]` is where cell c ends.
        _cell_start.assign(cell_count + 1, 0);
        _item_cells.resize(items.size());
        std::size_t index = 0;
        for(const auto& item : items) {
            const auto cell = cell_index(cell_of(item.location.xy()));
            _item_cells[index++] = cell;
            ++_cell_start[cell];
        }
        for(std::size_t cell = 1; cell <= cell_count; ++cell) {
            _cell_start[cell] += _cell_start[cell - 1];
        }
        // Fill every cell from its end, last item first: this keeps the items' order within a
        // cell and leaves `_cell_start[c]` where cell c begins.
        auto item = std::rbegin(items);
        for(std::size_t reverse = items.size(); reverse > 0; --reverse, ++item) {
            const auto slot = --_cell_start[_item_cells[reverse - 1]];
            _entries[slot] = {item->location.xy(), &*item};
        }
    }

    /// Calls 'fn' for every item within 'radius' of 'pos'.
    template <std::invocable<const Value&> Fn>
    void for_each_in_range(Point pos, double radius, Fn&& fn) const
    {
        const auto radius_squared = radius * radius;
        const auto visit = [&](const Entry& entry) {
            if(distance_squared(entry.position, pos) <= radius_squared) {
                fn(*entry.item);
            }
        };

        if(_column_count > 0) {
            // The cells within reach, clipped to the grid. Clipped as doubles: for a large radius
            // or a point far off the grid, the unclipped cells need not fit an int32.
            const double reach = std::ceil(radius / _cell_size);
            const double column = std::floor(pos.x() / _cell_size);
            const double row = std::floor(pos.y() / _cell_size);
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
                for(std::int32_t x = first; x <= last; ++x) {
                    const auto begin = _cell_start[cell_index({x, low})];
                    const auto end = _cell_start[cell_index({x, high}) + 1];
                    for(auto entry = begin; entry < end; ++entry) {
                        visit(_entries[entry]);
                    }
                }
            }
        }
        for(const auto& entry : _added) {
            visit(entry);
        }
    }
};
