// SPDX-License-Identifier: LGPL-3.0-or-later
//
// WarpDriver collision-avoidance model.
// Based on Wolinski, Lin, and Pettré (2016) — PhD thesis Chapter 4, Appendix B.
//
// Warp operators implemented (B.1–B.15):
//   W_ref  — local frame change (W_local: A's frame → B's frame)
//   W_th   — time horizon normalization (B.1–B.3)
//   W_tu   — time uncertainty with probability scaling (B.4–B.6)
//   W_r    — radius normalization via Minkowski sum (B.7–B.9)
//   W_v    — velocity shear (B.10–B.12)
//   W_vu   — anisotropic velocity uncertainty with probability scaling (B.13–B.15)
//
// Non-thesis safety mechanisms (practical additions):
//   - Short-range repulsion (3× combined radius) to prevent overlaps
//   - Boundary wall steering
//   - Stuck detection with lateral detour to break narrow-passage deadlocks
//   - Lateral perturbation for symmetry breaking
//
// TODO: W_ref currently uses simple W_local (straight-line frame change).
//   The thesis defines graph-based variants (Algorithm 3) that warp space
//   along navigable paths — W_el (environment layout), W_io (obstacle
//   interactions), W_ob (observed behaviors). These would enable anticipatory
//   avoidance around corners and bends. The routing infrastructure exists
//   (RoutingEngine::get_shortest_path provides the full waypoint path);
//   the path could serve as the graph for Algorithm 3's spatial projection.
//
#include "WarpDriverModel.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <variant>

// ============================================================================
// IntrinsicField
// ============================================================================

void WarpDriverModel::IntrinsicField::compute(double sigma)
{
    nx = static_cast<int>(std::round((x_max - x_min) / dx)) + 1;
    ny = static_cast<int>(std::round((y_max - y_min) / dy)) + 1;
    values.resize(static_cast<size_t>(nx * ny), 0.0);
    gradients.resize(static_cast<size_t>(nx * ny), Point{0.0, 0.0});

    const double sigma_squared = sigma * sigma;

    // Compute I(x,y) = (f * g)(x,y) where g = unit disk, f = Gaussian(sigma).
    // For each grid point, numerically integrate the convolution over the disk.
    const double integration_step = 0.05;
    const double integration_radius = 1.0; // unit disk support

    for(int ix = 0; ix < nx; ++ix) {
        for(int iy = 0; iy < ny; ++iy) {
            const double px = x_min + ix * dx;
            const double py = y_min + iy * dy;

            double val = 0.0;
            // Integrate f(px-u, py-v) * g(u,v) du dv over g's support (unit disk)
            for(double u = -integration_radius; u <= integration_radius; u += integration_step) {
                for(double v = -integration_radius; v <= integration_radius;
                    v += integration_step) {
                    if(u * u + v * v <= 1.0) {
                        double dx2 = px - u;
                        double dy2 = py - v;
                        val += std::exp(-(dx2 * dx2 + dy2 * dy2) / (2.0 * sigma_squared));
                    }
                }
            }
            val *= integration_step * integration_step;
            values[static_cast<size_t>(ix * ny + iy)] = val;
        }
    }

    // Normalize so peak ≈ 1
    const double max_val = *std::max_element(values.begin(), values.end());
    if(max_val > 0.0) {
        for(auto& v : values) {
            v /= max_val;
        }
    }

    // Compute gradients via central differences
    for(int ix = 0; ix < nx; ++ix) {
        for(int iy = 0; iy < ny; ++iy) {
            double d_idx = 0.0;
            double d_idy = 0.0;
            if(ix > 0 && ix < nx - 1) {
                d_idx = (values[static_cast<size_t>((ix + 1) * ny + iy)] -
                         values[static_cast<size_t>((ix - 1) * ny + iy)]) /
                        (2.0 * dx);
            }
            if(iy > 0 && iy < ny - 1) {
                d_idy = (values[static_cast<size_t>(ix * ny + (iy + 1))] -
                         values[static_cast<size_t>(ix * ny + (iy - 1))]) /
                        (2.0 * dy);
            }
            gradients[static_cast<size_t>(ix * ny + iy)] = Point{d_idx, d_idy};
        }
    }
}

