// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"

class EnvironmentQuery;
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "WarpDriverModelState.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <random>
#include <utility>
#include <vector>

struct NeighborView;

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
        double x_min{-3.0};
        double x_max{3.0};
        double y_min{-3.0};
        double y_max{3.0};
        double dx{0.1};
        double dy{0.1};
        int nx{61};
        int ny{61};

        void compute(double sigma);
        /// Bilinear interpolation. Returns (0, {0,0}) for out-of-bounds.
        std::pair<double, Point> sample(double x, double y) const;
    };

    // Model-level parameters
    double _time_horizon;
    double _step_size;
    double _time_uncertainty;
    double _velocity_uncertainty_x;
    double _velocity_uncertainty_y;
    int _num_samples;

    // Genuinely simulation-global state
    double _cut_off_radius;

    IntrinsicField _intrinsic_field;
    mutable std::mt19937 _rng;

public:
    WarpDriverModel(
        double sigma,
        double time_horizon = 2.0,
        double step_size = 0.5,
        double time_uncertainty = 0.5,
        double velocity_uncertainty_x = 0.2,
        double velocity_uncertainty_y = 0.2,
        int num_samples = 20,
        uint64_t rng_seed = 42);

    ~WarpDriverModel() override = default;

    OperationalModelType type() const override;

    Point compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;

    void check_model_constraint(const GenericAgent& agent, const AgentView& view) const override;
};
