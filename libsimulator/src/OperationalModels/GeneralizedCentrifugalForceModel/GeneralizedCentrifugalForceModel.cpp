// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModel.hpp"

#include "AgentView.hpp"
#include "Ellipse.hpp"
#include "GenericAgent.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Simulation.hpp"
#include "SimulationError.hpp"

#include <Logger.hpp>

#include <optional>
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

Point GeneralizedCentrifugalForceModel::ComputeNextState(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& currentState = std::get<State>(current);
    const auto neighborhood = step.OtherAgentsInRange(
        _cutOffRadius, [&step](const NeighborView& n) { return step.NoGeometryBetween(n); });
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        F_rep += ForceRepPed(currentState, neighbor);
    }

    // ForceDriv leaves e0 untouched when the agent has practically arrived; the default
    // constructed value is what gets stored then.
    Point e0{};
    // repulsive forces to the walls and transitions that are not my target
    Point repwall{};
    for(const auto& wall : step.WallsNearby()) {
        repwall += ForceRepWall(currentState, wall);
    }

    const Point fd = ForceDriv(
        currentState,
        step.orientation_to_next_target(),
        currentState.mass,
        currentState.tau,
        step.dt(),
        e0);
    const Point acc = (fd + F_rep + repwall) / currentState.mass;

    const Point velocity = (currentState.orientation * currentState.speed) + acc * step.dt();

    auto& nextModel = std::get<State>(next);
    nextModel.e0 = e0;
    ++nextModel.orientationDelay;
    nextModel.orientation = velocity.Normalized();
    nextModel.speed = velocity.Norm();
    return velocity * step.dt();
}

void GeneralizedCentrifugalForceModel::CheckModelConstraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& currentState = std::get<State>(agent.state);

    if(!currentState.orientation.IsUnitLength()) {
        throw SimulationError(
            "Orientation is invalid: {}. Length should be 1.", currentState.orientation);
    }

    const auto mass = currentState.mass;
    constexpr double massMin = 1.;
    constexpr double massMax = 100.;
    validateConstraint(mass, massMin, massMax, "mass");

    const auto tau = currentState.tau;
    constexpr double tauMin = 0.1;
    constexpr double tauMax = 10.;
    validateConstraint(tau, tauMin, tauMax, "tau");

    const auto v0 = currentState.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto Av = currentState.Av;
    constexpr double AvMin = 0.;
    constexpr double AvMax = 10.;
    validateConstraint(Av, AvMin, AvMax, "Av");

    const auto AMin = currentState.AMin;
    constexpr double AMinMin = 0.1;
    constexpr double AMinMax = 1.;
    validateConstraint(AMin, AMinMin, AMinMax, "AMin");

    const auto BMin = currentState.BMin;
    constexpr double BMinMin = 0.1;
    constexpr double BMinMax = 1.;
    validateConstraint(BMin, BMinMin, BMinMax, "BMin");

    const auto BMax = currentState.BMax;
    const double BMaxMin = BMin;
    constexpr double BMaxMax = 2.;
    validateConstraint(BMax, BMaxMin, BMaxMax, "BMax");

    const auto neighbors = view.OtherAgentsInRange(2.0);
    for(const auto& neighbor : neighbors) {
        const auto contanctDist = AgentToAgentSpacing(currentState, neighbor);
        const auto distance = neighbor.RelativePosition.Norm();
        if(contanctDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent at {} too close to agent at {}: distance {}, "
                "contactDist {}, "
                "effective distance {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.RelativePosition,
                distance,
                contanctDist,
                distance - contanctDist);
        }
    }

    const auto maxRadius = std::max(AMin, BMax) / 2.;
    if(!view.WallsInRange(maxRadius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= {}",
            agent.location.xy(),
            maxRadius);
    }
}

Point GeneralizedCentrifugalForceModel::ForceDriv(
    const State& currentState,
    Point orientationToTarget,
    double mass,
    double tau,
    double deltaT,
    Point& e0update) const
{
    Point F_driv;
    if(orientationToTarget != Point{}) {
        // expect this to never trigger as stage system should cover it.
        const Point e0 =
            mollify_e0(orientationToTarget, deltaT, currentState.orientationDelay, currentState.e0);
        e0update = e0;
        F_driv =
            ((e0 * currentState.v0 - (currentState.orientation * currentState.speed)) * mass) / tau;
    } else {
        const Point e0 = currentState.e0;
        F_driv =
            ((e0 * currentState.v0 - (currentState.orientation * currentState.speed)) * mass) / tau;
    }
    return F_driv;
}

