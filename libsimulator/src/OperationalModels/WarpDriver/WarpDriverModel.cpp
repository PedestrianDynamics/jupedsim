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
//   (RoutingEngine::ComputeAllWaypoints provides the full waypoint path);
//   the path could serve as the graph for Algorithm 3's spatial projection.
//
#include "WarpDriverModel.hpp"

#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <algorithm>
#include <cmath>
#include <variant>

// ============================================================================
// IntrinsicField
// ============================================================================

void WarpDriverModel::IntrinsicField::Compute(double sigma)
{
    nx = static_cast<int>(std::round((xMax - xMin) / dx)) + 1;
    ny = static_cast<int>(std::round((yMax - yMin) / dy)) + 1;
    values.resize(static_cast<size_t>(nx * ny), 0.0);
    gradients.resize(static_cast<size_t>(nx * ny), Point{0.0, 0.0});

    const double sigma_squared = sigma * sigma;

    // Compute I(x,y) = (f * g)(x,y) where g = unit disk, f = Gaussian(sigma).
    // For each grid point, numerically integrate the convolution over the disk.
    const double integrationStep = 0.05;
    const double integrationRadius = 1.0; // unit disk support

    for(int ix = 0; ix < nx; ++ix) {
        for(int iy = 0; iy < ny; ++iy) {
            const double px = xMin + ix * dx;
            const double py = yMin + iy * dy;

            double val = 0.0;
            // Integrate f(px-u, py-v) * g(u,v) du dv over g's support (unit disk)
            for(double u = -integrationRadius; u <= integrationRadius; u += integrationStep) {
                for(double v = -integrationRadius; v <= integrationRadius; v += integrationStep) {
                    if(u * u + v * v <= 1.0) {
                        double dx2 = px - u;
                        double dy2 = py - v;
                        val += std::exp(-(dx2 * dx2 + dy2 * dy2) / (2.0 * sigma_squared));
                    }
                }
            }
            val *= integrationStep * integrationStep;
            values[static_cast<size_t>(ix * ny + iy)] = val;
        }
    }

    // Normalize so peak ≈ 1
    const double maxVal = *std::max_element(values.begin(), values.end());
    if(maxVal > 0.0) {
        for(auto& v : values) {
            v /= maxVal;
        }
    }

    // Compute gradients via central differences
    for(int ix = 0; ix < nx; ++ix) {
        for(int iy = 0; iy < ny; ++iy) {
            double dIdx = 0.0;
            double dIdy = 0.0;
            if(ix > 0 && ix < nx - 1) {
                dIdx = (values[static_cast<size_t>((ix + 1) * ny + iy)] -
                        values[static_cast<size_t>((ix - 1) * ny + iy)]) /
                       (2.0 * dx);
            }
            if(iy > 0 && iy < ny - 1) {
                dIdy = (values[static_cast<size_t>(ix * ny + (iy + 1))] -
                        values[static_cast<size_t>(ix * ny + (iy - 1))]) /
                       (2.0 * dy);
            }
            gradients[static_cast<size_t>(ix * ny + iy)] = Point{dIdx, dIdy};
        }
    }
}

