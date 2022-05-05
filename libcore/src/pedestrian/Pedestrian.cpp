/**
 * \file        Pedestrian.cpp
 * \date        Sep 30, 2010
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
#include "Pedestrian.hpp"

#include "geometry/Building.hpp"
#include "geometry/Line.hpp"
#include "geometry/SubRoom.hpp"
#include "geometry/WaitingArea.hpp"

#include <Logger.hpp>
#include <cassert>

double Pedestrian::SelectV0(SubroomType type, double delta) const
{
    switch(type) {
        case SubroomType::ESCALATOR_UP:
        case SubroomType::ESCALATOR_DOWN:
            // TODO(kkratz): This needs to be reimplemented with areas, This should not be in
            // Pedestrian.
            assert(false);
        case SubroomType::STAIR:
            if(fabs(delta) < 1)
                return std::max(0., _ellipse.GetV0());
            else
                return (delta < 0) ? _v0DownStairs : _v0UpStairs;

        case SubroomType::FLOOR:
        case SubroomType::CORRIDOR:
        case SubroomType::ENTRANCE:
        case SubroomType::LOBBY:
        case SubroomType::DA:
        case SubroomType::UNKNOWN:
            return _ellipse.GetV0();
    }
}
double Pedestrian::SelectSmoothFactor(SubroomType type, double delta) const
{
    switch(type) {
        case SubroomType::ESCALATOR_UP:
            return _smoothFactorEscalatorUpStairs;
        case SubroomType::ESCALATOR_DOWN:
            return _smoothFactorEscalatorDownStairs;
        case SubroomType::STAIR:
            return (delta < 0) ? _smoothFactorDownStairs : _smoothFactorUpStairs;
        case SubroomType::FLOOR:
        case SubroomType::CORRIDOR:
        case SubroomType::ENTRANCE:
        case SubroomType::LOBBY:
        case SubroomType::DA:
        case SubroomType::UNKNOWN:
            return 15.0; /// magic number that works best so far!
    }
}

void Pedestrian::SetTau(double tau)
{
    _tau = tau;
}

void Pedestrian::SetT(double T)
{
    _t = T;
}

void Pedestrian::SetEllipse(const JEllipse& e)
{
    _ellipse = e;
}

void Pedestrian::SetPos(const Point& pos)
{
    _ellipse.SetCenter(pos);
}

void Pedestrian::SetV(const Point& v)
{
    _ellipse.SetV(v);
}

void Pedestrian::SetSmoothFactorUpStairs(double c)
{
    _smoothFactorUpStairs = c;
}

void Pedestrian::SetSmoothFactorDownStairs(double c)
{
    _smoothFactorDownStairs = c;
}

void Pedestrian::SetSmoothFactorEscalatorUpStairs(double c)
{
    _smoothFactorEscalatorUpStairs = c;
}

void Pedestrian::SetSmoothFactorEscalatorDownStairs(double c)
{
    _smoothFactorEscalatorDownStairs = c;
}

void Pedestrian::SetV0Norm(
    double v0,
    double v0UpStairs,
    double v0DownStairs,
    double escalatorUp,
    double escalatorDown)
{
    _ellipse.SetV0(v0);
    _v0DownStairs = v0DownStairs;
    _v0UpStairs = v0UpStairs;
    _v0EscalatorUpStairs = escalatorUp;
    _v0EscalatorDownStairs = escalatorDown;
}

void Pedestrian::SetDeltaT(double dt)
{
    _deltaT = dt;
}
Pedestrian::UID Pedestrian::GetUID() const
{
    return _uid;
}

double Pedestrian::GetMass() const
{
    return _mass;
}

double Pedestrian::GetTau() const
{
    return _tau;
}

double Pedestrian::GetT() const
{
    return _t;
}

const JEllipse& Pedestrian::GetEllipse() const
{
    return _ellipse;
}

const Point& Pedestrian::GetPos() const
{
    return _ellipse.GetCenter();
}

const Point& Pedestrian::GetV() const
{
    return _ellipse.GetV();
}

const Point& Pedestrian::GetV0() const
{
    return _v0;
}

double Pedestrian::GetV0Norm() const
{
    double smoothFactor = SelectSmoothFactor(SubroomType::FLOOR, 0);
    double v0 = SelectV0(SubroomType::FLOOR, 0);
    double f = 2.0 / (1 + exp(-smoothFactor)) - 1;
    double g = 2.0 / (1 + exp(-smoothFactor)) - 1;

    double walking_speed = (1 - f * g) * _ellipse.GetV0() + f * g * v0;
    return walking_speed;
}

// get axis in the walking direction
double Pedestrian::GetLargerAxis() const
{
    return _ellipse.GetEA();
}
// get axis in the shoulder direction = orthogonal to the walking direction
double Pedestrian::GetSmallerAxis() const
{
    return _ellipse.GetEB();
}

void Pedestrian::SetPhiPed()
{
    double cosPhi;
    double sinPhi;
    double vx = GetV().x;
    double vy = GetV().y;

    if(fabs(vx) > J_EPS || fabs(vy) > J_EPS) {
        double normv = sqrt(vx * vx + vy * vy);
        cosPhi = vx / normv;
        sinPhi = vy / normv;
    } else {
        cosPhi = GetEllipse().GetCosPhi();
        sinPhi = GetEllipse().GetSinPhi();
    }
    _ellipse.SetCosPhi(cosPhi);
    _ellipse.SetSinPhi(sinPhi);
}

void Pedestrian::InitV0(const Point& target)
{
    const Point& pos = GetPos();
    Point delta = target - pos;

    _v0 = delta.Normalized();
}

Point Pedestrian::GetV0(const Point& target) const
{
    // Molification around the targets makes little sense
    const Point& pos = GetPos();
    Point delta = target - pos;
    Point new_v0 = delta.Normalized();

    double t = _newOrientationDelay * _deltaT;

    // Handover new target
    return _v0 + (new_v0 - _v0) * (1 - exp(-t / _tau));
}

void Pedestrian::IncrementOrientationDelay()
{
    ++_newOrientationDelay;
}

void Pedestrian::SetSmoothTurning()
{
    _newOrientationDelay = 0;
}

int Pedestrian::GetGroup() const
{
    return _group;
}

void Pedestrian::SetGroup(int group)
{
    _group = group;
}

double Pedestrian::GetV0UpStairsNorm() const
{
    return _v0UpStairs;
}

double Pedestrian::GetV0DownStairsNorm() const
{
    return _v0DownStairs;
}

double Pedestrian::GetV0EscalatorUpNorm() const
{
    return _v0EscalatorUpStairs;
}

double Pedestrian::GetV0EscalatorDownNorm() const
{
    return _v0EscalatorDownStairs;
}

double Pedestrian::GetSmoothFactorUpStairs() const
{
    return _smoothFactorUpStairs;
}
double Pedestrian::GetSmoothFactorDownStairs() const
{
    return _smoothFactorDownStairs;
}
double Pedestrian::GetSmoothFactorUpEscalators() const
{
    return _smoothFactorEscalatorUpStairs;
}
double Pedestrian::GetSmoothFactorDownEscalators() const
{
    return _smoothFactorEscalatorDownStairs;
}

std::ostream& operator<<(std::ostream& out, const Pedestrian& pedestrian)
{
    // TODO(kkratz) Fix
    return out << "";
}
