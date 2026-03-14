// SPDX-License-Identifier: LGPL-3.0-or-later
#include "WarpDriverModel.hpp"

#include "GenericAgent.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "WarpDriverModelData.hpp"
#include "WarpDriverModelUpdate.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
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

    const double sigma2 = sigma * sigma;

    // Compute I(x,y) = (f * g)(x,y) where g = unit disk, f = Gaussian(sigma).
    // For each grid point, numerically integrate the convolution over the disk.
    const double intStep = 0.05;
    const double intRadius = 1.0 + 4.0 * sigma; // integration bound

    for(int ix = 0; ix < nx; ++ix) {
        for(int iy = 0; iy < ny; ++iy) {
            const double px = xMin + ix * dx;
            const double py = yMin + iy * dy;

            double val = 0.0;
            // Integrate f(px-u, py-v) * g(u,v) du dv over g's support (unit disk)
            for(double u = -intRadius; u <= intRadius; u += intStep) {
                for(double v = -intRadius; v <= intRadius; v += intStep) {
                    if(u * u + v * v <= 1.0) {
                        double dx2 = px - u;
                        double dy2 = py - v;
                        val += std::exp(-(dx2 * dx2 + dy2 * dy2) / (2.0 * sigma2));
                    }
                }
            }
            val *= intStep * intStep;
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
    const double val = v00 * (1 - sx) * (1 - sy) + v10 * sx * (1 - sy) + v01 * (1 - sx) * sy +
                       v11 * sx * sy;

    const Point g00 = gradients[idx(ix, iy)];
    const Point g10 = gradients[idx(ix + 1, iy)];
    const Point g01 = gradients[idx(ix, iy + 1)];
    const Point g11 = gradients[idx(ix + 1, iy + 1)];
    const Point grad =
        g00 * ((1 - sx) * (1 - sy)) + g10 * (sx * (1 - sy)) + g01 * ((1 - sx) * sy) +
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
STP WarpLocalForward(
    const STP& s,
    Point posA,
    Point orientA,
    Point posB,
    Point orientB)
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

// W_r: radius scaling. Scale (x,y) by 1/r_b
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

// W_vu: velocity uncertainty. Scale (x,y) by 1/(1 + mu*|v_b|)
STP WarpVelocityUncertaintyForward(const STP& s, double mu, double speedB)
{
    const double scale = 1.0 / (1.0 + mu * speedB);
    return STP{s.x * scale, s.y * scale, s.t};
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
    double mu;
    double timeHorizon;
};

STP ComposeForward(const STP& s, const WarpParams& p)
{
    auto s1 = WarpLocalForward(s, p.posA, p.orientA, p.posB, p.orientB);
    auto s2 = WarpVelocityForward(s1, p.speedB);
    auto s3 = WarpRadiusForward(s2, p.radiusB);
    auto s4 = WarpTimeUncertaintyForward(s3, p.lambda);
    auto s5 = WarpVelocityUncertaintyForward(s4, p.mu, p.speedB);
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

    // W_vu inverse Jacobian: scale (x,y) by (1 + mu*speedB)
    {
        const double scale = 1.0 + p.mu * p.speedB;
        gx *= scale;
        gy *= scale;
    }

    // W_ts inverse Jacobian: scale (x,y) by (1 + lambda*t), plus cross-term
    {
        const double t = sOriginal.t; // time in a's frame
        const double scale = 1.0 + p.lambda * std::max(t, 0.0);
        gx *= scale;
        gy *= scale;
        // Cross-term: d/dt of scaling introduces spatial->temporal coupling
        // but since we only need the spatial gradient for velocity correction,
        // the temporal component is not used in the final solve.
    }

    // W_r inverse Jacobian: scale (x,y) by radiusB
    {
        gx *= p.radiusB;
        gy *= p.radiusB;
    }

    // W_v inverse Jacobian: shear introduces time component
    // x_original = x_warped + speedB * t, so dx/dx_warped = 1, dx/dt = speedB
    // gradient: gx stays, gt += gx * speedB
    {
        gt += gx * p.speedB;
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
    double velocityUncertainty,
    int numSamples,
    double jamSpeedThreshold,
    int jamStepCount)
    : _timeHorizon(timeHorizon)
    , _stepSize(stepSize)
    , _sigma(sigma)
    , _timeUncertainty(timeUncertainty)
    , _velocityUncertainty(velocityUncertainty)
    , _numSamples(numSamples)
    , _jamSpeedThreshold(jamSpeedThreshold)
    , _jamStepCount(jamStepCount)
    , _cutOffRadius(3.0 * timeHorizon) // conservative estimate
{
    _intrinsicField.Compute(sigma);
}

OperationalModelType WarpDriverModel::Type() const
{
    return OperationalModelType::WARP_DRIVER;
}

std::unique_ptr<OperationalModel> WarpDriverModel::Clone() const
{
    return std::make_unique<WarpDriverModel>(*this);
}

void WarpDriverModel::ApplyUpdate(
    const OperationalModelUpdate& update,
    GenericAgent& agent) const
{
    const auto& upd = std::get<WarpDriverModelUpdate>(update);
    agent.pos = upd.position;
    agent.orientation = upd.orientation;
    auto& data = std::get<WarpDriverModelData>(agent.model);
    data.jamCounter = upd.jamCounter;
}

void WarpDriverModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& /*neighborhoodSearch*/,
    const CollisionGeometry& /*geometry*/) const
{
    const auto* data = std::get_if<WarpDriverModelData>(&agent.model);
    if(!data) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} does not have WarpDriverModelData",
            agent.id);
    }
    if(data->radius <= 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid radius {}", agent.id,
            data->radius);
    }
    if(data->v0 < 0.0) {
        throw SimulationError(
            "WarpDriverModel constraint check: agent {} has invalid v0 {}", agent.id, data->v0);
    }
}

