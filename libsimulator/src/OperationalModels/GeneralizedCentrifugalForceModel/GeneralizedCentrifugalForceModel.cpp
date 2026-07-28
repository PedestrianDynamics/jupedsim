// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModel.hpp"

#include "Ellipse.hpp"
#include "EnvironmentQuery.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "OperationalModelType.hpp"
#include "Simulation.hpp"
#include "SimulationError.hpp"

#include <Logger.hpp>

#include <numeric>
#include <stdexcept>

GeneralizedCentrifugalForceModel::GeneralizedCentrifugalForceModel(
    double strengthNeighborRepulsion_,
    double strengthGeometryRepulsion_,
    double maxNeighborInteractionDistance_,
    double maxGeometryInteractionDistance_,
    double maxNeighborInterpolationDistance_,
    double maxGeometryInterpolationDistance_,
    double maxNeighborRepulsionForce_,
    double maxGeometryRepulsionForce_)
    : strengthNeighborRepulsion(strengthNeighborRepulsion_)
    , strengthGeometryRepulsion(strengthGeometryRepulsion_)
    , maxNeighborInteractionDistance(maxNeighborInteractionDistance_)
    , maxGeometryInteractionDistance(maxGeometryInteractionDistance_)
    , maxNeighborInterpolationDistance(maxNeighborInterpolationDistance_)
    , maxGeometryInterpolationDistance(maxGeometryInterpolationDistance_)
    , maxNeighborRepulsionForce(maxNeighborRepulsionForce_)
    , maxGeometryRepulsionForce(maxGeometryRepulsionForce_)
{
}

OperationalModelType GeneralizedCentrifugalForceModel::Type() const
{
    return OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE;
}

void GeneralizedCentrifugalForceModel::ComputeNextState(
    double dT,
    const OperationalModelState& current,
    OperationalModelState& next,
    const Point& destination,
    const EnvironmentQuery& envQuery) const
{
    const auto& state = std::get<State>(current);
    const auto neighborStates = envQuery.OtherAgentStatesInRange(
        current, _cutOffRadius, [&envQuery, from = state.position](const Point& to) {
            return envQuery.NoGeometryBetween(from, to);
        });

    Point F_rep;
    for(const auto& neighbor : neighborStates) {
        F_rep += ForceRepPed(state, std::get<State>(neighbor));
    }

    Point e0{};
    Point repwall = ForceRepRoom(state, envQuery);
    Point fd = ForceDriv(state, destination, state.mass, state.tau, dT, e0);
    Point acc = (fd + F_rep + repwall) / state.mass;

    const Point velocity = (state.orientation * state.speed) + acc * dT;
    const Point position = state.position + velocity * dT;

    auto& nextState = std::get<State>(next);
    nextState.e0 = e0;
    ++nextState.orientationDelay;
    nextState.position = position;
    nextState.orientation = velocity.Normalized();
    nextState.speed = velocity.Norm();
}

void GeneralizedCentrifugalForceModel::CheckModelConstraint(
    const OperationalModelState& generic_state,
    const EnvironmentQuery& envQuery) const
{
    const auto& state = std::get<State>(generic_state);

    if(!state.orientation.IsUnitLength()) {
        throw SimulationError("Orientation is invalid: {}. Length should be 1.", state.orientation);
    }

    const auto mass = state.mass;
    constexpr double massMin = 1.;
    constexpr double massMax = 100.;
    validateConstraint(mass, massMin, massMax, "mass");

    const auto tau = state.tau;
    constexpr double tauMin = 0.1;
    constexpr double tauMax = 10.;
    validateConstraint(tau, tauMin, tauMax, "tau");

    const auto v0 = state.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto Av = state.Av;
    constexpr double AvMin = 0.;
    constexpr double AvMax = 10.;
    validateConstraint(Av, AvMin, AvMax, "Av");

    const auto AMin = state.AMin;
    constexpr double AMinMin = 0.1;
    constexpr double AMinMax = 1.;
    validateConstraint(AMin, AMinMin, AMinMax, "AMin");

    const auto BMin = state.BMin;
    constexpr double BMinMin = 0.1;
    constexpr double BMinMax = 1.;
    validateConstraint(BMin, BMinMin, BMinMax, "BMin");

    const auto BMax = state.BMax;
    const double BMaxMin = BMin;
    constexpr double BMaxMax = 2.;
    validateConstraint(BMax, BMaxMin, BMaxMax, "BMax");

    const auto neighbors = envQuery.OtherAgentStatesInRange(generic_state, 2.0);
    for(const auto& neighbor : neighbors) {
        const auto& neighborState = std::get<State>(neighbor);
        const auto contanctDist = AgentToAgentSpacing(state, neighborState);
        const auto distance = (state.position - neighborState.position).Norm();
        if(contanctDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "contactDist {}, effective distance {}",
                state.position,
                neighborState.position,
                distance,
                contanctDist,
                distance - contanctDist);
        }
    }

    const auto maxRadius = std::max(state.AMin, state.BMax) / 2.;
    const auto lineSegments = envQuery.LineSegmentsInRange(state.position, maxRadius);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= {}",
            state.position,
            maxRadius);
    }
}

