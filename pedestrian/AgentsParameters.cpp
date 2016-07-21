/**
 * \file        AgentsParameters.cpp
 * \date        Jul 4, 2014
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

void AgentsParameters::InitV0UpStairs(double mean, double stdv)
{
     _V0UpStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitV0DownStairs(double mean, double stdv)
{
     _V0DownStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitEscalatorUpStairs(double mean, double stdv)
{
     _EscalatorUpStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitEscalatorDownStairs(double mean, double stdv)
{
     _EscalatorDownStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitV0IdleEscalatorUpStairs(double mean, double stdv)
{
     _V0IdleEscalatorUpStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitV0IdleEscalatorDownStairs(double mean, double stdv)
{
     _V0IdleEscalatorDownStairs = std::normal_distribution<double>(mean,stdv);
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

void AgentsParameters::InitT(double mean, double stdv)
{
    _T = std::normal_distribution<double>(mean,stdv);
}


void AgentsParameters::SetSwayParams(double freqA, double freqB, double ampA, double ampB) {
     _swayFreqA = freqA;
     _swayFreqB = freqB;
     _swayAmpA = ampA;
     _swayAmpB = ampB;
}

void AgentsParameters::EnableStretch(bool stretch)
{
    _enableStretch = stretch;
}



double AgentsParameters::GetV0()
{
    return _V0(_generator);
}

double AgentsParameters::GetV0UpStairs()
{
     return _V0UpStairs(_generator);
}

double AgentsParameters::GetV0DownStairs()
{
     return _V0DownStairs(_generator);
}

double AgentsParameters::GetEscalatorUpStairs()
{
     return _EscalatorUpStairs(_generator);
}

double AgentsParameters::GetEscalatorDownStairs()
{
     return _EscalatorDownStairs(_generator);
}


double AgentsParameters::GetV0IdleEscalatorUpStairs()
{
     return _V0IdleEscalatorUpStairs(_generator);
}

double AgentsParameters::GetV0IdleEscalatorDownStairs()
{
     return _V0IdleEscalatorDownStairs(_generator);
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

double AgentsParameters::GetT()
{
    return _T(_generator);
}

double AgentsParameters::GetSwayFreqA() const {
     return _swayFreqA;
}

double AgentsParameters::GetSwayFreqB() const {
     return _swayFreqB;
}

double AgentsParameters::GetSwayAmpA() const {
     return _swayAmpA;
}

double AgentsParameters::GetSwayAmpB() const {
     return _swayAmpB;
}

bool AgentsParameters::StretchEnabled()
{
     return _enableStretch;
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
    sprintf(tmp, "\t\tT ~ N(%f, %f)\n",  _T.mean(), _T.stddev());
    s.append(tmp);
    return s;
}