OperationalModelUpdate WarpDriverModel::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const auto& agentData = std::get<WarpDriverModelData>(ped.model);
    const double speed = agentData.v0;

    // Agent orientation (unit vector). If zero, default to +x.
    Point orient = ped.orientation;
    if(orient.Norm() < 1e-9) {
        orient = Point{1.0, 0.0};
    } else {
        orient = orient.Normalized();
    }

    // Direction towards destination
    Point toTarget = ped.destination - ped.pos;
    const double distToTarget = toTarget.Norm();
    if(distToTarget < 1e-9) {
        return WarpDriverModelUpdate{ped.pos, orient};
    }
    Point desiredDir = toTarget.Normalized();

    // Use desired direction as agent's effective orientation for the frame
    Point effectiveOrient = desiredDir;

    // === Step 1: Projected trajectory in agent-centric space ===
    // r(t) = (speed * t, 0, t) for t in [0, timeHorizon]
    const double dtSample = _timeHorizon / std::max(_numSamples - 1, 1);

    // === Step 2: Perceive - build collision probability field ===
    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(ped.pos, _cutOffRadius);

    // Random perturbation: small lateral offset on trajectory samples to break
    // symmetry in perfectly aligned head-on encounters where the gradient field
    // cancels by symmetry, producing no lateral avoidance.
    std::mt19937 rng(static_cast<uint32_t>(
        std::hash<double>{}(ped.pos.x * 1000.0 + ped.pos.y) ^ ped.id.getID()));
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
        const double lateralPerturbation = perturbDist(rng);
        samples[static_cast<size_t>(i)] =
            Sample{t, STP{speed * t, lateralPerturbation, t}, 0.0, STP{0, 0, 0}};
    }

    for(const auto& neighbor : neighbors) {
        if(neighbor.id == ped.id) {
            continue;
        }

        const auto* nbData = std::get_if<WarpDriverModelData>(&neighbor.model);
        if(!nbData) {
            continue;
        }

        // Neighbor orientation
        Point nbOrient = neighbor.orientation;
        if(nbOrient.Norm() < 1e-9) {
            nbOrient = Point{1.0, 0.0};
        } else {
            nbOrient = nbOrient.Normalized();
        }

        // Neighbor speed (from v0)
        const double nbSpeed = nbData->v0;

        WarpParams wp{};
        wp.posA = ped.pos;
        wp.orientA = effectiveOrient;
        wp.posB = neighbor.pos;
        wp.orientB = nbOrient;
        wp.speedB = nbSpeed;
        wp.radiusB = agentData.radius + nbData->radius; // Minkowski sum
        wp.lambda = _timeUncertainty;
        wp.mu = _velocityUncertainty;
        wp.timeHorizon = _timeHorizon;

        for(auto& s : samples) {
            // Forward warp sample point to neighbor's Intrinsic Field space
            STP warped = ComposeForward(s.r, wp);

            // Time validity check: must be in [0, 1] (normalized)
            if(warped.t < 0.0 || warped.t > 1.0) {
                continue;
            }

            // Lookup Intrinsic Field (2D)
            auto [pB, gradI] = _intrinsicField.Sample(warped.x, warped.y);

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
        newVelWorld = Point{
            effectiveOrient.x * newDirLocal.x - effectiveOrient.y * newDirLocal.y,
            effectiveOrient.y * newDirLocal.x + effectiveOrient.x * newDirLocal.y} *
                      newSpeed;
    } else {
        newVelWorld = desiredDir * agentData.v0 * 0.01; // tiny push towards goal
    }

    // Boundary avoidance: steer agents away from walls
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);
    for(const auto& wall : walls) {
        const Point wallVec = wall.p2 - wall.p1;
        const Point toAgent = ped.pos - wall.p1;
        const double t =
            std::clamp(toAgent.ScalarProduct(wallVec) / wallVec.ScalarProduct(wallVec), 0.0, 1.0);
        const Point closest = wall.p1 + wallVec * t;
        const Point diff = ped.pos - closest;
        const double dist = diff.Norm();
        if(dist < agentData.radius * 3.0 && dist > 1e-6) {
            const double steering = agentData.v0 * (agentData.radius * 3.0 - dist) / dist;
            newVelWorld = newVelWorld + diff.Normalized() * steering;
        }
    }

    // Jam detection: if speed is below threshold, increment counter
    int jamCounter = agentData.jamCounter;
    if(newSpeed < _jamSpeedThreshold) {
        ++jamCounter;
        if(jamCounter >= _jamStepCount) {
            // Chill mode: skip avoidance correction, just stay in place
            return WarpDriverModelUpdate{ped.pos, orient, jamCounter};
        }
    } else {
        jamCounter = 0;
    }

    Point newPos = ped.pos + newVelWorld * dT;
    Point newOrient = (newVelWorld.Norm() > 1e-9) ? newVelWorld.Normalized() : orient;

    return WarpDriverModelUpdate{newPos, newOrient, jamCounter};
}
