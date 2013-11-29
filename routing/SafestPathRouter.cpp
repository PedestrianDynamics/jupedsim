/**
 * @file    SafestPathRouter.cpp
 * Created on: 29.11.2013
 * Copyright (C) <2009-2013>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * This class computes the safest path for pedestrians based on information
 * received from FDS
 *
 */


#include "SafestPathRouter.h"

SafestPathRouter::SafestPathRouter() {

}

SafestPathRouter::~SafestPathRouter() {

}

void SafestPathRouter::Init(Building* building) {
	//Load the FDS file info

	//handle over to the global router engine
	GlobalRouter::Init(building);
}


int SafestPathRouter::FindExit(Pedestrian* p) {
	if(ComputeSafestPath(p)==-1) {
		Log->Write("");
	}
	//handle over to the global router engine
	return GlobalRouter::FindExit(p);
}


int SafestPathRouter::ComputeSafestPath(Pedestrian* p) {

	return -1;
}
