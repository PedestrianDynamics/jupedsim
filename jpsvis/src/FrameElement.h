/**
 * Copyright (C) <2009-2014>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include <glm/vec3.hpp>

/// Represents a single entry for a single agent in a frame
struct FrameElement {
    glm::dvec3 pos;
    glm::dvec3 radius;
    glm::dvec3 orientation;
    double color;
    int id;
};
