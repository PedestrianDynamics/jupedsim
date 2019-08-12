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

#include "general/Macros.h"

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
	if (stdv == 0)
	{
		stdv = judge;
	}
    _V0 = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitV0UpStairs(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
     _V0UpStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitV0DownStairs(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
     _V0DownStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitEscalatorUpStairs(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
     _EscalatorUpStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitEscalatorDownStairs(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
     _EscalatorDownStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitV0IdleEscalatorUpStairs(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
     _V0IdleEscalatorUpStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitV0IdleEscalatorDownStairs(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
     _V0IdleEscalatorDownStairs = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitBmax(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
    _Bmax= std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitBmin(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
    _Bmin = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitAmin(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
    _Amin = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitAtau(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
    _Atau = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitTau(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
    _Tau = std::normal_distribution<double>(mean,stdv);
}

void AgentsParameters::InitT(double mean, double stdv)
{
	if (stdv == 0)
	{
		stdv = judge;
	}
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
	if (_V0.stddev() == judge)
	{
		return _V0.mean();
	}
	else
	{
		return _V0(_generator);
	}
}

double AgentsParameters::GetV0UpStairs()
{
	if (_V0UpStairs.stddev() == judge)
	{
		return _V0UpStairs.mean();
	}
	else
	{
		return _V0UpStairs(_generator);
	}
}

double AgentsParameters::GetV0DownStairs()
{
	if (_V0DownStairs.stddev() == judge)
	{
		return _V0DownStairs.mean();
	}
	else
	{
		return _V0DownStairs(_generator);
	}
}

double AgentsParameters::GetEscalatorUpStairs()
{
	if (_EscalatorUpStairs.stddev() == judge)
	{
		return _EscalatorUpStairs.mean();
	}
	else
	{
		return _EscalatorUpStairs(_generator);
	}
}

double AgentsParameters::GetEscalatorDownStairs()
{

	if (_EscalatorDownStairs.stddev() == judge)
	{
		return _EscalatorDownStairs.mean();
	}
	else
	{
		return _EscalatorDownStairs(_generator);
	}
}


double AgentsParameters::GetV0IdleEscalatorUpStairs()
{
	if (_V0IdleEscalatorUpStairs.stddev() == judge)
	{
		return _V0IdleEscalatorUpStairs.mean();
	}
	else
	{
		return _V0IdleEscalatorUpStairs(_generator);
	}
}

double AgentsParameters::GetV0IdleEscalatorDownStairs()
{
	if (_V0IdleEscalatorDownStairs.stddev() == judge)
	{
		return _V0IdleEscalatorDownStairs.mean();
	}
	else
	{
		return _V0IdleEscalatorDownStairs(_generator);
	}
}


double AgentsParameters::GetBmax()
{
	if (_Bmax.stddev() == judge)
	{
		return _Bmax.mean();
	}
	else
	{
		return _Bmax(_generator);
	}
}

double AgentsParameters::GetBmin()
{
	if (_Bmin.stddev() == judge)
	{
		return _Bmin.mean();
	}
	else
	{
		return _Bmin(_generator);
	}
}

double AgentsParameters::GetAtau()
{
	if (_Atau.stddev() == judge)
	{
		return _Atau.mean();
	}
	else
	{
		return _Atau(_generator);
	}
}

double AgentsParameters::GetAmin()
{
	if (_Amin.stddev() == judge)
	{
		return  _Amin.mean();
	}
	else
	{
		return  _Amin(_generator);
	}
}

double AgentsParameters::GetTau()
{
	if (_Tau.stddev() == judge)
	{
		return _Tau.mean();
	}
	else
	{
		return _Tau(_generator);
	}
}

double AgentsParameters::GetT()
{
	if (_T.stddev() == judge)
	{
		return _T.mean();
	}
	else
	{
		return _T(_generator);
	}
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
	if (_V0.stddev() == judge)
	{
		sprintf(tmp, "\t\tv0 ~ N(%f, %f)\n", _V0.mean(), _V0.stddev()-judge);
	}
	else
	{
		sprintf(tmp, "\t\tv0 ~ N(%f, %f)\n", _V0.mean(), _V0.stddev());
	}
    s.append(tmp);
	if (_Bmax.stddev() == judge)
	{
		sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Bmax.mean(), _Bmax.stddev()-judge);
	}
	else
	{
		sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Bmax.mean(), _Bmax.stddev());
	}
    s.append(tmp);
	if (_Bmin.stddev() == judge)
	{
		sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Bmin.mean(), _Bmin.stddev() - judge);
	}
	else
	{
		sprintf(tmp, "\t\tb_min ~ N(%f, %f)\n", _Bmin.mean(), _Bmin.stddev());
	}
    s.append(tmp);
	if (_Amin.stddev() == judge)
	{
		sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Amin.mean(), _Amin.stddev() - judge);
	}
	else
	{
		sprintf(tmp, "\t\tb_min ~ N(%f, %f)\n", _Amin.mean(), _Amin.stddev());
	}
    s.append(tmp);
	if (_Atau.stddev() == judge)
	{
		sprintf(tmp, "\t\ta_tau ~ N(%f, %f)\n", _Atau.mean(), _Atau.stddev()-judge);
	}
	else
	{
		sprintf(tmp, "\t\ta_tau ~ N(%f, %f)\n", _Atau.mean(), _Atau.stddev());
	}
    s.append(tmp);
	if (_Tau.stddev() == judge)
	{
		sprintf(tmp, "\t\ttau ~ N(%f, %f)\n", _Tau.mean(), _Tau.stddev()-judge);
	}
	else
	{
		sprintf(tmp, "\t\ttau ~ N(%f, %f)\n", _Tau.mean(), _Tau.stddev());
	}
    s.append(tmp);
	if (_T.stddev() == judge)
	{
		sprintf(tmp, "\t\tT ~ N(%f, %f)\n", _T.mean(), _T.stddev()-judge);
	}
	else
	{
		sprintf(tmp, "\t\tT ~ N(%f, %f)\n", _T.mean(), _T.stddev());
	}
    s.append(tmp);
    return s;
}

double AgentsParameters::GetAminMean() {
    return _Amin.mean();
}

double AgentsParameters::GetBmaxMean() {
    return _Bmax.mean();
}



