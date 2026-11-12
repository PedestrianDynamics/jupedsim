// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "generic_agent.hpp"
#include "hash_combine.hpp"
#include "kernel.hpp"
#include "simulation_error.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename T>
T* as_ptr(T* t)
{
    return t;
}

template <typename T>
const T* as_ptr(const T* t)
{
    return t;
}

template <typename T>
T* as_ptr(T& t)
{
    return &t;
}

template <typename T>
const T* as_ptr(const T& t)
{
    return &t;
}

struct Grid2DIndex {
    std::int32_t idx;
    std::int32_t idy;

    bool operator<(const Grid2DIndex& other) const
    {
        return idx < other.idx || (idx == other.idx && idy < other.idy);
    }

    bool operator==(const Grid2DIndex& other) const { return idx == other.idx && idy == other.idy; }
};

template <>
struct std::hash<Grid2DIndex> {
    std::size_t operator()(const Grid2DIndex& id) const noexcept
    {
        std::hash<std::int32_t> hasher{};
        return jps::hash_combine(hasher(id.idx), hasher(id.idy));
    }
};

template <typename Value>
class NeighborhoodSearch
{
    struct Bounds {
        std::int32_t x0, x1, y0, y1;
    };

    double _cell_size;

    // Flat layout ("compressed rows"): the items of each occupied cell lie contiguously in
    // _items, in container order within a cell, which keeps the neighbour order -- and with it
    // every floating-point sum over neighbours -- the same as a cell-by-cell push_back.
    std::vector<const Value*> _items{};
    // Dense index over the bounding box of occupied cells: cell c holds
    // _items[_cell_begin[c], _cell_begin[c + 1]).
    std::vector<std::uint32_t> _cell_begin{};
    std::int32_t _x0{0};
    std::int32_t _y0{0};
    std::int32_t _nx{0};
    std::int32_t _ny{0};
    // Fallback for sparse crowds whose bounding box would be too large for a dense index.
    std::unordered_map<Grid2DIndex, std::pair<std::uint32_t, std::uint32_t>> _sparse{};
    bool _dense{true};
    // Added since the last update(); searched linearly until the next one.
    std::vector<const Value*> _pending{};
    // Scratch, reused between updates.
    std::vector<Grid2DIndex> _keys{};
    std::vector<std::uint32_t> _cursor{};
    std::vector<Bounds> _block_bounds{};
    std::vector<std::uint32_t> _block_counts{};

public:
    /// The cell @p pos falls into.
    Grid2DIndex get_index(const Point& pos) const
    {
        const int32_t idx = static_cast<int32_t>(pos.x() / _cell_size);
        const int32_t idy = static_cast<int32_t>(pos.y() / _cell_size);
        return Grid2DIndex{idx, idy};
    }

private:
    /// [begin, end) into _items for cell (x, y); empty if no item lies there.
    std::pair<std::uint32_t, std::uint32_t> cell_range(std::int32_t x, std::int32_t y) const
    {
        if(_dense) {
            const std::int32_t cx = x - _x0;
            const std::int32_t cy = y - _y0;
            if(cx < 0 || cy < 0 || cx >= _nx || cy >= _ny) {
                return {0, 0};
            }
            const auto c = static_cast<std::size_t>(cx) * static_cast<std::size_t>(_ny) +
                           static_cast<std::size_t>(cy);
            return {_cell_begin[c], _cell_begin[c + 1]};
        }
        const auto it = _sparse.find({x, y});
        return it == _sparse.end() ? std::pair<std::uint32_t, std::uint32_t>{0, 0} : it->second;
    }

public:
    explicit NeighborhoodSearch(double cell_size) : _cell_size(cell_size) {};

    /// Makes @p item findable until the next update(). The item must stay at its address.
    void add_agent(const Value& item) { _pending.push_back(&item); }

    /// Whether items were added since the last update(); the next iteration must rebuild then,
    /// so that neighbour order is the same as after a full rebuild.
    bool has_pending() const { return !_pending.empty(); }

    void remove_agent(const Value& item)
    {
        for(auto* list : {&_pending, &_items}) {
            const auto iter = std::find_if(list->begin(), list->end(), [&item](const Value* v) {
                return v != nullptr && v->id == item.id;
            });
            if(iter != list->end()) {
                *iter = nullptr;
                return;
            }
        }
        throw SimulationError("Unknown agent id {}", item.id);
    }

    /// Rebuilds the index from @p items. With @p pool the per-item cell lookups, which read
    /// every item's position, run in parallel; counting and scattering stay serial and only
    /// touch small, contiguous arrays.
    template <typename Pool = void>
    void update(const AgentContainer<Value>& items, Pool* pool = nullptr)
    {
        const std::size_t n = std::size(items);
        _pending.clear();
        _keys.resize(n);
        const auto key_of = [&](std::size_t i) {
            _keys[i] = get_index(as_ptr(items[i])->location.xy());
        };
        if constexpr(!std::is_void_v<Pool>) {
            if(pool != nullptr) {
                pool->parallel_for(n, std::max<std::size_t>(64, n / (pool->size() * 4)), key_of);
            } else {
                for(std::size_t i = 0; i < n; ++i) {
                    key_of(i);
                }
            }
        } else {
            for(std::size_t i = 0; i < n; ++i) {
                key_of(i);
            }
        }
        build_from_keys(items, pool);
    }

