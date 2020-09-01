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

AgentsParameters::AgentsParameters(int id)
{
    _id = id;
}

int AgentsParameters::GetID() const
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

double AgentsParameters::GetV0() const
{
    if(_v0Sigma == judge) {
        return _v0Mean;
    }
    return Random::GetNormal(_v0Mean, _v0Sigma);
}

double AgentsParameters::GetV0UpStairs() const
{
    if(_v0UpStairsSigma == judge) {
        return _v0UpStairsMean;
    }
    return Random::GetNormal(_v0UpStairsMean, _v0UpStairsSigma);
}

double AgentsParameters::GetV0DownStairs() const
{
    if(_v0DownStairsSigma == judge) {
        return _v0DownStairsMean;
    }
    return Random::GetNormal(_v0DownStairsMean, _v0DownStairsSigma);
}

double AgentsParameters::GetEscalatorUpStairs() const
{
    if(_v0EscalatorUpStairsSigma == judge) {
        return _v0EscalatorUpStairsMean;
    }
    return Random::GetNormal(_v0EscalatorUpStairsMean, _v0EscalatorUpStairsSigma);
}

double AgentsParameters::GetEscalatorDownStairs() const
{
    if(_v0EscalatorDownStairsSigma == judge) {
        return _v0EscalatorDownStairsMean;
    }
    return Random::GetNormal(_v0EscalatorDownStairsMean, _v0EscalatorDownStairsSigma);
}


double AgentsParameters::GetV0IdleEscalatorUpStairs() const
{
    if(_v0IdleEscalatorUpStairsSigma == judge) {
        return _v0IdleEscalatorUpStairsMean;
    }
    return Random::GetNormal(_v0IdleEscalatorUpStairsMean, _v0IdleEscalatorUpStairsSigma);
}

double AgentsParameters::GetV0IdleEscalatorDownStairs() const
{
    if(_v0IdleEscalatorDownStairsSigma == judge) {
        return _v0IdleEscalatorDownStairsMean;
    }
    return Random::GetNormal(_v0IdleEscalatorDownStairsMean, _v0IdleEscalatorDownStairsSigma);
}


double AgentsParameters::GetBmax() const
{
    if(_bMaxSigma == judge) {
        return _bMaxMean;
    }
    return Random::GetNormal(_bMaxMean, _bMaxSigma);
}

double AgentsParameters::GetBmin() const
{
    if(_bMinSigma == judge) {
        return _bMinMean;
    }
    return Random::GetNormal(_bMinMean, _bMinSigma);
}

double AgentsParameters::GetAtau() const
{
    if(_aTauSigma == judge) {
        return _aTauMean;
    }
    return Random::GetNormal(_aTauMean, _aTauSigma);
}

double AgentsParameters::GetAmin() const
{
    if(_aMinSigma == judge) {
        return _aMinMean;
    } else {
        return Random::GetNormal(_aMinMean, _aMinSigma);
    }
}

double AgentsParameters::GetTau() const
{
    if(_tauSigma == judge) {
        return _tauMean;
    }
    return Random::GetNormal(_tauMean, _tauSigma);
}

double AgentsParameters::GetT() const
{
    if(_tSigma == judge) {
        return _tMean;
    }
    return Random::GetNormal(_tMean, _tSigma);
}

bool AgentsParameters::StretchEnabled() const
{
    return _enableStretch;
}

double AgentsParameters::GetAminMean() const
{
    return _aMinMean;
}

double AgentsParameters::GetBmaxMean() const
{
    return _bMaxMean;
}
