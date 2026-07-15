// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "TacticalModelState.hpp"
#include "WarpDriverModelState.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <random>
#include <utility>
#include <vector>

class WarpDriverModel : public OperationalModel
{
public:
    using State = WarpDriverModelState;

    /// 3-component space-time point/vector used internally
    struct SpaceTimePoint {
        double x{};
        double y{};
        double t{};
    };

private:
    /// Precomputed 2D collision probability field I(x,y) and its gradient.
    /// Constant along time axis; time is a validity window [0,1] normalized.
    struct IntrinsicField {
        std::vector<double> values;
        std::vector<Point> gradients; // (dI/dx, dI/dy)
        double xMin{-3.0};
        double xMax{3.0};
        double yMin{-3.0};
        double yMax{3.0};
        double dx{0.1};
        double dy{0.1};
        int nx{61};
        int ny{61};

        void Compute(double sigma);
        /// Bilinear interpolation. Returns (0, {0,0}) for out-of-bounds.
        std::pair<double, Point> Sample(double x, double y) const;
    };

    // Model-level parameters
    double _timeHorizon;
    double _stepSize;
    double _timeUncertainty;
    double _velocityUncertaintyX;
    double _velocityUncertaintyY;
    int _numSamples;

    IntrinsicField _intrinsicField;
    mutable std::mt19937 _rng;

public:
    using OperationalModel::GenericState;
    using OperationalModel::StateContainer;

    WarpDriverModel(
        double sigma,
        double timeHorizon = 2.0,
        double stepSize = 0.5,
        double timeUncertainty = 0.5,
        double velocityUncertaintyX = 0.2,
        double velocityUncertaintyY = 0.2,
        int numSamples = 20,
        uint64_t rngSeed = 42);

    ~WarpDriverModel() override = default;

    OperationalModelType Type() const override;

    void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const TacticalModelState& tactical,
        const CollisionGeometry& geometry,
        const StateContainer& neighborStates) const override;

    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override;
};