std::pair<double, Point> WarpDriverModel::IntrinsicField::sample(double x, double y) const
{
    if(x < x_min || x > x_max || y < y_min || y > y_max) {
        return {0.0, Point{0.0, 0.0}};
    }

    const double fx = (x - x_min) / dx;
    const double fy = (y - y_min) / dy;
    const int ix = std::clamp(static_cast<int>(fx), 0, nx - 2);
    const int iy = std::clamp(static_cast<int>(fy), 0, ny - 2);
    const double sx = fx - ix;
    const double sy = fy - iy;

    const auto idx = [&](int i, int j) -> size_t { return static_cast<size_t>(i * ny + j); };

    // Bilinear interpolation
    const double v00 = values[idx(ix, iy)];
    const double v10 = values[idx(ix + 1, iy)];
    const double v01 = values[idx(ix, iy + 1)];
    const double v11 = values[idx(ix + 1, iy + 1)];
    const double val =
        v00 * (1 - sx) * (1 - sy) + v10 * sx * (1 - sy) + v01 * (1 - sx) * sy + v11 * sx * sy;

    const Point g00 = gradients[idx(ix, iy)];
    const Point g10 = gradients[idx(ix + 1, iy)];
    const Point g01 = gradients[idx(ix, iy + 1)];
    const Point g11 = gradients[idx(ix + 1, iy + 1)];
    const Point grad = g00 * ((1 - sx) * (1 - sy)) + g10 * (sx * (1 - sy)) + g01 * ((1 - sx) * sy) +
                       g11 * (sx * sy);

    return {val, grad};
}

// ============================================================================
// Warp Operators
// ============================================================================

