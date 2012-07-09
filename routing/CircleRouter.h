/**
 * @file    CircleRouter.h
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: Jan 29, 2012
 * Copyright (C) <2009-2012>
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
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
 * @section DESCRIPTION
 *
 *
 *
 */

#ifndef CIRCLEROUTER_H_
#define CIRCLEROUTER_H_

#include "GlobalRouter.h"
#include "AccessPoint.h"
#include "Routing.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"


//log output
extern OutputHandler* Log;


class CircleRouter: public GlobalRouter {
public:
	CircleRouter();
	virtual ~CircleRouter();

	virtual int FindExit(Pedestrian* ped);
	virtual void Init(Building* building);
};

#endif /* CIRCLEROUTER_H_ */