Point GeneralizedCentrifugalForceModel::ForceRepPed(
    const State& currentState,
    const NeighborView& neighbor) const
{
    const auto& neighborState = std::get<State>(*neighbor.state);
    Point F_rep;
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = neighbor.RelativePosition;
    const Point vp1 = (currentState.orientation * currentState.speed); // v Ped1
    const Point vp2 = (neighborState.orientation * neighborState.speed); // v Ped2
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    double tmp, tmp2;
    double v_ij;
    double K_ij;
    double nom; // nominator of Frep
    double px; // hermite Interpolation value
    const auto dist_eff = AgentToAgentSpacing(currentState, neighbor);
    const auto agent1_mass = currentState.mass;

    //          smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //       ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    // If the pedestrian is outside the cutoff distance, the force is zero.
    if(dist_eff >= maxNeighborInteractionDistance) {
        F_rep = Point(0.0, 0.0);
        return F_rep;
    }

    const double mindist =
        0.5; // for performance reasons, it is assumed that this distance is about 50 cm
    const double dist_intpol_left =
        mindist + maxNeighborInterpolationDistance; // lower cut-off for Frep (modCFM)
    const double dist_intpol_right =
        maxNeighborInteractionDistance -
        maxNeighborInterpolationDistance; // upper cut-off for Frep (modCFM)
    const double smax = mindist - maxNeighborInterpolationDistance; // max overlapping
    double f = 0.0f; // fuction value
    double f1 = 0.0f; // derivative of function value

    // todo: runtime normsquare?
    if(distp12.Norm() >= J_EPS) {
        ep12 = distp12.Normalized();

    } else {
        LOG_WARNING(
            "Distance between two pedestrians is small ({}<{}). Force can not be calculated.",
            distp12.Norm(),
            J_EPS);
        return F_rep; // Parameter values are not chosen wisely --> unrealistic overlaping ...
                      // ignore.
    }
    // calculate the parameter (whatever dist is)
    tmp = (vp1 - vp2).ScalarProduct(ep12); // < v_ij , e_ij >
    v_ij = 0.5 * (tmp + fabs(tmp));
    tmp2 = vp1.ScalarProduct(ep12); // < v_i , e_ij >

    // todo: runtime normsquare?
    if(vp1.Norm() < J_EPS) { // if(norm(v_i)==0)
        K_ij = 0;
    } else {
        double bla = tmp2 + fabs(tmp2);
        K_ij = 0.25 * bla * bla / vp1.ScalarProduct(vp1); // squared

        if(K_ij < J_EPS * J_EPS) {
            F_rep = Point(0.0, 0.0);
            return F_rep;
        }
    }

    const auto v0_1 = currentState.v0;
    nom = strengthNeighborRepulsion * v0_1 + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
    nom *= nom;

    K_ij = sqrt(K_ij);
    if(dist_eff <= smax) { // 5
        f = -agent1_mass * K_ij * nom / dist_intpol_left;
        F_rep = ep12 * maxNeighborRepulsionForce * f;
        return F_rep;
    }

    //          smax    dist_intpol_left           dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //           5   |     4       |            3             |      2       | 1
    if(dist_eff >= dist_intpol_right) { // 2
        f = -agent1_mass * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
        f1 = -f / dist_intpol_right;
        px = hermite_interp(
            dist_eff, dist_intpol_right, maxNeighborInteractionDistance, f, 0, f1, 0);
        F_rep = ep12 * px;
    } else if(dist_eff >= dist_intpol_left) { // 3
        f = -agent1_mass * K_ij * nom / fabs(dist_eff); // abs(NR-Dv(i)+Sa)
        F_rep = ep12 * f;
    } else { // 4
        f = -agent1_mass * K_ij * nom / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, maxNeighborRepulsionForce * f, f, 0, f1);
        F_rep = ep12 * px;
    }
    if(F_rep.x != F_rep.x || F_rep.y != F_rep.y) {
        LOG_ERROR(
            "NAN return distp12 {} Frepx={:f} Frepy={:f} K_ij={:f}",
            distp12,
            F_rep.x,
            F_rep.y,
            K_ij);
    }
    return F_rep;
}

inline Point GeneralizedCentrifugalForceModel::ForceRepWall(
    const State& currentState,
    const WallView& wall) const
{
    Point F = Point(0.0, 0.0);
    const auto& w = wall.segment;

    if(w.LengthSquare() < 0.01) { // ignore walls smaller than 10 cm
        return F;
    }
    // Kraft soll nur orthgonal wirken
    // ???
    if(fabs((w.p1 - w.p2).ScalarProduct(Point{} - wall.closest_point)) > J_EPS) {
        return F;
    }
    double mind = 0.5; // for performance reasons this distance is assumed to be constant
    double vn = w.NormalComp(
        currentState.orientation *
        currentState.speed); // normal component of the velocity on the wall
    F = ForceRepStatPoint(currentState, wall.closest_point, mind, vn);

    return F; // line --> l != 0
}

/* abstoßende Punktkraft zwischen ped und Punkt p
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - p: Punkt von dem die Kaft wirkt
 *   - l: Parameter zur Käfteinterpolation
 *   - vn: Parameter zur Käfteinterpolation
 * Rückgabewerte:
 *   - Vektor(x,y) mit abstoßender Kraft
 * */
