/**
 * \copyright   <2009-2020> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#pragma once

#include <cstddef>
#include <vector>

template <typename T>
class Grid2D
{
private:
    template <typename element_type>
    using container_type = std::vector<element_type>;

public:
    using size_type  = std::size_t;
    using value_type = T;
    using row_type   = container_type<value_type>;
    using grid_type  = container_type<row_type>;

    Grid2D() = delete;

    Grid2D(const Grid2D &) = delete;
    Grid2D(Grid2D &&)      = delete;

    Grid2D & operator=(const Grid2D &) = delete;
    Grid2D & operator=(Grid2D &&) = delete;

    Grid2D(size_type num_rows, size_type num_columns) :
        _grid(num_rows, row_type(num_columns)), _num_rows(num_rows), _num_columns(num_columns)
    {
    }

    row_type const & operator[](size_type row) const { return _grid[row]; }
    row_type & operator[](size_type row)
    {
        return const_cast<row_type &>(const_cast<const Grid2D *>(this)->operator[](row));
    }

    void clear()
    {
        _grid.clear();
        _grid.insert(_grid.begin(), _num_rows, row_type(_num_columns));
    }

private:
    grid_type _grid;
    size_type _num_rows, _num_columns;
};