Point GeneralizedCentrifugalForceModel::ForceDriv(
    const State& state,
    Point target,
    double mass,
    double tau,
    double deltaT,
    Point& e0update) const
{
    Point F_driv;
    const auto pos = state.position;
    const auto dist = (target - pos).Norm();
    if(dist > J_EPS_GOAL) {
        const Point e0 = mollify_e0(target, pos, deltaT, state.orientationDelay, state.e0);
        e0update = e0;
        F_driv = ((e0 * state.v0 - (state.orientation * state.speed)) * mass) / tau;
    } else {
        const Point e0 = state.e0;
        F_driv = ((e0 * state.v0 - (state.orientation * state.speed)) * mass) / tau;
    }
    return F_driv;
}

Point GeneralizedCentrifugalForceModel::ForceRepPed(const State& s1, const State& s2) const
{
    Point F_rep;
    Point distp12 = s2.position - s1.position;
    const Point vp1 = (s1.orientation * s1.speed);
    const Point vp2 = (s2.orientation * s2.speed);
    Point ep12;
    double tmp, tmp2;
    double v_ij;
    double K_ij;
    double nom;
    double px;
    const auto dist_eff = AgentToAgentSpacing(s1, s2);
    const auto agent1_mass = s1.mass;

    if(dist_eff >= maxNeighborInteractionDistance) {
        F_rep = Point(0.0, 0.0);
        return F_rep;
    }

    const double mindist = 0.5;
    const double dist_intpol_left = mindist + maxNeighborInterpolationDistance;
    const double dist_intpol_right =
        maxNeighborInteractionDistance - maxNeighborInterpolationDistance;
    const double smax = mindist - maxNeighborInterpolationDistance;
    double f = 0.0f;
    double f1 = 0.0f;

    if(distp12.Norm() >= J_EPS) {
        ep12 = distp12.Normalized();
    } else {
        LOG_WARNING(
            "Distance between two pedestrians is small ({}<{}). Force can not be calculated.",
            distp12.Norm(),
            J_EPS);
        return F_rep;
    }
    tmp = (vp1 - vp2).ScalarProduct(ep12);
    v_ij = 0.5 * (tmp + fabs(tmp));
    tmp2 = vp1.ScalarProduct(ep12);

    if(vp1.Norm() < J_EPS) {
        K_ij = 0;
    } else {
        double bla = tmp2 + fabs(tmp2);
        K_ij = 0.25 * bla * bla / vp1.ScalarProduct(vp1);

        if(K_ij < J_EPS * J_EPS) {
            F_rep = Point(0.0, 0.0);
            return F_rep;
        }
    }

    const auto v0_1 = s1.v0;
    nom = strengthNeighborRepulsion * v0_1 + v_ij;
    nom *= nom;

    K_ij = sqrt(K_ij);
    if(dist_eff <= smax) {
        f = -agent1_mass * K_ij * nom / dist_intpol_left;
        F_rep = ep12 * maxNeighborRepulsionForce * f;
        return F_rep;
    }

    if(dist_eff >= dist_intpol_right) {
        f = -agent1_mass * K_ij * nom / dist_intpol_right;
        f1 = -f / dist_intpol_right;
        px = hermite_interp(
            dist_eff, dist_intpol_right, maxNeighborInteractionDistance, f, 0, f1, 0);
        F_rep = ep12 * px;
    } else if(dist_eff >= dist_intpol_left) {
        f = -agent1_mass * K_ij * nom / fabs(dist_eff);
        F_rep = ep12 * f;
    } else {
        f = -agent1_mass * K_ij * nom / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, maxNeighborRepulsionForce * f, f, 0, f1);
        F_rep = ep12 * px;
    }
    if(F_rep.x != F_rep.x || F_rep.y != F_rep.y) {
        LOG_ERROR(
            "NAN return p1={} p2={} Frepx={:f} Frepy={:f} K_ij={:f}",
            s1.position,
            s2.position,
            F_rep.x,
            F_rep.y,
            K_ij);
    }
    return F_rep;
}

