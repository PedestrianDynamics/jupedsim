// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <random>
#include <utility>
#include <vector>

class WarpDriverModel : public OperationalModel
{
public:
    /// Per-agent state of the warp driver model.
    struct State {
        Point position{};
        Point orientation{0.0, 0.0};
        double radius{0.15};
        double v0{1.2};
        double stuckTime{0.0}; // elapsed time since anchor was set
        double anchorX{0.0}; // position when stuck tracking began
        double anchorY{0.0};
        double detourTime{0.0}; // remaining time in detour mode
        int detourSide{1}; // +1 = left, -1 = right of desired direction
    };

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

    // Builder-configured, simulation-wide values used by ComputeNextState
    double _timeHorizon;
    double _stepSize;
    double _timeUncertainty;
    double _velocityUncertaintyX;
    double _velocityUncertaintyY;
    int _numSamples;
    // Genuinely simulation-global state
    double _cutOffRadius;

    IntrinsicField _intrinsicField;
    mutable std::mt19937 _rng;

public:
    WarpDriverModel(
        double timeHorizon,
        double stepSize,
        double sigma,
        double timeUncertainty,
        double velocityUncertaintyX,
        double velocityUncertaintyY,
        int numSamples,
        uint64_t rngSeed = 42);

    ~WarpDriverModel() override = default;

    OperationalModelType Type() const override;

    void ComputeNextState(
        double dT,
        const GenericAgent& current,
        GenericAgent& next,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const override;

    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override;
};

template <>
struct fmt::formatter<WarpDriverModel::State> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const WarpDriverModel::State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "WarpDriver[orientation={}, radius={}, v0={}]",
            m.orientation,
            m.radius,
            m.v0);
    }
};
