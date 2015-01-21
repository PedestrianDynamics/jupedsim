/**
 * \file        Knowledge.cpp
 * \date        Sep 18, 2012
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/


#include "Knowledge.h"
#include "../IO/OutputHandler.h"
extern OutputHandler* Log;

Knowledge::Knowledge()
{
     _isClosed = false;
     _time = 0;
     _quality=1;
     _id=-1;
}

Knowledge::~Knowledge()
{
}

void Knowledge::Dump()
{
     Log->Write("INFO: \tdoor [%d] state [%d]  since [%f]",_id,_isClosed,_time);
}

void Knowledge::SetState(int id, bool is_closed, double time)
{
     _isClosed=is_closed;
     _time=time;
     _id=id;
}

bool Knowledge::GetState() const
{
     return _isClosed;
}

double Knowledge::GetQuality() const
{
     return _quality;
}

double Knowledge::GetTime() const
{
     return _time;
}
