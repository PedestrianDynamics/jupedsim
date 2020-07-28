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

#include <RandomToolset.h>

AgentsParameters::AgentsParameters(int id, int seed)
{
    _id = id;
}

AgentsParameters::~AgentsParameters() {}

int AgentsParameters::GetID()
{
    return _id;
}

void AgentsParameters::InitV0(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _v0Mean  = mean;
    _v0Sigma = stdv;
}

void AgentsParameters::InitV0UpStairs(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _v0UpStairsMean  = mean;
    _v0UpStairsSigma = stdv;
}

void AgentsParameters::InitV0DownStairs(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _v0DownStairsMean  = mean;
    _v0DownStairsSigma = stdv;
}

void AgentsParameters::InitEscalatorUpStairs(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _v0EscalatorUpStairsMean  = mean;
    _v0EscalatorUpStairsSigma = stdv;
}

void AgentsParameters::InitEscalatorDownStairs(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _v0EscalatorDownStairsMean  = mean;
    _v0EscalatorDownStairsSigma = stdv;
}

void AgentsParameters::InitV0IdleEscalatorUpStairs(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _v0IdleEscalatorUpStairsMean  = mean;
    _v0IdleEscalatorUpStairsSigma = stdv;
}

void AgentsParameters::InitV0IdleEscalatorDownStairs(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _v0IdleEscalatorDownStairsMean  = mean;
    _v0IdleEscalatorDownStairsSigma = stdv;
}

void AgentsParameters::InitBmax(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _bMaxMean  = mean;
    _bMaxSigma = stdv;
}

void AgentsParameters::InitBmin(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _bMinMean  = mean;
    _bMinSigma = stdv;
}

void AgentsParameters::InitAmin(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _aMinMean  = mean;
    _aMinSigma = stdv;
}

void AgentsParameters::InitAtau(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _aTauMean  = mean;
    _aTauSigma = stdv;
}

void AgentsParameters::InitTau(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _tauMean  = mean;
    _tauSigma = stdv;
}

void AgentsParameters::InitT(double mean, double stdv)
{
    if(stdv == 0) {
        stdv = judge;
    }
    _tMean  = mean;
    _tSigma = stdv;
}


void AgentsParameters::EnableStretch(bool stretch)
{
    _enableStretch = stretch;
}


double AgentsParameters::GetV0()
{
    if(_v0Sigma == judge) {
        return _v0Mean;
    } else {
        return Random::GetNormal(_v0Mean, _v0Sigma);
    }
}

double AgentsParameters::GetV0UpStairs()
{
    if(_v0UpStairsSigma == judge) {
        return _v0UpStairsMean;
    } else {
        return Random::GetNormal(_v0UpStairsMean, _v0UpStairsSigma);
    }
}

double AgentsParameters::GetV0DownStairs()
{
    if(_v0DownStairsSigma == judge) {
        return _v0DownStairsMean;
    } else {
        return Random::GetNormal(_v0DownStairsMean, _v0DownStairsSigma);
    }
}

double AgentsParameters::GetEscalatorUpStairs()
{
    if(_v0EscalatorUpStairsSigma == judge) {
        return _v0EscalatorUpStairsMean;
    } else {
        return Random::GetNormal(_v0EscalatorUpStairsMean, _v0EscalatorUpStairsSigma);
    }
}

double AgentsParameters::GetEscalatorDownStairs()
{
    if(_v0EscalatorDownStairsSigma == judge) {
        return _v0EscalatorDownStairsMean;
    } else {
        return Random::GetNormal(_v0EscalatorDownStairsMean, _v0EscalatorDownStairsSigma);
    }
}


double AgentsParameters::GetV0IdleEscalatorUpStairs()
{
    if(_v0IdleEscalatorUpStairsSigma == judge) {
        return _v0IdleEscalatorUpStairsMean;
    } else {
        return Random::GetNormal(_v0IdleEscalatorUpStairsMean, _v0IdleEscalatorUpStairsSigma);
    }
}

double AgentsParameters::GetV0IdleEscalatorDownStairs()
{
    if(_v0IdleEscalatorDownStairsSigma == judge) {
        return _v0IdleEscalatorDownStairsMean;
    } else {
        return Random::GetNormal(_v0IdleEscalatorDownStairsMean, _v0IdleEscalatorDownStairsSigma);
    }
}


