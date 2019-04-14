//
// Created by arne on 3/29/17.
//
/**
 * \file        FFKit.h
 * \date        Mar 29, 2017
 * \version
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * \section Description
 * The Floorfield Kit is the rework of the prototype FloorfieldViaFM - class.
 * It will restructur the former version to avoid several downsides (diamond
 * inheritance, line/centerpoint version (DRY), overdoing the precalc, ...)
 * We try to build it in a cleaner, easier version.
 *
 * A floorfield will be calculated and stored. The user can access distances(cost)
 * values as well as the gradient of that.
 *
 * Cost Values are calculated via the fastmarching algorithm. Other algorithms might
 * follow, if they provide better abilities to work in paralell.
 *
 * The new grid-code/subroom version should be reused for all floorfields, that work
 * on the same part of the geometry (room/subroom e.g. floor/room)
 *
 **/

#include "FFKit.h"
