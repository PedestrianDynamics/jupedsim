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
    using size_type          = std::size_t;
    using value_type         = T;
    using row_type           = container_type<value_type>;
    using row_container_type = container_type<row_type>;

    Grid2D() = default;

    Grid2D(const Grid2D &) = default;
    Grid2D(Grid2D &&)      = default;

    Grid2D & operator=(const Grid2D &) = default;
    Grid2D & operator=(Grid2D &&) = default;

    Grid2D(size_type num_rows, size_type num_columns) : _rows(num_rows, row_type(num_columns)) {}

    row_type const & operator[](size_type row) const { return _rows[row]; }
    row_type & operator[](size_type row)
    {
        return const_cast<row_type &>(const_cast<const Grid2D *>(this)->operator[](row));
    }

    void clear()
    {
        for(auto & row : _rows) {
            for(auto & elem : row) {
                elem.clear();
            }
        }
    }

    size_type num_rows() const { return _rows.size(); }
    size_type num_columns() const
    {
        if(_rows.empty()) {
            return 0;
        } else {
            return _rows.front().size();
        }
    }


private:
    row_container_type _rows;
};
