/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Ellipse.hpp"
#include "Journey.hpp"
#include "Line.hpp"
#include "Macros.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

#include <memory>

class Agent
{
public:
    using ID = jps::UniqueID<Agent>;
    ID id{};

    // This is evaluated by the "strategic level"
    std::unique_ptr<Behaviour> behaviour{};

    // This is evaluated by the "operational level"
    Point destination{};
    Point waypoint{};

    OperationalModel::ParametersID parameterProfileId;

private:
    double _deltaT = 0.01; // step size
    JEllipse _ellipse{}; // the shape of this pedestrian
    Point _e0 = Point(0, 0); // desired direction
    int _newOrientationDelay = 0;

public:
    Agent() = default;
    ~Agent() = default;

    void SetEllipse(const JEllipse& e);

    void SetDeltaT(double dt);
    void SetPos(const Point& pos);
    void SetV(const Point& v);
    void SetV0(double v0);
    void SetE0(const Point& p) { _e0 = p; }
    void SetSmoothTurning();
    void IncrementOrientationDelay();
    void SetPhiPed();

    const JEllipse& GetEllipse() const;
    const Point& GetPos() const;
    const Point& GetV() const;
    const Point& GetE0() const;
    Point GetE0(const Point& target, double tau) const;
    void InitE0(const Point& target);

    /**
     * the desired speed is the projection of the speed on the horizontal plane.
     * @return the norm of the desired speed.
     */
    double GetV0() const;
};

std::ostream& operator<<(std::ostream& out, const Agent& pedestrian);