// TODO: use effective DistanceToEllipse and simplify this function.
Point GeneralizedCentrifugalForceModel::ForceRepStatPoint(
    const State& currentState,
    const Point& p,
    double l,
    double vn) const
{
    Point F_rep = Point(0.0, 0.0);
    // TODO(kkratz): this will fail for speed 0.
    // I think the code can be rewritten to account for orientation and speed separately
    const Point v = currentState.orientation * currentState.speed;
    Point dist = p; // p is relative to the agent, so it already is the distance vector
    double d = dist.Norm(); // distance between the centre of ped and point p
    Point e_ij; // x- and y-coordinate of the normalized vector between ped and p

    double tmp;
    double bla;
    Point r;
    Point pinE; // vorher x1, y1
    const Ellipse E{currentState.Av, currentState.AMin, currentState.BMax, currentState.BMin};

    if(d < J_EPS)
        return Point(0.0, 0.0);
    e_ij = dist / d;
    tmp = v.ScalarProduct(e_ij); // < v_i , e_ij >;
    bla = (tmp + fabs(tmp));
    if(!bla) // Fussgaenger nicht im Sichtfeld
        return Point(0.0, 0.0);
    if(fabs(v.x) < J_EPS && fabs(v.y) < J_EPS) // v==0)
        return Point(0.0, 0.0);
    double K_ij;
    K_ij = 0.5 * bla / v.Norm(); // K_ij
    // Punkt auf der Ellipse
    pinE = p.TransformToEllipseCoordinates(
        Point{}, currentState.orientation.x, currentState.orientation.y);
    const auto v0 = currentState.v0;
    // Punkt auf der Ellipse
    r = E.PointOnEllipse(
        pinE, currentState.speed / v0, Point{}, currentState.speed, currentState.orientation);
    // interpolierte Kraft
    F_rep = ForceInterpolation(v0, K_ij, e_ij, vn, d, r.Norm(), l);
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
    double f = 0, f1 = 0; // function value and its derivative at the interpolation point
    double smax = l - maxGeometryInterpolationDistance; // max overlapping radius
    double dist_intpol_left = l + maxGeometryInterpolationDistance; // r_eps
    double dist_intpol_right = maxGeometryInteractionDistance - maxGeometryInterpolationDistance;

    double dist_eff = d - r;

    //         smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    double px = 0; // value of the interpolated function
    double tmp1 = maxGeometryInteractionDistance;
    double tmp2 = dist_intpol_right;
    double tmp3 = dist_intpol_left;
    double tmp5 = smax + r;

    if(dist_eff >= tmp1) { // 1
        // F_rep = Point(0.0, 0.0);
        return F_rep;
    }

    if(dist_eff <= tmp5) { // 5
        F_rep = e * (-maxGeometryRepulsionForce);
        return F_rep;
    }

    if(dist_eff > tmp2) { // 2
        f = -nominator / dist_intpol_right;
        f1 = -f / dist_intpol_right; // nominator / (dist_intpol_right^2) = derivativ of f
        px = hermite_interp(
            dist_eff, dist_intpol_right, maxGeometryInteractionDistance, f, 0, f1, 0);
        F_rep = e * px;
    } else if(dist_eff >= tmp3) { // 3
        f = -nominator / fabs(dist_eff); // devided by abs f the effective distance
        F_rep = e * f;
    } else { // 4 d > smax FIXME
        f = -nominator / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, maxGeometryRepulsionForce * f, f, 0, f1);
        F_rep = e * px;
    }
    return F_rep;
}
double GeneralizedCentrifugalForceModel::AgentToAgentSpacing(
    const State& currentState,
    const NeighborView& neighbor) const
{
    const auto& neighborState = std::get<State>(*neighbor.state);
    const Ellipse E1{currentState.Av, currentState.AMin, currentState.BMax, currentState.BMin};
    const Ellipse E2{neighborState.Av, neighborState.AMin, neighborState.BMax, neighborState.BMin};
    const auto v0_1 = currentState.v0;
    const auto v0_2 = neighborState.v0;
    // Avoid division by zero by setting scale to 1 when v0 is 0
    const double scale1 = (v0_1 == 0.0) ? 1.0 : currentState.speed / v0_1;
    const double scale2 = (v0_2 == 0.0) ? 1.0 : neighborState.speed / v0_2;

    // The ellipse distance is translation invariant, so we evaluate it in the frame of the
    // agent that asked, which sits at the origin.
    return E1.EffectiveDistanceToEllipse(
        E2,
        Point{0.0, 0.0},
        neighbor.RelativePosition,
        scale1,
        scale2,
        currentState.speed,
        neighborState.speed,
        currentState.orientation,
        neighborState.orientation);
}
