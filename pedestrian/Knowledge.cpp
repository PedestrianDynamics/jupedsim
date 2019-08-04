/**
 * \file        Knowledge.cpp
 * \date        Sep 18, 2012
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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

Knowledge::Knowledge()
{
     _isClosed = false;
     _time = 0;
     _quality=1;
     _id=-1;
     _hasBeenRefusedOnce=false;
     _latency=0.0;
}

Knowledge::~Knowledge()
{
}

std::string Knowledge::Dump() const
{
     char tmp[2048];
     sprintf(tmp,"door [%d] state [%d]  since [%.2f] sec. Refused= %d, Quality=%.2f, latency=%.2f",_id,_isClosed,_time,_hasBeenRefusedOnce,_quality,_latency);
     return std::string(tmp);
}

void Knowledge::SetState(int id, bool is_closed, double time, double quality, double latency, bool refuse)
{
     _isClosed=is_closed;
     _time=time;
     _id=id;
     _quality=quality;
     _hasBeenRefusedOnce=refuse;
     _latency=latency;
}

bool Knowledge::GetState() const
{
     return _isClosed;
}

double Knowledge::GetQuality() const
{
     return _quality;
}

void Knowledge::SetQuality(double quality)
{
     _quality=quality;
}

double Knowledge::GetTime() const
{
     return _time;
}

bool Knowledge::HasBeenRefused() const
{
     return _hasBeenRefusedOnce;
}

void Knowledge::Refuse(bool state)
{
     _hasBeenRefusedOnce=state;
}

bool Knowledge::CanBeForwarded() const
{
     return (_latency<=0);
}

void Knowledge::SetLatency(double latency)
{
     _latency=latency;
}

double Knowledge::GetLatency() const
{
     return _latency;
}

void Knowledge::DecreaseLatency(double minus)
{
     _latency = _latency - minus;
     if (_latency <= 0)
          _latency = 0.0;
}