inline Point GeneralizedCentrifugalForceModel::ForceRepRoom(
    const State& state,
    const EnvironmentQuery& envQuery) const
{
    const auto& walls = envQuery.LineSegmentsInRange(state.position);
    return std::accumulate(
        std::begin(walls),
        std::end(walls),
        Point(0, 0),
        [this, &state](const auto& acc, const auto& element) {
            return acc + ForceRepWall(state, element);
        });
}

inline Point
GeneralizedCentrifugalForceModel::ForceRepWall(const State& state, const LineSegment& w) const
{
    Point F = Point(0.0, 0.0);
    Point pt = w.ShortestPoint(state.position);
    double wlen = w.LengthSquare();

    if(wlen < 0.01) {
        return F;
    }
    if(fabs((w.p1 - w.p2).ScalarProduct(state.position - pt)) > J_EPS) {
        return F;
    }
    double mind = 0.5;
    double vn = w.NormalComp(state.orientation * state.speed);
    F = ForceRepStatPoint(state, pt, mind, vn);

    return F;
}

Point GeneralizedCentrifugalForceModel::ForceRepStatPoint(
    const State& state,
    const Point& p,
    double l,
    double vn) const
{
    Point F_rep = Point(0.0, 0.0);
    const Point v = state.orientation * state.speed;
    Point dist = p - state.position;
    double d = dist.Norm();
    Point e_ij;

    double tmp;
    double bla;
    Point r;
    Point pinE;
    const Ellipse E{state.Av, state.AMin, state.BMax, state.BMin};

    if(d < J_EPS)
        return Point(0.0, 0.0);
    e_ij = dist / d;
    tmp = v.ScalarProduct(e_ij);
    bla = (tmp + fabs(tmp));
    if(!bla)
        return Point(0.0, 0.0);
    if(fabs(v.x) < J_EPS && fabs(v.y) < J_EPS)
        return Point(0.0, 0.0);
    double K_ij;
    K_ij = 0.5 * bla / v.Norm();
    pinE =
        p.TransformToEllipseCoordinates(state.position, state.orientation.x, state.orientation.y);
    const auto v0 = state.v0;
    r = E.PointOnEllipse(pinE, state.speed / v0, state.position, state.speed, state.orientation);
    F_rep = ForceInterpolation(v0, K_ij, e_ij, vn, d, (r - state.position).Norm(), l);
    return F_rep;
}

Point GeneralizedCentrifugalForceModel::ForceInterpolation(
    double v0,
    double K_ij,
    const Point& e,
    double vn,
    double d,
    double r,
    double l) const
{
    Point F_rep;
    double nominator = strengthGeometryRepulsion * v0 + vn;
    nominator *= nominator * K_ij;
    double f = 0, f1 = 0;
    double smax = l - maxGeometryInterpolationDistance;
    double dist_intpol_left = l + maxGeometryInterpolationDistance;
    double dist_intpol_right = maxGeometryInteractionDistance - maxGeometryInterpolationDistance;

    double dist_eff = d - r;

    double px = 0;
    double tmp1 = maxGeometryInteractionDistance;
    double tmp2 = dist_intpol_right;
    double tmp3 = dist_intpol_left;
    double tmp5 = smax + r;

    if(dist_eff >= tmp1) {
        return F_rep;
    }

    if(dist_eff <= tmp5) {
        F_rep = e * (-maxGeometryRepulsionForce);
        return F_rep;
    }

    if(dist_eff > tmp2) {
        f = -nominator / dist_intpol_right;
        f1 = -f / dist_intpol_right;
        px = hermite_interp(
            dist_eff, dist_intpol_right, maxGeometryInteractionDistance, f, 0, f1, 0);
        F_rep = e * px;
    } else if(dist_eff >= tmp3) {
        f = -nominator / fabs(dist_eff);
        F_rep = e * f;
    } else {
        f = -nominator / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, maxGeometryRepulsionForce * f, f, 0, f1);
        F_rep = e * px;
    }
    return F_rep;
}

double GeneralizedCentrifugalForceModel::AgentToAgentSpacing(const State& s1, const State& s2) const
{
    const Ellipse E1{s1.Av, s1.AMin, s1.BMax, s1.BMin};
    const Ellipse E2{s2.Av, s2.AMin, s2.BMax, s2.BMin};
    const auto v0_1 = s1.v0;
    const auto v0_2 = s2.v0;
    const double scale1 = (v0_1 == 0.0) ? 1.0 : s1.speed / v0_1;
    const double scale2 = (v0_2 == 0.0) ? 1.0 : s2.speed / v0_2;

    return E1.EffectiveDistanceToEllipse(
        E2,
        s1.position,
        s2.position,
        scale1,
        scale2,
        s1.speed,
        s2.speed,
        s1.orientation,
        s2.orientation);
}
