// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModel.hpp"

#include "Ellipse.hpp"
#include "GenericAgent.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Simulation.hpp"
#include "SimulationError.hpp"
#include "TacticalModelState.hpp"

#include <Logger.hpp>

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <vector>

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
    _cutOffRadius = 4.0;
}

OperationalModelType GeneralizedCentrifugalForceModel::Type() const
{
    return OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE;
}

void GeneralizedCentrifugalForceModel::ComputeNextState(
    double dT,
    const GenericState& current,
    GenericState& next,
    const TacticalModelState& tactical,
    const CollisionGeometry& geometry,
    const StateContainer& neighborStates) const
{
    const auto& state = std::get<State>(current);
    Point F_rep;
    for(const auto& neighbor : neighborStates) {
        F_rep += ForceRepPed(state, std::get<State>(neighbor));
    }

    // e0 stays default constructed when ForceDriv does not overwrite it, matching the old
    // update struct semantics.
    Point e0{};
    // repulsive forces to the walls and transitions that are not my target
    Point repwall = ForceRepRoom(state, geometry);
    Point fd = ForceDriv(state, tactical.destination, state.mass, state.tau, dT, e0);
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
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& state = std::get<State>(agent.state);

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

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(state.position, 2);
    for(const auto& neighbor : neighbors) {
        if(agent.id == neighbor.id) {
            continue;
        }

        const auto& neighbor_state = std::get<State>(neighbor.state);
        const auto contanctDist = AgentToAgentSpacing(state, neighbor_state);
        const auto distance = (Pos(agent) - Pos(neighbor)).Norm();
        if(contanctDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "contactDist {}, "
                "effective distance {}",
                state.position,
                neighbor_state.position,
                distance,
                contanctDist,
                distance - contanctDist);
        }
    }

    const auto maxRadius = std::max(AMin, BMax) / 2.;
    const auto lineSegments = geometry.LineSegmentsInDistanceTo(maxRadius, state.position);
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
    const auto pos = Pos(state);
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

Point GeneralizedCentrifugalForceModel::ForceRepPed(const State& state1, const State& state2) const
{
    Point F_rep;
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = state2.position - state1.position;
    const Point vp1 = (state1.orientation * state1.speed); // v Ped1
    const Point vp2 = (state2.orientation * state2.speed); // v Ped2
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    double tmp, tmp2;
    double v_ij;
    double K_ij;
    double nom; // nominator of Frep
    double px; // hermite Interpolation value
    const auto dist_eff = AgentToAgentSpacing(state1, state2);
    const auto agent1_mass = state1.mass;

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

    const auto v0_1 = state1.v0;
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
            "NAN return p1{} p2 {} Frepx={:f} Frepy={:f} K_ij={:f}",
            state1,
            state2,
            F_rep.x,
            F_rep.y,
            K_ij);
    }
    return F_rep;
}

/* abstoßende Kraft zwischen ped und subroom
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - subroom: SubRoom für den alle abstoßende Kräfte von Wänden berechnet werden
 * Rückgabewerte:
 *   - Vektor(x,y) mit Summe aller abstoßenden Kräfte im SubRoom
 * */

inline Point GeneralizedCentrifugalForceModel::ForceRepRoom(
    const State& state,
    const CollisionGeometry& geometry) const
{
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(state.position);

    auto f = std::accumulate(
        walls.cbegin(),
        walls.cend(),
        Point(0, 0),
        [this, &state](const auto& acc, const auto& element) {
            return acc + ForceRepWall(state, element);
        });
    return f;
}

inline Point
GeneralizedCentrifugalForceModel::ForceRepWall(const State& state, const LineSegment& w) const
{
    Point F = Point(0.0, 0.0);
    Point pt = w.ShortestPoint(state.position);
    double wlen = w.LengthSquare();

    if(wlen < 0.01) { // ignore walls smaller than 10 cm
        return F;
    }
    // Kraft soll nur orthgonal wirken
    // ???
    if(fabs((w.p1 - w.p2).ScalarProduct(state.position - pt)) > J_EPS) {
        return F;
    }
    double mind = 0.5; // for performance reasons this distance is assumed to be constant
    double vn = w.NormalComp(
        state.orientation * state.speed); // normal component of the velocity on the wall
    F = ForceRepStatPoint(state, pt, mind, vn);

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
    const State& state,
    const Point& p,
    double l,
    double vn) const
{
    Point F_rep = Point(0.0, 0.0);
    // TODO(kkratz): this will fail for speed 0.
    // I think the code can be rewritten to account for orientation and speed separately
    const Point v = state.orientation * state.speed;
    Point dist = p - state.position; // x- and y-coordinate of the distance between ped and p
    double d = dist.Norm(); // distance between the centre of ped and point p
    Point e_ij; // x- and y-coordinate of the normalized vector between ped and p

    double tmp;
    double bla;
    Point r;
    Point pinE; // vorher x1, y1
    const Ellipse E{state.Av, state.AMin, state.BMax, state.BMin};

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
    pinE =
        p.TransformToEllipseCoordinates(state.position, state.orientation.x, state.orientation.y);
    const auto v0 = state.v0;
    // Punkt auf der Ellipse
    r = E.PointOnEllipse(pinE, state.speed / v0, state.position, state.speed, state.orientation);
    // interpolierte Kraft
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
    const State& state1,
    const State& state2) const
{
    const Ellipse E1{state1.Av, state1.AMin, state1.BMax, state1.BMin};
    const Ellipse E2{state2.Av, state2.AMin, state2.BMax, state2.BMin};
    const auto v0_1 = state1.v0;
    const auto v0_2 = state2.v0;
    // Avoid division by zero by setting scale to 1 when v0 is 0
    const double scale1 = (v0_1 == 0.0) ? 1.0 : state1.speed / v0_1;
    const double scale2 = (v0_2 == 0.0) ? 1.0 : state2.speed / v0_2;

    return E1.EffectiveDistanceToEllipse(
        E2,
        state1.position,
        state2.position,
        scale1,
        scale2,
        state1.speed,
        state2.speed,
        state1.orientation,
        state2.orientation);
}