    /// Scratch for the cells of the next update_from_keys(): entry i is items[i]'s cell.
    /// Filling it while the items are written (e.g. right after moving an agent) saves reading
    /// every position again from wherever it was written.
    std::vector<Grid2DIndex>& keys_for(std::size_t n)
    {
        _keys.resize(n);
        return _keys;
    }

    /// Rebuilds from the cells stored through keys_for(); same result as update(items).
    template <typename Pool = void>
    void update_from_keys(const AgentContainer<Value>& items, Pool* pool = nullptr)
    {
        _pending.clear();
        build_from_keys(items, pool);
    }

private:
    /// Parallel counting sort into the dense layout: per-block bounds and per-block cell counts,
    /// a column-wise prefix over blocks (in parallel over cells), a serial prefix over cells,
    /// and a parallel scatter. Blocks are contiguous index ranges, so container order within a
    /// cell -- and with it the neighbour order -- is the same as the serial build's. Returns
    /// false (and does nothing) when the crowd is too small or too sparse for it to pay off.
    template <typename Pool>
    bool build_dense_parallel(const AgentContainer<Value>& items, Pool& pool)
    {
        // SPIKE: JPS_GRID_PAR_MIN is the crowd size from which this runs (default 2048).
        static const std::size_t min_items = [] {
            const char* env = std::getenv("JPS_GRID_PAR_MIN");
            return env != nullptr ? static_cast<std::size_t>(std::strtoul(env, nullptr, 10)) :
                                    std::size_t{2048};
        }();
        const std::size_t n = std::size(items);
        if(n < min_items || pool.size() < 2) {
            return false;
        }
        std::size_t blocks = std::min<std::size_t>(pool.size(), n / 256);
        if(blocks < 2) {
            return false;
        }
        const auto lo = [&](std::size_t b) { return b * n / blocks; };
        _block_bounds.resize(blocks);
        pool.parallel_for(blocks, 1, [&](std::size_t b) {
            Bounds bb{_keys[lo(b)].idx, _keys[lo(b)].idx, _keys[lo(b)].idy, _keys[lo(b)].idy};
            for(std::size_t i = lo(b); i < lo(b + 1); ++i) {
                bb.x0 = std::min(bb.x0, _keys[i].idx);
                bb.x1 = std::max(bb.x1, _keys[i].idx);
                bb.y0 = std::min(bb.y0, _keys[i].idy);
                bb.y1 = std::max(bb.y1, _keys[i].idy);
            }
            _block_bounds[b] = bb;
        });
        Bounds all = _block_bounds[0];
        for(const auto& bb : _block_bounds) {
            all.x0 = std::min(all.x0, bb.x0);
            all.x1 = std::max(all.x1, bb.x1);
            all.y0 = std::min(all.y0, bb.y0);
            all.y1 = std::max(all.y1, bb.y1);
        }
        const auto nx = static_cast<std::int64_t>(all.x1) - all.x0 + 1;
        const auto ny = static_cast<std::int64_t>(all.y1) - all.y0 + 1;
        if(nx * ny > static_cast<std::int64_t>(8 * n + 4096)) {
            return false; // sparse: the serial build's hash fallback handles it
        }
        const auto cells = static_cast<std::size_t>(nx * ny);
        // Keep the per-block count table small next to the crowd.
        blocks = std::min(blocks, std::max<std::size_t>(2, 4 * n / cells));
        if(blocks < 2) {
            return false;
        }
        _x0 = all.x0;
        _y0 = all.y0;
        _nx = static_cast<std::int32_t>(nx);
        _ny = static_cast<std::int32_t>(ny);
        _dense = true;
        _sparse.clear();
        _items.resize(n);
        const auto cell_of = [&](const Grid2DIndex& k) {
            return static_cast<std::size_t>(k.idx - _x0) * static_cast<std::size_t>(_ny) +
                   static_cast<std::size_t>(k.idy - _y0);
        };
        _block_counts.resize(blocks * cells);
        pool.parallel_for(blocks, 1, [&](std::size_t b) {
            auto* row = _block_counts.data() + b * cells;
            std::fill(row, row + cells, 0U);
            for(std::size_t i = lo(b); i < lo(b + 1); ++i) {
                ++row[cell_of(_keys[i])];
            }
        });
        // Column prefix: per cell, where each block's items start relative to the cell start.
        _cell_begin.assign(cells + 1, 0);
        pool.parallel_for(
            cells, std::max<std::size_t>(1024, cells / (pool.size() * 4)), [&](std::size_t c) {
                std::uint32_t running = 0;
                for(std::size_t b = 0; b < blocks; ++b) {
                    const auto count = _block_counts[b * cells + c];
                    _block_counts[b * cells + c] = running;
                    running += count;
                }
                _cell_begin[c + 1] = running;
            });
        for(std::size_t c = 0; c < cells; ++c) {
            _cell_begin[c + 1] += _cell_begin[c];
        }
        pool.parallel_for(blocks, 1, [&](std::size_t b) {
            auto* row = _block_counts.data() + b * cells;
            for(std::size_t i = lo(b); i < lo(b + 1); ++i) {
                const auto c = cell_of(_keys[i]);
                _items[_cell_begin[c] + row[c]++] = as_ptr(items[i]);
            }
        });
        return true;
    }