namespace
{

using STP = WarpDriverModel::SpaceTimePoint;

// W_local: change of reference frame from agent a to agent b. Everything is expressed
// relative to a, so a sits at the origin and b at 'rel_pos_b'.
STP warp_local_forward(const STP& s, Point rel_pos_b, Point orient_a, Point orient_b)
{
    // Rotate from a's frame to the axis aligned frame around a
    const double cos_a = orient_a.x;
    const double sin_a = orient_a.y;
    const double wx = cos_a * s.x - sin_a * s.y;
    const double wy = sin_a * s.x + cos_a * s.y;

    // ... and on into b's frame
    const double dx = wx - rel_pos_b.x;
    const double dy = wy - rel_pos_b.y;
    const double cos_b = orient_b.x;
    const double sin_b = orient_b.y;
    return STP{cos_b * dx + sin_b * dy, -sin_b * dx + cos_b * dy, s.t};
}

// W_v: velocity shear. In b's frame, x' = x - speed_b * t
STP warp_velocity_forward(const STP& s, double speed_b)
{
    return STP{s.x - speed_b * s.t, s.y, s.t};
}

// W_r: radius scaling (B.7). W_r(s) = s ★ (1/α, 1/α, 1).
STP warp_radius_forward(const STP& s, double radius_b)
{
    const double inv_r = 1.0 / std::max(radius_b, 1e-6);
    return STP{s.x * inv_r, s.y * inv_r, s.t};
}

// W_ts: time uncertainty. Scale (x,y) by 1/(1 + lambda*t)
STP warp_time_uncertainty_forward(const STP& s, double lambda)
{
    const double scale = 1.0 / (1.0 + lambda * std::max(s.t, 0.0));
    return STP{s.x * scale, s.y * scale, s.t};
}

struct VelocityUncertaintyScale {
    double beta1;
    double beta2;
};

// B.13: β₁ = 1/(1 + α₁·v/v_pref), β₂ = 1 + α₂·v/v_pref.
// Since we use v0 for both current and preferred speed, v/v_pref = 1.
VelocityUncertaintyScale velocity_uncertainty_factors(double uncertainty_x, double uncertainty_y)
{
    return {1.0 / (1.0 + uncertainty_x), 1.0 + uncertainty_y};
}

// W_vu: velocity uncertainty (B.13). Anisotropic scaling:
// β₁ = 1/(1 + α₁) compresses x, β₂ = 1 + α₂ expands y.
STP warp_velocity_uncertainty_forward(const STP& s, double uncertainty_x, double uncertainty_y)
{
    const auto [beta1, beta2] = velocity_uncertainty_factors(uncertainty_x, uncertainty_y);
    return STP{s.x * beta1, s.y * beta2, s.t};
}

// Full composition: forward from a's frame to b's Intrinsic Field space
// Order: W_local -> W_v -> W_r -> W_ts -> W_vu
// Then check W_th (time validity)
struct WarpParams {
    Point rel_pos_b;
    Point orient_a;
    Point orient_b;
    double speed_b;
    double radius_b;
    double lambda;
    double velocity_uncertainty_x;
    double velocity_uncertainty_y;
    double time_horizon;
};

// Probability scaling (B.5 + B.14): product of inverse probability transforms.
// W_tu^{-1}(p) = p*beta^2, W_vu^{-1}(p) = p*beta1*beta2.
double probability_scale(const STP& s_original, const WarpParams& p)
{
    const double beta_tu = 1.0 / (1.0 + p.lambda * std::max(s_original.t, 0.0));
    const auto [beta1, beta2] =
        velocity_uncertainty_factors(p.velocity_uncertainty_x, p.velocity_uncertainty_y);
    return beta_tu * beta_tu * beta1 * beta2;
}

STP compose_forward(const STP& s, const WarpParams& p)
{
    auto s1 = warp_local_forward(s, p.rel_pos_b, p.orient_a, p.orient_b);
    auto s2 = warp_velocity_forward(s1, p.speed_b);
    auto s3 = warp_radius_forward(s2, p.radius_b);
    auto s4 = warp_time_uncertainty_forward(s3, p.lambda);
    auto s5 =
        warp_velocity_uncertainty_forward(s4, p.velocity_uncertainty_x, p.velocity_uncertainty_y);
    // Normalize time: map [0, time_horizon] -> [0, 1]
    s5.t = (p.time_horizon > 0.0) ? s5.t / p.time_horizon : 0.0;
    return s5;
}

// Gradient transform: takes 2D gradient from IntrinsicField, returns 3D space-time gradient
// in a's frame. Applies inverse Jacobians in reverse order.
STP compose_gradient_inverse(const Point& grad_i, const STP& s_original, const WarpParams& p)
{
    // Start with 3-component gradient in Intrinsic Field space: (grad_i.x, grad_i.y, 0)
    // since dI/dt = 0
    double gx = grad_i.x;
    double gy = grad_i.y;
    double gt = 0.0;

    // Time normalization inverse Jacobian: dt_original = dt_normalized * time_horizon
    // So dI/dt_original = dI/dt_normalized / time_horizon
    // But dI/dt = 0, so gt stays 0 at this point. However, the spatial components
    // pick up time contributions from the velocity shear.

    // W_vu^-1: anisotropic scaling (B.15). Inverse scales gradient by the
    // forward factors (beta1, beta2) since J_vu = diag(beta1, beta2, 1).
    {
        const auto [beta1, beta2] =
            velocity_uncertainty_factors(p.velocity_uncertainty_x, p.velocity_uncertainty_y);
        gx *= beta1;
        gy *= beta2;
    }

    // W_tu^-1 (B.6): spatial gradient scaled by beta, temporal gets cross-terms.
    // Coordinates at W_tu input = after W_ref -> W_v -> W_r on s_original.
    // TODO(perf): compose_forward already computes this intermediate; cache and
    // reuse instead of recomputing the three warps here. ~15% per-sample saving.
    {
        const double t = s_original.t;
        const double beta = 1.0 / (1.0 + p.lambda * std::max(t, 0.0));
        auto s_at_tu = warp_local_forward(s_original, p.rel_pos_b, p.orient_a, p.orient_b);
        s_at_tu = warp_velocity_forward(s_at_tu, p.speed_b);
        s_at_tu = warp_radius_forward(s_at_tu, p.radius_b);
        const double gamma1 = -p.lambda * beta * beta * s_at_tu.x;
        const double gamma2 = -p.lambda * beta * beta * s_at_tu.y;
        const double gx_old = gx;
        const double gy_old = gy;
        gx *= beta;
        gy *= beta;
        gt = gamma1 * gx_old + gamma2 * gy_old + gt;
    }

    // W_r^-1: identity (B.9).

    // W_v^-1 (B.12): g + (0, 0, -v·g.x).
    {
        gt -= p.speed_b * gx;
    }

    // W_local inverse Jacobian: rotate from b's frame back to a's frame
    {
        const double cos_a = p.orient_a.x;
        const double sin_a = p.orient_a.y;
        const double cos_b = p.orient_b.x;
        const double sin_b = p.orient_b.y;

        // Combined rotation: b's frame -> world -> a's frame
        // R_a^T * R_b applied to gradient
        const double cos_ab = cos_a * cos_b + sin_a * sin_b;
        const double sin_ab = sin_a * cos_b - cos_a * sin_b;
        const double gx_new = cos_ab * gx + sin_ab * gy;
        const double gy_new = -sin_ab * gx + cos_ab * gy;
        gx = gx_new;
        gy = gy_new;
    }

    return STP{gx, gy, gt};
}

} // anonymous namespace