std::pair<double, Point> WarpDriverModel::IntrinsicField::Sample(double x, double y) const
{
    if(x < xMin || x > xMax || y < yMin || y > yMax) {
        return {0.0, Point{0.0, 0.0}};
    }

    const double fx = (x - xMin) / dx;
    const double fy = (y - yMin) / dy;
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

// W_local: change of reference frame from agent a to agent b
STP WarpLocalForward(const STP& s, Point posA, Point orientA, Point posB, Point orientB)
{
    // Rotate from a's frame to world
    const double cosA = orientA.x;
    const double sinA = orientA.y;
    const double wx = cosA * s.x - sinA * s.y + posA.x;
    const double wy = sinA * s.x + cosA * s.y + posA.y;

    // World to b's frame
    const double dx = wx - posB.x;
    const double dy = wy - posB.y;
    const double cosB = orientB.x;
    const double sinB = orientB.y;
    return STP{cosB * dx + sinB * dy, -sinB * dx + cosB * dy, s.t};
}

// W_v: velocity shear. In b's frame, x' = x - speed_b * t
STP WarpVelocityForward(const STP& s, double speedB)
{
    return STP{s.x - speedB * s.t, s.y, s.t};
}

// W_r: radius scaling (B.7). W_r(s) = s ★ (1/α, 1/α, 1).
STP WarpRadiusForward(const STP& s, double radiusB)
{
    const double invR = 1.0 / std::max(radiusB, 1e-6);
    return STP{s.x * invR, s.y * invR, s.t};
}

// W_ts: time uncertainty. Scale (x,y) by 1/(1 + lambda*t)
STP WarpTimeUncertaintyForward(const STP& s, double lambda)
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
VelocityUncertaintyScale VelocityUncertaintyFactors(double uncertaintyX, double uncertaintyY)
{
    return {1.0 / (1.0 + uncertaintyX), 1.0 + uncertaintyY};
}

// W_vu: velocity uncertainty (B.13). Anisotropic scaling:
// β₁ = 1/(1 + α₁) compresses x, β₂ = 1 + α₂ expands y.
STP WarpVelocityUncertaintyForward(const STP& s, double uncertaintyX, double uncertaintyY)
{
    const auto [beta1, beta2] = VelocityUncertaintyFactors(uncertaintyX, uncertaintyY);
    return STP{s.x * beta1, s.y * beta2, s.t};
}

// Full composition: forward from a's frame to b's Intrinsic Field space
// Order: W_local -> W_v -> W_r -> W_ts -> W_vu
// Then check W_th (time validity)
struct WarpParams {
    Point posA;
    Point orientA;
    Point posB;
    Point orientB;
    double speedB;
    double radiusB;
    double lambda;
    double velocityUncertaintyX;
    double velocityUncertaintyY;
    double timeHorizon;
};

// Probability scaling (B.5 + B.14): product of inverse probability transforms.
// W_tu^{-1}(p) = p*beta^2, W_vu^{-1}(p) = p*beta1*beta2.
double ProbabilityScale(const STP& sOriginal, const WarpParams& p)
{
    const double beta_tu = 1.0 / (1.0 + p.lambda * std::max(sOriginal.t, 0.0));
    const auto [beta1, beta2] =
        VelocityUncertaintyFactors(p.velocityUncertaintyX, p.velocityUncertaintyY);
    return beta_tu * beta_tu * beta1 * beta2;
}

STP ComposeForward(const STP& s, const WarpParams& p)
{
    auto s1 = WarpLocalForward(s, p.posA, p.orientA, p.posB, p.orientB);
    auto s2 = WarpVelocityForward(s1, p.speedB);
    auto s3 = WarpRadiusForward(s2, p.radiusB);
    auto s4 = WarpTimeUncertaintyForward(s3, p.lambda);
    auto s5 = WarpVelocityUncertaintyForward(s4, p.velocityUncertaintyX, p.velocityUncertaintyY);
    // Normalize time: map [0, timeHorizon] -> [0, 1]
    s5.t = (p.timeHorizon > 0.0) ? s5.t / p.timeHorizon : 0.0;
    return s5;
}

// Gradient transform: takes 2D gradient from IntrinsicField, returns 3D space-time gradient
// in a's frame. Applies inverse Jacobians in reverse order.
STP ComposeGradientInverse(const Point& gradI, const STP& sOriginal, const WarpParams& p)
{
    // Start with 3-component gradient in Intrinsic Field space: (gradI.x, gradI.y, 0)
    // since dI/dt = 0
    double gx = gradI.x;
    double gy = gradI.y;
    double gt = 0.0;

    // Time normalization inverse Jacobian: dt_original = dt_normalized * timeHorizon
    // So dI/dt_original = dI/dt_normalized / timeHorizon
    // But dI/dt = 0, so gt stays 0 at this point. However, the spatial components
    // pick up time contributions from the velocity shear.

    // W_vu^-1: anisotropic scaling (B.15). Inverse scales gradient by the
    // forward factors (beta1, beta2) since J_vu = diag(beta1, beta2, 1).
    {
        const auto [beta1, beta2] =
            VelocityUncertaintyFactors(p.velocityUncertaintyX, p.velocityUncertaintyY);
        gx *= beta1;
        gy *= beta2;
    }

    // W_tu^-1 (B.6): spatial gradient scaled by beta, temporal gets cross-terms.
    // Coordinates at W_tu input = after W_ref -> W_v -> W_r on sOriginal.
    // TODO(perf): ComposeForward already computes this intermediate; cache and
    // reuse instead of recomputing the three warps here. ~15% per-sample saving.
    {
        const double t = sOriginal.t;
        const double beta = 1.0 / (1.0 + p.lambda * std::max(t, 0.0));
        auto sAtTu = WarpLocalForward(sOriginal, p.posA, p.orientA, p.posB, p.orientB);
        sAtTu = WarpVelocityForward(sAtTu, p.speedB);
        sAtTu = WarpRadiusForward(sAtTu, p.radiusB);
        const double gamma1 = -p.lambda * beta * beta * sAtTu.x;
        const double gamma2 = -p.lambda * beta * beta * sAtTu.y;
        const double gxOld = gx;
        const double gyOld = gy;
        gx *= beta;
        gy *= beta;
        gt = gamma1 * gxOld + gamma2 * gyOld + gt;
    }

    // W_r^-1: identity (B.9).

    // W_v^-1 (B.12): g + (0, 0, -v·g.x).
    {
        gt -= p.speedB * gx;
    }

    // W_local inverse Jacobian: rotate from b's frame back to a's frame
    {
        const double cosA = p.orientA.x;
        const double sinA = p.orientA.y;
        const double cosB = p.orientB.x;
        const double sinB = p.orientB.y;

        // Combined rotation: b's frame -> world -> a's frame
        // R_a^T * R_b applied to gradient
        const double cos_ab = cosA * cosB + sinA * sinB;
        const double sin_ab = sinA * cosB - cosA * sinB;
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
    double timeHorizon,
    double stepSize,
    double sigma,
    double timeUncertainty,
    double velocityUncertaintyX,
    double velocityUncertaintyY,
    int numSamples,
    uint64_t rngSeed)
    : _timeHorizon(timeHorizon)
    , _stepSize(stepSize)
    , _timeUncertainty(timeUncertainty)
    , _velocityUncertaintyX(velocityUncertaintyX)
    , _velocityUncertaintyY(velocityUncertaintyY)
    , _numSamples(numSamples)
    // Neighborhood cutoff: maximum distance at which a neighbor can still
    // collide with us within timeHorizon. Two agents closing head-on cover
    // 2 * v_max * timeHorizon, plus their combined radii, plus a small margin.
    // v_max and r_max are hardcoded pedestrian defaults; promote to constructor
    // parameters if mixed-speed populations need a tighter or wider cutoff.
    , _cutOffRadius(2.0 * 1.5 * timeHorizon + 2.0 * 0.3 + 0.5)
    , _rng(rngSeed)
{
    _intrinsicField.Compute(sigma);
}

OperationalModelType WarpDriverModel::Type() const
{
    return OperationalModelType::WARP_DRIVER;
}

void WarpDriverModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& /*neighborhoodSearch*/,
    const CollisionGeometry& /*geometry*/) const
{
    const auto* data = std::get_if<State>(&agent.model);
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
}

void WarpDriverModel::ComputeNextState(
    double dT,
    const GenericAgent& current,
    GenericAgent& next,
    const CollisionGeometry& geometry,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const
{
    const auto& agentData = std::get<State>(current.model);
    auto& nextData = std::get<State>(next.model);
    const double speed = agentData.v0;

    // State orientation (unit vector). If zero, default to +x.
    Point orient = agentData.orientation;
    if(orient.Norm() < 1e-9) {
        orient = Point{1.0, 0.0};
    } else {
        orient = orient.Normalized();
    }

    // Direction towards destination
    Point toTarget = current.destination - current.pos;
    const double distToTarget = toTarget.Norm();
    if(distToTarget < 1e-9) {
        // The old update carried default-initialized stuck/detour state here,
        // so applying it reset that state; replicate that reset.
        next.pos = current.pos;
        nextData.orientation = orient;
        nextData.stuckTime = 0.0;
        nextData.anchorX = 0.0;
        nextData.anchorY = 0.0;
        nextData.detourTime = 0.0;
        nextData.detourSide = 1;
        return;
    }
    Point desiredDir = toTarget.Normalized();

    // Use desired direction as agent's effective orientation for the frame
    Point effectiveOrient = desiredDir;

    // === Step 1: Projected trajectory in agent-centric space ===
    // r(t) = (speed * t, 0, t) for t in [0, timeHorizon]
    const double dtSample = _timeHorizon / std::max(_numSamples - 1, 1);

    // === Step 2: Perceive - build collision probability field ===
    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(current.pos, _cutOffRadius);

    // Short-range repulsion: not part of the original Wolinski et al. (2016)
    // model, which is purely anticipatory. Added as a practical safety net
    // because the collision probability field alone cannot guarantee separation
    // when agents are already close (dense crowds, late reactions).
    // Similar to the pushout mechanisms in CFS and AVM.
    Point repulsion{0.0, 0.0};
    for(const auto& neighbor : neighbors) {
        if(neighbor.id == current.id) {
            continue;
        }
        const auto* nbData = std::get_if<State>(&neighbor.model);
        if(!nbData) {
            continue;
        }
        Point diff = current.pos - neighbor.pos;
        const double dist = diff.Norm();
        const double combinedRadius = agentData.radius + nbData->radius;
        if(dist < combinedRadius * 3.0 && dist > 1e-6) {
            const double overlap = combinedRadius * 3.0 - dist;
            repulsion = repulsion + diff.Normalized() * (speed * overlap / dist);
        } else if(dist <= 1e-6) {
            repulsion = repulsion + Point{-desiredDir.y, desiredDir.x} * speed;
        }
    }

    // Random perturbation: small lateral offset on trajectory samples to break
    // symmetry in perfectly aligned head-on encounters where the gradient field
    // cancels by symmetry, producing no lateral avoidance.
    std::uniform_real_distribution<double> perturbDist(-0.05, 0.05);

    // Storage for per-sample combined probability and gradient
    struct Sample {
        double t;
        STP r; // trajectory point in agent-centric space-time
        double pTotal;
        STP gradTotal;
    };
    std::vector<Sample> samples(static_cast<size_t>(_numSamples));

    for(int i = 0; i < _numSamples; ++i) {
        const double t = i * dtSample;
        const double lateralPerturbation = perturbDist(_rng);
        samples[static_cast<size_t>(i)] =
            Sample{t, STP{speed * t, lateralPerturbation, t}, 0.0, STP{0, 0, 0}};
    }

    for(const auto& neighbor : neighbors) {
        if(neighbor.id == current.id) {
            continue;
        }

        const auto* nbData = std::get_if<State>(&neighbor.model);
        if(!nbData) {
            continue;
        }

        // Neighbor orientation
        Point nbOrient = nbData->orientation;
        if(nbOrient.Norm() < 1e-9) {
            nbOrient = Point{1.0, 0.0};
        } else {
            nbOrient = nbOrient.Normalized();
        }

        // Neighbor speed (from v0)
        const double nbSpeed = nbData->v0;

        // TODO(perf): WarpParams and all neighbor-derived constants (orientation,
        // speed, Minkowski radius, rotation matrix cos_ab/sin_ab used in the
        // gradient inverse) are loop-invariant w.r.t. the sample index. Hoist
        // them out of the sample loop and precompute once per (ped, neighbor).
        WarpParams wp{};
        wp.posA = current.pos;
        wp.orientA = effectiveOrient;
        wp.posB = neighbor.pos;
        wp.orientB = nbOrient;
        wp.speedB = nbSpeed;
        wp.radiusB = agentData.radius + nbData->radius; // Minkowski sum
        wp.lambda = _timeUncertainty;
        wp.velocityUncertaintyX = _velocityUncertaintyX;
        wp.velocityUncertaintyY = _velocityUncertaintyY;
        wp.timeHorizon = _timeHorizon;

        for(auto& s : samples) {
            // Forward warp sample point to neighbor's Intrinsic Field space
            STP warped = ComposeForward(s.r, wp);

            // Time validity check: must be in [0, 1] (normalized)
            if(warped.t < 0.0 || warped.t > 1.0) {
                continue;
            }

            // Lookup Intrinsic Field (2D) and apply probability scaling (B.5, B.14)
            auto [intrinsicP, gradI] = _intrinsicField.Sample(warped.x, warped.y);
            const double pB = intrinsicP * ProbabilityScale(s.r, wp);

            if(pB < 1e-12) {
                continue;
            }

            // Transform gradient back to agent's frame
            STP gradB = ComposeGradientInverse(gradI, s.r, wp);

            // Union formula: p_new = p + pB - p * pB
            double pOld = s.pTotal;
            s.pTotal = pOld + pB - pOld * pB;
            s.gradTotal.x = s.gradTotal.x + gradB.x - pOld * gradB.x - pB * s.gradTotal.x;
            s.gradTotal.y = s.gradTotal.y + gradB.y - pOld * gradB.y - pB * s.gradTotal.y;
            s.gradTotal.t = s.gradTotal.t + gradB.t - pOld * gradB.t - pB * s.gradTotal.t;
        }
    }

    // === Step 3: Solve - gradient descent on trajectory ===
    // Integrate N, P, G, S per Eq. 4-7
    double N = 0.0;
    double P = 0.0;
    STP G{0, 0, 0};
    STP S{0, 0, 0};

    for(const auto& s : samples) {
        N += s.pTotal * dtSample;
        P += s.pTotal * s.pTotal * dtSample;
        G.x += s.pTotal * s.gradTotal.x * dtSample;
        G.y += s.pTotal * s.gradTotal.y * dtSample;
        G.t += s.pTotal * s.gradTotal.t * dtSample;
        S.x += s.pTotal * s.r.x * dtSample;
        S.y += s.pTotal * s.r.y * dtSample;
        S.t += s.pTotal * s.r.t * dtSample;
    }

    Point newVelLocal;

    if(N < 1e-9) {
        // No collision risk — follow projected trajectory
        newVelLocal = Point{speed, 0.0};
    } else {
        P /= N;
        G.x /= N;
        G.y /= N;
        G.t /= N;
        S.x /= N;
        S.y /= N;
        S.t /= N;

        // q = S - alpha * P * G  (Eq. 8)
        STP q{};
        q.x = S.x - _stepSize * P * G.x;
        q.y = S.y - _stepSize * P * G.y;
        q.t = S.t - _stepSize * P * G.t;

        if(q.t > 1e-9) {
            newVelLocal = Point{q.x / q.t, q.y / q.t};
        } else {
            newVelLocal = Point{speed, 0.0};
        }
    }

    // Clamp speed to [0, v0]
    const double newSpeed = std::min(newVelLocal.Norm(), agentData.v0);

    // Convert to world coordinates: rotate by effectiveOrient
    Point newVelWorld;
    if(newSpeed > 1e-9) {
        Point newDirLocal = newVelLocal.Normalized();
        // Rotate from agent-centric to world
        newVelWorld =
            Point{
                effectiveOrient.x * newDirLocal.x - effectiveOrient.y * newDirLocal.y,
                effectiveOrient.y * newDirLocal.x + effectiveOrient.x * newDirLocal.y} *
            newSpeed;
    } else {
        newVelWorld = desiredDir * agentData.v0 * 0.01; // tiny push towards goal
    }

    // State repulsion
    newVelWorld = newVelWorld + repulsion;

    // Boundary avoidance: steer agents away from walls
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(current.pos);
    for(const auto& wall : walls) {
        const Point wallVec = wall.p2 - wall.p1;
        const double wallLen2 = wallVec.ScalarProduct(wallVec);
        if(wallLen2 < 1e-12) {
            continue; // degenerate wall segment
        }
        const Point toAgent = current.pos - wall.p1;
        const double t = std::clamp(toAgent.ScalarProduct(wallVec) / wallLen2, 0.0, 1.0);
        const Point closest = wall.p1 + wallVec * t;
        const Point diff = current.pos - closest;
        const double dist = diff.Norm();
        if(dist < agentData.radius * 3.0 && dist > 1e-6) {
            const double steering = agentData.v0 * (agentData.radius * 3.0 - dist) / dist;
            newVelWorld = newVelWorld + diff.Normalized() * steering;
        }
    }

    // Re-clamp speed to v0 after wall steering
    double finalSpeed = newVelWorld.Norm();
    if(finalSpeed > agentData.v0 && finalSpeed > 1e-9) {
        newVelWorld = newVelWorld * (agentData.v0 / finalSpeed);
        finalSpeed = agentData.v0;
    }

    // Stuck detection: measure net displacement from an anchor position over a
    // time window. Catches oscillating agents that periodically spike above the
    // speed threshold but make no real progress.
    double stuckTime = agentData.stuckTime;
    double anchorX = agentData.anchorX;
    double anchorY = agentData.anchorY;
    double detourTime = agentData.detourTime;
    int detourSide = agentData.detourSide;

    // Detour mode: agent is currently on a lateral detour to break a deadlock
    if(detourTime > 0.0) {
        detourTime -= dT;
        Point lateral{-desiredDir.y * detourSide, desiredDir.x * detourSide};
        Point detourDir = (lateral * 0.8 + desiredDir * 0.2).Normalized();
        Point detourVel = detourDir * agentData.v0 * 0.5;
        Point newPos = current.pos + detourVel * dT;
        // If detour would leave the walkable area, try the other side
        if(!geometry.InsideGeometry(newPos)) {
            detourSide = -detourSide;
            lateral = Point{-desiredDir.y * detourSide, desiredDir.x * detourSide};
            detourDir = (lateral * 0.8 + desiredDir * 0.2).Normalized();
            detourVel = detourDir * agentData.v0 * 0.5;
            newPos = current.pos + detourVel * dT;
            // If both sides fail, just creep toward goal
            if(!geometry.InsideGeometry(newPos)) {
                newPos = current.pos + desiredDir * agentData.v0 * 0.1 * dT;
                detourDir = desiredDir;
            }
        }
        if(detourTime <= 0.0) {
            detourTime = 0.0;
            stuckTime = 0.0;
            anchorX = newPos.x;
            anchorY = newPos.y;
        }
        next.pos = newPos;
        nextData.orientation = detourDir;
        nextData.stuckTime = stuckTime;
        nextData.anchorX = anchorX;
        nextData.anchorY = anchorY;
        nextData.detourTime = detourTime;
        nextData.detourSide = detourSide;
        return;
    }

    // Measure net displacement from anchor over the stuck window
    constexpr double stuckThreshold = 5.0; // seconds before triggering detour
    constexpr double detourDuration = 1.0; // seconds of lateral movement
    constexpr double progressRadius = 0.3; // must move this far from anchor to count as progress

    stuckTime += dT;
    const double netDisplacement = std::hypot(current.pos.x - anchorX, current.pos.y - anchorY);

    if(netDisplacement > progressRadius) {
        // Real progress — reset anchor to current position
        stuckTime = 0.0;
        anchorX = current.pos.x;
        anchorY = current.pos.y;
    } else if(stuckTime >= stuckThreshold) {
        // Stuck: no net progress for stuckThreshold seconds — enter detour
        std::uniform_int_distribution<int> sideDist(0, 1);
        detourSide = sideDist(_rng) * 2 - 1; // -1 or +1
        detourTime = detourDuration;
        stuckTime = 0.0;
    }

    // Velocity smoothing: blend new velocity with previous orientation to damp
    // oscillations in dense clusters where agents flip direction every frame.
    const double smoothing = 0.5; // weight of new velocity (1.0 = no smoothing)
    Point smoothedVel = newVelWorld * smoothing + orient * (newVelWorld.Norm() * (1.0 - smoothing));
    double smoothedSpeed = smoothedVel.Norm();
    if(smoothedSpeed > agentData.v0 && smoothedSpeed > 1e-9) {
        smoothedVel = smoothedVel * (agentData.v0 / smoothedSpeed);
    }

    Point newPos = current.pos + smoothedVel * dT;
    Point newOrient = (smoothedVel.Norm() > 1e-9) ? smoothedVel.Normalized() : orient;

    next.pos = newPos;
    nextData.orientation = newOrient;
    nextData.stuckTime = stuckTime;
    nextData.anchorX = anchorX;
    nextData.anchorY = anchorY;
    nextData.detourTime = detourTime;
    nextData.detourSide = detourSide;
}