    template <typename Pool>
    void build_from_keys(const AgentContainer<Value>& items, Pool* pool)
    {
        if constexpr(!std::is_void_v<Pool>) {
            if(pool != nullptr && build_dense_parallel(items, *pool)) {
                return;
            }
        }
        const std::size_t n = std::size(items);
        _items.resize(n);
        _sparse.clear();
        if(n == 0) {
            _dense = true;
            _nx = _ny = 0;
            _cell_begin.assign(1, 0);
            return;
        }
        std::int32_t x0 = _keys[0].idx, x1 = x0, y0 = _keys[0].idy, y1 = y0;
        for(const auto& k : _keys) {
            x0 = std::min(x0, k.idx);
            x1 = std::max(x1, k.idx);
            y0 = std::min(y0, k.idy);
            y1 = std::max(y1, k.idy);
        }
        const auto nx = static_cast<std::int64_t>(x1) - x0 + 1;
        const auto ny = static_cast<std::int64_t>(y1) - y0 + 1;
        _dense = nx * ny <= static_cast<std::int64_t>(8 * n + 4096);
        if(_dense) {
            _x0 = x0;
            _y0 = y0;
            _nx = static_cast<std::int32_t>(nx);
            _ny = static_cast<std::int32_t>(ny);
            const auto cells = static_cast<std::size_t>(nx * ny);
            const auto cell_of = [&](const Grid2DIndex& k) {
                return static_cast<std::size_t>(k.idx - _x0) * static_cast<std::size_t>(_ny) +
                       static_cast<std::size_t>(k.idy - _y0);
            };
            _cell_begin.assign(cells + 1, 0);
            for(const auto& k : _keys) {
                ++_cell_begin[cell_of(k) + 1];
            }
            for(std::size_t c = 0; c < cells; ++c) {
                _cell_begin[c + 1] += _cell_begin[c];
            }
            _cursor.assign(_cell_begin.begin(), _cell_begin.end() - 1);
            for(std::size_t i = 0; i < n; ++i) {
                _items[_cursor[cell_of(_keys[i])]++] = as_ptr(items[i]);
            }
            return;
        }
        // Sparse: order item indices by cell, keeping container order within a cell.
        _cursor.resize(n);
        for(std::size_t i = 0; i < n; ++i) {
            _cursor[i] = static_cast<std::uint32_t>(i);
        }
        std::stable_sort(_cursor.begin(), _cursor.end(), [&](std::uint32_t a, std::uint32_t b) {
            return _keys[a] < _keys[b];
        });
        for(std::size_t j = 0; j < n;) {
            const auto key = _keys[_cursor[j]];
            const auto begin = static_cast<std::uint32_t>(j);
            for(; j < n && _keys[_cursor[j]] == key; ++j) {
                _items[j] = as_ptr(items[_cursor[j]]);
            }
            _sparse.emplace(key, std::pair{begin, static_cast<std::uint32_t>(j)});
        }
    }

public:
    /// Calls 'fn' for every item within 'radius' of 'pos'.
    template <std::invocable<const Value&> Fn>
    void for_each_in_range(Point pos, double radius, Fn&& fn) const
    {
        const auto pos_idx = get_index(pos);
        const auto offset = static_cast<int32_t>(std::ceil(radius / _cell_size));
        const int32_t x_min = pos_idx.idx - offset;
        const int32_t x_max = pos_idx.idx + offset;
        const int32_t y_min = pos_idx.idy - offset;
        const int32_t y_max = pos_idx.idy + offset;

        const auto radius_squared = radius * radius;

        for(int32_t x = x_min; x <= x_max; ++x) {
            for(int32_t y = y_min; y <= y_max; ++y) {
                const auto [begin, end] = cell_range(x, y);
                for(auto i = begin; i < end; ++i) {
                    const Value* item = _items[i];
                    if(item != nullptr &&
                       distance_squared(item->location.xy(), pos) <= radius_squared) {
                        fn(*item);
                    }
                }
            }
        }
        for(const Value* item : _pending) {
            if(item != nullptr && distance_squared(item->location.xy(), pos) <= radius_squared) {
                fn(*item);
            }
        }
    }
};
