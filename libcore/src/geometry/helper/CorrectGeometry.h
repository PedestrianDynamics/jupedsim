/**
 * \copyright   <2009-2019> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/
#pragma once

class Building;

namespace geometry::helper
{
/**
 * \ingroup
 * Correct geometries by deleting "big" walls
 * In a subroom, "big" refers to a wall that intersects with other walls in the same subroom in a point,
 * which does not coincide with one of the end points.
 * For example:
 *
 *```
 *                            C
 *  A x------------------------o-----------------x B
 *                ^            |
 *                wall1        |
 *                             |  <--- wall2
 *                             o
 *                             D
 *```
 *  Here wall `[AB]` is a big wall, then it intersects wall `[CD]` whether in A nor in B
 *  What happens in this method:
 *  1. `[AB]` will be splited in two lines `[AC]` and `[CB]`
 *  2. `[AB]` will be removed
 *  3. `[AC]` or `[CB]` will be added to the subroom
 *
 */
void CorrectInputGeometry(Building & building);

} // namespace geometry::helper
