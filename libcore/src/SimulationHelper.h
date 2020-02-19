/**
 * Copyright (c) 2020 Forschungszentrum Jülich GmbH. All rights reserved.
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
 **/
#pragma once

#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <optional>

namespace SimulationHelper
{
/**
     * Checks whether the pedestrian \p ped is still in the assigned room. If not check if \p ped is
     * in an neighbouring room/subroom and update the information. If no relocation is possible,
     * nullopt is returned. When this happens the pedestrian has not moved to one of the neighboring
     * rooms, but somewhere else.
     * @param building
     * @param ped Pedestrian, whose position is checked and information may be updated
     * @return true if relocation was successful, false if no reloaction was needed. Nullopt if
     * relocation failed.
     */
std::optional<bool> RelocatePedestrian(Building & building, Pedestrian & ped);
}; // namespace SimulationHelper
