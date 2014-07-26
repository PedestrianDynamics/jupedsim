/**
 * \file        AgentsParameters.cpp
 * \date        Jul 4, 2014
 * \version     v0.5
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
 * This class contains the different force models dependent parameters for the agents.
 * They are used to defined different population group, for instance children, men, elderly...
 * It is done by specifying different sizes, desired velocities and reaction times
 *
 *
 **/


#include "AgentsParameters.h"
#include "../general/Macros.h"
//#include <random>

AgentsParameters::AgentsParameters(int id, int seed)
{
    _id=id;
    _generator = std::default_random_engine(seed);
    //std::mt19937 _gen(rd());
}

AgentsParameters::~AgentsParameters()
{
}

int AgentsParameters::GetID()
{
    return _id;
}

void AgentsParameters::InitV0(double mean, double stdv)
{
    _V0 = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitBmax(double mean, double stdv)
{
    _Bmax= std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitBmin(double mean, double stdv)
{
    _Bmin = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitAmin(double mean, double stdv)
{
    _Amin = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitAtau(double mean, double stdv)
{
    _Atau = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitTau(double mean, double stdv)
{
    _Tau = std::normal_distribution<double>(mean,stdv);
}

double AgentsParameters::GetV0()
{
    return _V0(_generator);
}

double AgentsParameters::GetBmax()
{
    return _Bmax(_generator);
}

double AgentsParameters::GetBmin()
{
    return _Bmin(_generator);
}

double AgentsParameters::GetAtau()
{
    return _Atau(_generator);
}

double AgentsParameters::GetAmin()
{
    return _Amin(_generator);
}

double AgentsParameters::GetTau()
{
    return _Tau(_generator);
}

std::string AgentsParameters::writeParameter()
{
    std::string s;
    char tmp[CLENGTH];

    s.append("\tPedestrians Parameter:\n");
    sprintf(tmp, "\t\tv0 ~ N(%f, %f)\n", _V0.mean(), _V0.stddev());
    s.append(tmp);
    sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n",  _Bmax.mean(), _Bmax.stddev());
    s.append(tmp);
    sprintf(tmp, "\t\tb_min ~ N(%f, %f)\n",  _Bmin.mean(), _Bmin.stddev());
    s.append(tmp);
    sprintf(tmp, "\t\ta_min ~ N(%f, %f)\n",  _Amin.mean(), _Amin.stddev());
    s.append(tmp);
    sprintf(tmp, "\t\ta_tau ~ N(%f, %f)\n",  _Atau.mean(), _Atau.stddev());
    s.append(tmp);
    sprintf(tmp, "\t\ttau ~ N(%f, %f)\n",  _Tau.mean(), _Tau.stddev());
    s.append(tmp);

    return s;
}
