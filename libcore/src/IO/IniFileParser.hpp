/**
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

#include "Area.hpp"
#include "general/Configuration.hpp"
#include "geometry/Point.hpp"

#include <tinyxml.h>

#include <filesystem>
#include <vector>

Configuration ParseIniFile(const std::filesystem::path& path);

// <areas>
//     <area id="1">
//         <lables>
//             <label>exit</label>
//         </lables>
//         <polygon>
//             <vertex px="70" py="101" />
//             <vertex px="70" py="103" />
//             <vertex px="75" py="103" />
//             <vertex px="75" py="101" />
//         </polygon>
//     </area>
// </areas>
std::map<Area::Id, Area> ParseAreas(const TiXmlElement* node);
