/**
 * \file        Pedestrian.h
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
#pragma once

#include "Ellipse.hpp"
#include "Journey.hpp"
#include "Line.hpp"
#include "Macros.hpp"
#include "UniqueID.hpp"

#include <map>
#include <optional>
#include <vector>

class Pedestrian
{
public:
    using UID = jps::UniqueID<Pedestrian>;

    // This is evaluated by the "strategic level"
    std::unique_ptr<Behaviour> behaviour{};

    // This is evaluated by the "tactical level"
    // TODO(kkratz): this is the new yet unused waypoint list
    std::vector<Point> waypoints{};

    // This is evaluated by the "operational level"
    Point destination{};
    Point waypoint{};

    /// Point in time after this agent gets active.
    /// TODO(kkratz): Document premovement concept
    double premovementTime = 0;

private:
    const UID _uid{};

    int _group = -1;

    // gcfm specific parameters
    double _mass = 1; // Mass: 1
    double _tau = 0.5; // Reaction time: 0.5
    double _t = 1.0; // OV function

    double _deltaT = 0.01; // step size
    JEllipse _ellipse{}; // the shape of this pedestrian
    Point _v0 = Point(0, 0); // vector V0

    double _v0UpStairs = 0.6;
    double _v0DownStairs = 0.6;
    double _v0EscalatorUpStairs = 0.8;
    double _v0EscalatorDownStairs = 0.8;
    /// c in f() and g() for v0 transition on stairs up
    double _smoothFactorUpStairs = 15;
    /// c in f() and g() for v0 transition on stairs down
    double _smoothFactorDownStairs = 15;
    /// c in f() and g() for v0 transition on escalators up
    double _smoothFactorEscalatorUpStairs = 15;
    /// c in f() and g() for v0 transition on escalators down
    double _smoothFactorEscalatorDownStairs = 15;

    int _newOrientationDelay = 0;

public:
    Pedestrian() = default;
    ~Pedestrian() = default;

    void SetSmoothFactorUpStairs(double c);
    void SetSmoothFactorDownStairs(double c);
    void SetSmoothFactorEscalatorUpStairs(double c);
    void SetSmoothFactorEscalatorDownStairs(double c);
    void SetTau(double tau);
    void SetEllipse(const JEllipse& e);

    double GetT() const;
    void SetT(double T);
    void SetDeltaT(double dt);
    // Eigenschaften der Ellipse
    void SetPos(const Point& pos); // setzt x und y-Koordinaten
    void SetV(const Point& v); // setzt x und y-Koordinaten der Geschwindigkeit
    void SetV0Norm(
        double v0,
        double v0UpStairs,
        double v0DownStairs,
        double escalatorUp,
        double escalatorDown);
    void SetV0(const Point& p) { _v0 = p; }
    void SetSmoothTurning();
    void IncrementOrientationDelay();
    void SetPhiPed();

    double GetV0UpStairsNorm() const;
    double GetV0DownStairsNorm() const;
    double GetV0EscalatorUpNorm() const;
    double GetV0EscalatorDownNorm() const;

    double GetSmoothFactorUpStairs() const;
    double GetSmoothFactorDownStairs() const;
    double GetSmoothFactorUpEscalators() const;
    double GetSmoothFactorDownEscalators() const;

    UID GetUID() const;
    double GetMass() const;
    double GetTau() const;
    const JEllipse& GetEllipse() const;
    // Eigenschaften der Ellipse
    const Point& GetPos() const;
    const Point& GetV() const;
    const Point& GetV0() const;
    Point GetV0(const Point& target) const;
    void InitV0(const Point& target);

    /**
     * the desired speed is the projection of the speed on the horizontal plane.
     * @return the norm of the desired speed.
     */
    double GetV0Norm() const;

    /// get axis in the walking direction
    double GetLargerAxis() const;
    /// get axis in the shoulder direction = orthogonal to the walking direction
    double GetSmallerAxis() const;

    int GetGroup() const;
    void SetGroup(int group);
};

std::ostream& operator<<(std::ostream& out, const Pedestrian& pedestrian);