double AgentsParameters::GetBmax()
{
    if(_bMaxSigma == judge) {
        return _bMaxMean;
    } else {
        return Random::GetNormal(_bMaxMean, _bMaxSigma);
    }
}

double AgentsParameters::GetBmin()
{
    if(_bMinSigma == judge) {
        return _bMinMean;
    } else {
        return Random::GetNormal(_bMinMean, _bMinSigma);
    }
}

double AgentsParameters::GetAtau()
{
    if(_aTauSigma == judge) {
        return _aTauMean;
    } else {
        return Random::GetNormal(_aTauMean, _aTauSigma);
    }
}

double AgentsParameters::GetAmin()
{
    if(_aMinSigma == judge) {
        return _aMinMean;
    } else {
        return Random::GetNormal(_aMinMean, _aMinSigma);
    }
}

double AgentsParameters::GetTau()
{
    if(_tauSigma == judge) {
        return _tauMean;
    } else {
        return Random::GetNormal(_tauMean, _tauSigma);
    }
}

double AgentsParameters::GetT()
{
    if(_tSigma == judge) {
        return _tMean;
    } else {
        return Random::GetNormal(_tMean, _tSigma);
    }
}

bool AgentsParameters::StretchEnabled()
{
    return _enableStretch;
}

std::string AgentsParameters::writeParameter()
{
    std::string s;
    char tmp[1024];

    s.append("\tPedestrians Parameter:\n");
    if(_v0Sigma == judge) {
        sprintf(tmp, "\t\tv0 ~ N(%f, %f)\n", _v0Mean, _v0Sigma - judge);
    } else {
        sprintf(tmp, "\t\tv0 ~ N(%f, %f)\n", _v0Mean, _v0Sigma);
    }
    s.append(tmp);
    //    if(_Bmax.stddev() == judge) {
    //        sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Bmax.mean(), _Bmax.stddev() - judge);
    //    } else {
    //        sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Bmax.mean(), _Bmax.stddev());
    //    }
    //    s.append(tmp);
    //    if(_Bmin.stddev() == judge) {
    //        sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Bmin.mean(), _Bmin.stddev() - judge);
    //    } else {
    //        sprintf(tmp, "\t\tb_min ~ N(%f, %f)\n", _Bmin.mean(), _Bmin.stddev());
    //    }
    //    s.append(tmp);
    //    if(_Amin.stddev() == judge) {
    //        sprintf(tmp, "\t\tb_max ~ N(%f, %f)\n", _Amin.mean(), _Amin.stddev() - judge);
    //    } else {
    //        sprintf(tmp, "\t\tb_min ~ N(%f, %f)\n", _Amin.mean(), _Amin.stddev());
    //    }
    //    s.append(tmp);
    //    if(_Atau.stddev() == judge) {
    //        sprintf(tmp, "\t\ta_tau ~ N(%f, %f)\n", _Atau.mean(), _Atau.stddev() - judge);
    //    } else {
    //        sprintf(tmp, "\t\ta_tau ~ N(%f, %f)\n", _Atau.mean(), _Atau.stddev());
    //    }
    //    s.append(tmp);
    //    if(_Tau.stddev() == judge) {
    //        sprintf(tmp, "\t\ttau ~ N(%f, %f)\n", _Tau.mean(), _Tau.stddev() - judge);
    //    } else {
    //        sprintf(tmp, "\t\ttau ~ N(%f, %f)\n", _Tau.mean(), _Tau.stddev());
    //    }
    //    s.append(tmp);
    //    if(_T.stddev() == judge) {
    //        sprintf(tmp, "\t\tT ~ N(%f, %f)\n", _T.mean(), _T.stddev() - judge);
    //    } else {
    //        sprintf(tmp, "\t\tT ~ N(%f, %f)\n", _T.mean(), _T.stddev());
    //    }
    //    s.append(tmp);
    return s;
}

double AgentsParameters::GetAminMean()
{
    return _aMinMean;
}

double AgentsParameters::GetBmaxMean()
{
    return _bMaxMean;
}