// ============================================================================
// WarpDriverModel
// ============================================================================

WarpDriverModel::WarpDriverModel(
    double sigma,
    double time_horizon,
    double step_size,
    double time_uncertainty,
    double velocity_uncertainty_x,
    double velocity_uncertainty_y,
    int num_samples,
    uint64_t rng_seed)
    : _time_horizon(time_horizon)
    , _step_size(step_size)
    , _time_uncertainty(time_uncertainty)
    , _velocity_uncertainty_x(velocity_uncertainty_x)
    , _velocity_uncertainty_y(velocity_uncertainty_y)
    , _num_samples(num_samples)
    // Neighborhood cutoff: maximum distance at which a neighbor can still
    // collide with us within time_horizon. Two agents closing head-on cover
    // 2 * v_max * time_horizon, plus their combined radii, plus a small margin.
    // v_max and r_max are hardcoded pedestrian defaults.
    , _cut_off_radius(2.0 * 1.5 * time_horizon + 2.0 * 0.3 + 0.5)
    , _rng(rng_seed)
{
    if(sigma <= 0.0) {
        throw SimulationError("WarpDriverModel: sigma must be > 0, got {}", sigma);
    }
    _intrinsic_field.compute(sigma);
}

OperationalModelType WarpDriverModel::type() const
{
    return OperationalModelType::WarpDriver;
}

void WarpDriverModel::check_model_constraint(const GenericAgent& agent, const AgentView& view) const
{
    const auto* data = std::get_if<State>(&agent.state);
    if(!data) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} does not have WarpDriverModel data",
            agent.id);
    }
    if(data->radius <= 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid radius {}",
            agent.id,
            data->radius);
    }
    if(data->v0 < 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid v0 {}", agent.id, data->v0);
    }
    if(this->_time_horizon <= 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid timeHorizon {}, must be > 0",
            agent.id,
            this->_time_horizon);
    }
    if(this->_step_size <= 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid stepSize {}, must be > 0",
            agent.id,
            this->_step_size);
    }
    if(this->_num_samples < 1) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid numSamples {}, must be >= 1",
            agent.id,
            this->_num_samples);
    }
    if(this->_time_uncertainty < 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid timeUncertainty {}, must be "
            ">= 0",
            agent.id,
            this->_time_uncertainty);
    }
    if(this->_velocity_uncertainty_x < 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid velocityUncertaintyX {}, must "
            "be >= 0",
            agent.id,
            this->_velocity_uncertainty_x);
    }
    if(this->_velocity_uncertainty_y < 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid velocityUncertaintyY {}, must "
            "be >= 0",
            agent.id,
            this->_velocity_uncertainty_y);
    }

    const auto neighbors = view.other_agents_in_range(2.0);
    for(const auto& neighbor : neighbors) {
        const auto distance = neighbor.relative_position.norm();

        if(data->radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent at {} too close to agent at {}: distance {}, "
                "radius {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.relative_position,
                distance,
                data->radius);
        }
    }
    const auto max_radius = data->radius / 2;
    if(!view.walls_in_range(max_radius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent at {} too close to geometry boundaries, distance < "
            "{}/2",
            agent.location.xy(),
            data->radius);
    }
}

Point WarpDriverModel::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& agent_data = std::get<State>(current);
    auto& next_data = std::get<State>(next);
    const double speed = agent_data.v0;

    // State orientation (unit vector). If zero, default to +x.
    Point orient = agent_data.orientation;
    if(orient.norm() < 1e-9) {
        orient = Point{1.0, 0.0};
    } else {
        orient = orient.normalized();
    }

    // Direction towards destination
    Point desired_dir = step.route_orientation();
    if(desired_dir == Point{}) {
        // The old update carried default-initialized stuck/detour state here,
        // so applying it reset that state; replicate that reset.
        // [RL, FIXME] This is expected to be dead code and should be removed in a subsequent
        //             cleanup.
        next_data.orientation = orient;
        next_data.stuck_time = 0.0;
        next_data.displacement_x = 0.0;
        next_data.displacement_y = 0.0;
        next_data.detour_time = 0.0;
        next_data.detour_side = 1;
        return Point{0.0, 0.0};
    }

    // Use desired direction as agent's effective orientation for the frame
    Point effective_orient = desired_dir;

    // === Step 1: Projected trajectory in agent-centric space ===
    // r(t) = (speed * t, 0, t) for t in [0, time_horizon]
    const double dt_sample = this->_time_horizon / std::max(this->_num_samples - 1, 1);

    // === Step 2: Perceive - build collision probability field ===
    const auto neighbors = step.other_agents_in_range(
        _cut_off_radius, [&step](const NeighborView& n) { return step.no_geometry_between(n); });

    // Short-range repulsion: not part of the original Wolinski et al. (2016)
    // model, which is purely anticipatory. Added as a practical safety net
    // because the collision probability field alone cannot guarantee separation
    // when agents are already close (dense crowds, late reactions).
    // Similar to the pushout mechanisms in CFS and AVM.
    Point repulsion{0.0, 0.0};
    for(const auto& neighbor : neighbors) {
        const auto* nb_data = std::get_if<State>(neighbor.state);
        if(!nb_data) {
            continue;
        }
        Point diff = neighbor.relative_position * -1.0;
        const double dist = diff.norm();
        const double combined_radius = agent_data.radius + nb_data->radius;
        if(dist < combined_radius * 3.0 && dist > 1e-6) {
            const double overlap = combined_radius * 3.0 - dist;
            repulsion = repulsion + diff.normalized() * (speed * overlap / dist);
        } else if(dist <= 1e-6) {
            repulsion = repulsion + Point{-desired_dir.y, desired_dir.x} * speed;
        }
    }

    // Random perturbation: small lateral offset on trajectory samples to break
    // symmetry in perfectly aligned head-on encounters where the gradient field
    // cancels by symmetry, producing no lateral avoidance.
    std::uniform_real_distribution<double> perturb_dist(-0.05, 0.05);

    // Storage for per-sample combined probability and gradient
    struct Sample {
        double t;
        STP r; // trajectory point in agent-centric space-time
        double p_total;
        STP grad_total;
    };
    std::vector<Sample> samples(static_cast<size_t>(this->_num_samples));

    for(int i = 0; i < this->_num_samples; ++i) {
        const double t = i * dt_sample;
        const double lateral_perturbation = perturb_dist(_rng);
        samples[static_cast<size_t>(i)] =
            Sample{t, STP{speed * t, lateral_perturbation, t}, 0.0, STP{0, 0, 0}};
    }

    for(const auto& neighbor : neighbors) {
        const auto* nb_data = std::get_if<State>(neighbor.state);
        if(!nb_data) {
            continue;
        }

        // Neighbor orientation
        Point nb_orient = nb_data->orientation;
        if(nb_orient.norm() < 1e-9) {
            nb_orient = Point{1.0, 0.0};
        } else {
            nb_orient = nb_orient.normalized();
        }

        // Neighbor speed (from v0)
        const double nb_speed = nb_data->v0;

        // TODO(perf): WarpParams and all neighbor-derived constants (orientation,
        // speed, Minkowski radius, rotation matrix cos_ab/sin_ab used in the
        // gradient inverse) are loop-invariant w.r.t. the sample index. Hoist
        // them out of the sample loop and precompute once per (ped, neighbor).
        WarpParams wp{};
        wp.rel_pos_b = neighbor.relative_position;
        wp.orient_a = effective_orient;
        wp.orient_b = nb_orient;
        wp.speed_b = nb_speed;
        wp.radius_b = agent_data.radius + nb_data->radius; // Minkowski sum
        wp.lambda = this->_time_uncertainty;
        wp.velocity_uncertainty_x = this->_velocity_uncertainty_x;
        wp.velocity_uncertainty_y = this->_velocity_uncertainty_y;
        wp.time_horizon = this->_time_horizon;

        for(auto& s : samples) {
            // Forward warp sample point to neighbor's Intrinsic Field space
            STP warped = compose_forward(s.r, wp);

            // Time validity check: must be in [0, 1] (normalized)
            if(warped.t < 0.0 || warped.t > 1.0) {
                continue;
            }

            // Lookup Intrinsic Field (2D) and apply probability scaling (B.5, B.14)
            auto [intrinsic_p, grad_i] = _intrinsic_field.sample(warped.x, warped.y);
            const double p_b = intrinsic_p * probability_scale(s.r, wp);

            if(p_b < 1e-12) {
                continue;
            }

            // Transform gradient back to agent's frame
            STP grad_b = compose_gradient_inverse(grad_i, s.r, wp);

            // Union formula: p_new = p + p_b - p * p_b
            double p_old = s.p_total;
            s.p_total = p_old + p_b - p_old * p_b;
            s.grad_total.x = s.grad_total.x + grad_b.x - p_old * grad_b.x - p_b * s.grad_total.x;
            s.grad_total.y = s.grad_total.y + grad_b.y - p_old * grad_b.y - p_b * s.grad_total.y;
            s.grad_total.t = s.grad_total.t + grad_b.t - p_old * grad_b.t - p_b * s.grad_total.t;
        }
    }

    // === Step 3: Solve - gradient descent on trajectory ===
    // Integrate N, P, G, S per Eq. 4-7
    double n = 0.0;
    double p = 0.0;
    STP g{0, 0, 0};
    STP s{0, 0, 0};

    for(const auto& sample : samples) {
        n += sample.p_total * dt_sample;
        p += sample.p_total * sample.p_total * dt_sample;
        g.x += sample.p_total * sample.grad_total.x * dt_sample;
        g.y += sample.p_total * sample.grad_total.y * dt_sample;
        g.t += sample.p_total * sample.grad_total.t * dt_sample;
        s.x += sample.p_total * sample.r.x * dt_sample;
        s.y += sample.p_total * sample.r.y * dt_sample;
        s.t += sample.p_total * sample.r.t * dt_sample;
    }

    Point new_vel_local;

    if(n < 1e-9) {
        // No collision risk — follow projected trajectory
        new_vel_local = Point{speed, 0.0};
    } else {
        p /= n;
        g.x /= n;
        g.y /= n;
        g.t /= n;
        s.x /= n;
        s.y /= n;
        s.t /= n;

        // q = S - alpha * P * G  (Eq. 8)
        STP q{};
        q.x = s.x - this->_step_size * p * g.x;
        q.y = s.y - this->_step_size * p * g.y;
        q.t = s.t - this->_step_size * p * g.t;

        if(q.t > 1e-9) {
            new_vel_local = Point{q.x / q.t, q.y / q.t};
        } else {
            new_vel_local = Point{speed, 0.0};
        }
    }

    // Clamp speed to [0, v0]
    const double new_speed = std::min(new_vel_local.norm(), agent_data.v0);

    // Convert to world coordinates: rotate by effective_orient
    Point new_vel_world;
    if(new_speed > 1e-9) {
        Point new_dir_local = new_vel_local.normalized();
        // Rotate from agent-centric to world
        new_vel_world =
            Point{
                effective_orient.x * new_dir_local.x - effective_orient.y * new_dir_local.y,
                effective_orient.y * new_dir_local.x + effective_orient.x * new_dir_local.y} *
            new_speed;
    } else {
        new_vel_world = desired_dir * agent_data.v0 * 0.01; // tiny push towards goal
    }

    // State repulsion
    new_vel_world = new_vel_world + repulsion;

    // Boundary avoidance: steer agents away from walls
    const double reach = agent_data.radius * 3.0;
    for(const auto& wall : step.walls_in_range(reach)) {
        if(wall.segment.length_square() < 1e-12) {
            continue; // degenerate wall segment
        }
        if(wall.distance > 1e-6) {
            const double steering = agent_data.v0 * (reach - wall.distance) / wall.distance;
            new_vel_world = new_vel_world + wall.normal * steering;
        }
    }

    // Re-clamp speed to v0 after wall steering
    double final_speed = new_vel_world.norm();
    if(final_speed > agent_data.v0 && final_speed > 1e-9) {
        new_vel_world = new_vel_world * (agent_data.v0 / final_speed);
        final_speed = agent_data.v0;
    }

    // Stuck detection: accumulate the movement since the last reset over a time window.
    // Catches oscillating agents that periodically spike above the speed threshold but make
    // no real progress.
    double stuck_time = agent_data.stuck_time;
    Point displacement{agent_data.displacement_x, agent_data.displacement_y};
    double detour_time = agent_data.detour_time;
    int detour_side = agent_data.detour_side;

    // Detour mode: agent is currently on a lateral detour to break a deadlock
    if(detour_time > 0.0) {
        detour_time -= step.dt();
        Point lateral{-desired_dir.y * detour_side, desired_dir.x * detour_side};
        Point detour_dir = (lateral * 0.8 + desired_dir * 0.2).normalized();
        Point detour_vel = detour_dir * agent_data.v0 * 0.5;
        Point movement = detour_vel * step.dt();
        // If detour would leave the walkable area, try the other side
        if(!step.no_geometry_between(movement)) {
            detour_side = -detour_side;
            lateral = Point{-desired_dir.y * detour_side, desired_dir.x * detour_side};
            detour_dir = (lateral * 0.8 + desired_dir * 0.2).normalized();
            detour_vel = detour_dir * agent_data.v0 * 0.5;
            movement = detour_vel * step.dt();
            // If both sides fail, just creep toward goal
            if(!step.no_geometry_between(movement)) {
                movement = desired_dir * agent_data.v0 * 0.1 * step.dt();
                detour_dir = desired_dir;
            }
        }
        displacement += movement;
        if(detour_time <= 0.0) {
            detour_time = 0.0;
            stuck_time = 0.0;
            // FIXME: this drops the displacement *including* this step's movement, while the
            // progress reset below drops it *excluding* it. The two resets therefore disagree
            // by one step, so the progress window starts later after a detour than after
            // normal progress. Kept as it was to keep the behaviour unchanged.
            displacement = Point{};
        }
        next_data.orientation = detour_dir;
        next_data.stuck_time = stuck_time;
        next_data.displacement_x = displacement.x;
        next_data.displacement_y = displacement.y;
        next_data.detour_time = detour_time;
        next_data.detour_side = detour_side;
        return movement;
    }

    // Measure the accumulated displacement over the stuck window
    constexpr double stuck_threshold = 5.0; // seconds before triggering detour
    constexpr double detour_duration = 1.0; // seconds of lateral movement
    constexpr double progress_radius = 0.3; // must move this far to count as progress

    stuck_time += step.dt();
    const double net_displacement = displacement.norm();

    if(net_displacement > progress_radius) {
        // Real progress — start measuring again from where the agent stands now
        stuck_time = 0.0;
        displacement = Point{};
    } else if(stuck_time >= stuck_threshold) {
        // Stuck: no net progress for stuck_threshold seconds — enter detour
        std::uniform_int_distribution<int> side_dist(0, 1);
        detour_side = side_dist(_rng) * 2 - 1; // -1 or +1
        detour_time = detour_duration;
        stuck_time = 0.0;
    }

    // Velocity smoothing: blend new velocity with previous orientation to damp
    // oscillations in dense clusters where agents flip direction every frame.
    const double smoothing = 0.5; // weight of new velocity (1.0 = no smoothing)
    Point smoothed_vel =
        new_vel_world * smoothing + orient * (new_vel_world.norm() * (1.0 - smoothing));
    double smoothed_speed = smoothed_vel.norm();
    if(smoothed_speed > agent_data.v0 && smoothed_speed > 1e-9) {
        smoothed_vel = smoothed_vel * (agent_data.v0 / smoothed_speed);
    }

    Point new_orient = (smoothed_vel.norm() > 1e-9) ? smoothed_vel.normalized() : orient;

    const Point movement = smoothed_vel * step.dt();

    next_data.orientation = new_orient;
    next_data.stuck_time = stuck_time;
    next_data.displacement_x = displacement.x + movement.x;
    next_data.displacement_y = displacement.y + movement.y;
    next_data.detour_time = detour_time;
    next_data.detour_side = detour_side;
    return movement;
}
