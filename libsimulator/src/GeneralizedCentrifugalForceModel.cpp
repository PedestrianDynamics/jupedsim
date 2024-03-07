// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModel.hpp"

#include "Ellipse.hpp"
#include "GeneralizedCentrifugalForceModelData.hpp"
#include "GenericAgent.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Simulation.hpp"

#include <Logger.hpp>
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

OperationalModelUpdate GeneralizedCentrifugalForceModel::ComputeNewPosition(
    double dT,
    const GenericAgent& agent,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const double radius = 4.0; // TODO (MC) check this free parameter
    const auto neighborhood = neighborhoodSearch.GetNeighboringAgents(agent.pos, radius);
    const auto p1 = agent.pos;
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        // TODO(schroedtert): Only use neighbors who have an unobstructed line of sight to the
        // current agent
        if(neighbor.id == agent.id) {
            continue;
        }
        if(!geometry.IntersectsAny(LineSegment(p1, neighbor.pos))) {
            F_rep += ForceRepPed(agent, neighbor);
        }
    }

    GeneralizedCentrifugalForceModelUpdate update{};
    // repulsive forces to the walls and transitions that are not my target
    Point repwall = ForceRepRoom(agent, geometry);
    const auto& model = std::get<GeneralizedCentrifugalForceModelData>(agent.model);
    Point fd = ForceDriv(agent, agent.destination, model.mass, model.tau, dT, update);
    Point acc = (fd + F_rep + repwall) / model.mass;

    update.velocity = (agent.orientation * model.speed) + acc * dT;
    update.position = agent.pos + *update.velocity * dT;
    return update;
}

void GeneralizedCentrifugalForceModel::ApplyUpdate(
    const OperationalModelUpdate& upd,
    GenericAgent& agent) const
{
    auto& model = std::get<GeneralizedCentrifugalForceModelData>(agent.model);
    const auto& update = std::get<GeneralizedCentrifugalForceModelUpdate>(upd);
    model.e0 = update.e0;
    ++model.orientationDelay;
    if(update.position) {
        agent.pos = *update.position;
    }
    if(update.velocity) {
        agent.orientation = (*update.velocity).Normalized();
        model.speed = (*update.velocity).Norm();
    }
}

void GeneralizedCentrifugalForceModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    const auto& model = std::get<GeneralizedCentrifugalForceModelData>(agent.model);

    const auto mass = model.mass;
    constexpr double massMin = 1.;
    constexpr double massMax = 100.;
    validateConstraint(mass, massMin, massMax, "mass");

    const auto tau = model.tau;
    constexpr double tauMin = 0.1;
    constexpr double tauMax = 10.;
    validateConstraint(tau, tauMin, tauMax, "tau");

    const auto v0 = model.v0;
    constexpr double v0Min = 0.;
    constexpr double v0Max = 10.;
    validateConstraint(v0, v0Min, v0Max, "v0");

    const auto Av = model.Av;
    constexpr double AvMin = 0.;
    constexpr double AvMax = 10.;
    validateConstraint(Av, AvMin, AvMax, "Av");

    const auto AMin = model.AMin;
    constexpr double AMinMin = 0.1;
    constexpr double AMinMax = 1.;
    validateConstraint(AMin, AMinMin, AMinMax, "AMin");

    const auto BMin = model.BMin;
    constexpr double BMinMin = 0.1;
    constexpr double BMinMax = 1.;
    validateConstraint(BMin, BMinMin, BMinMax, "BMin");

    const auto BMax = model.BMax;
    const double BMaxMin = BMin;
    constexpr double BMaxMax = 2.;
    validateConstraint(BMax, BMaxMin, BMaxMax, "BMax");

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    for(const auto& neighbor : neighbors) {
        if(agent.id == neighbor.id) {
            continue;
        }

        const auto contanctDist = AgentToAgentSpacing(agent, neighbor);
        const auto distance = (agent.pos - neighbor.pos).Norm();
        if(contanctDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "effective distance {}",
                agent.pos,
                neighbor.pos,
                distance,
                distance - contanctDist);
        }
    }

    const auto maxRadius = std::max(AMin, BMax) / 2.;
    const auto lineSegments = geometry.LineSegmentsInDistanceTo(maxRadius, agent.pos);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= {}",
            agent.pos,
            maxRadius);
    }
}

std::unique_ptr<OperationalModel> GeneralizedCentrifugalForceModel::Clone() const
{
    return std::make_unique<GeneralizedCentrifugalForceModel>(*this);
}

Point GeneralizedCentrifugalForceModel::ForceDriv(
    const GenericAgent& ped,
    Point target,
    double mass,
    double tau,
    double deltaT,
    GeneralizedCentrifugalForceModelUpdate& update) const
{
    Point F_driv;
    const auto pos = ped.pos;
    const auto dest = ped.destination;
    const auto dist = (dest - pos).Norm();
    const auto& model = std::get<GeneralizedCentrifugalForceModelData>(ped.model);
    if(dist > J_EPS_GOAL) {

        const Point e0 = mollify_e0(target, pos, deltaT, model.orientationDelay, model.e0);
        update.e0 = e0;
        F_driv = ((e0 * model.v0 - (ped.orientation * model.speed)) * mass) / tau;
    } else {
        const Point e0 = model.e0;
        F_driv = ((e0 * model.v0 - (ped.orientation * model.speed)) * mass) / tau;
    }
    return F_driv;
}

Point GeneralizedCentrifugalForceModel::ForceRepPed(
    const GenericAgent& ped1,
    const GenericAgent& ped2) const
{
    const auto& model1 = std::get<GeneralizedCentrifugalForceModelData>(ped1.model);
    const auto& model2 = std::get<GeneralizedCentrifugalForceModelData>(ped2.model);
    Point F_rep;
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = ped2.pos - ped1.pos;
    const Point vp1 = (ped1.orientation * model1.speed); // v Ped1
    const Point vp2 = (ped2.orientation * model2.speed); // v Ped2
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    double tmp, tmp2;
    double v_ij;
    double K_ij;
    double nom; // nominator of Frep
    double px; // hermite Interpolation value
    const auto dist_eff = AgentToAgentSpacing(ped1, ped2);
    const auto agent1_mass = model1.mass;

    //          smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //       ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    // If the pedestrian is outside the cutoff distance, the force is zero.
    if(dist_eff >= maxNeighborRepulsionForce) {
        F_rep = Point(0.0, 0.0);
        return F_rep;
    }

    const double mindist =
        0.5; // for performance reasons, it is assumed that this distance is about 50 cm
    const double dist_intpol_left =
        mindist + maxNeighborInteractionDistance; // lower cut-off for Frep (modCFM)
    const double dist_intpol_right =
        maxNeighborRepulsionForce -
        maxNeighborInteractionDistance; // upper cut-off for Frep (modCFM)
    const double smax = mindist - maxNeighborInteractionDistance; // max overlapping
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

    const auto v0_1 = model1.v0;
    nom = strengthNeighborRepulsion * v0_1 + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
    nom *= nom;

    K_ij = sqrt(K_ij);
    if(dist_eff <= smax) { // 5
        f = -agent1_mass * K_ij * nom / dist_intpol_left;
        F_rep = ep12 * maxNeighborInterpolationDistance * f;
        return F_rep;
    }

    //          smax    dist_intpol_left           dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //           5   |     4       |            3             |      2       | 1

    if(dist_eff >= dist_intpol_right) { // 2
        f = -agent1_mass * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
        f1 = -f / dist_intpol_right;
        px = hermite_interp(dist_eff, dist_intpol_right, maxNeighborRepulsionForce, f, 0, f1, 0);
        F_rep = ep12 * px;
    } else if(dist_eff >= dist_intpol_left) { // 3
        f = -agent1_mass * K_ij * nom / fabs(dist_eff); // abs(NR-Dv(i)+Sa)
        F_rep = ep12 * f;
    } else { // 4
        f = -agent1_mass * K_ij * nom / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, maxNeighborInterpolationDistance * f, f, 0, f1);
        F_rep = ep12 * px;
    }
    if(F_rep.x != F_rep.x || F_rep.y != F_rep.y) {
        LOG_ERROR(
            "NAN return p1{} p2 {} Frepx={:f} Frepy={:f} K_ij={:f}",
            ped1.id,
            ped2.id,
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
    const GenericAgent& ped,
    const CollisionGeometry& geometry) const
{
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    auto f = std::accumulate(
        walls.cbegin(),
        walls.cend(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + ForceRepWall(ped, element);
        });
    return f;
}

inline Point
GeneralizedCentrifugalForceModel::ForceRepWall(const GenericAgent& ped, const LineSegment& w) const
{
    Point F = Point(0.0, 0.0);
    Point pt = w.ShortestPoint(ped.pos);
    double wlen = w.LengthSquare();

    if(wlen < 0.01) { // ignore walls smaller than 10 cm
        return F;
    }
    // Kraft soll nur orthgonal wirken
    // ???
    if(fabs((w.p1 - w.p2).ScalarProduct(ped.pos - pt)) > J_EPS) {
        return F;
    }
    double mind = 0.5; // for performance reasons this distance is assumed to be constant
    const auto& model = std::get<GeneralizedCentrifugalForceModelData>(ped.model);
    double vn =
        w.NormalComp(ped.orientation * model.speed); // normal component of the velocity on the wall
    F = ForceRepStatPoint(ped, pt, mind, vn);

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
    const GenericAgent& ped,
    const Point& p,
    double l,
    double vn) const
{
    Point F_rep = Point(0.0, 0.0);
    // TODO(kkratz): this will fail for speed 0.
    // I think the code can be rewritten to account for orientation and speed separately
    const auto& model = std::get<GeneralizedCentrifugalForceModelData>(ped.model);
    const Point v = ped.orientation * model.speed;
    Point dist = p - ped.pos; // x- and y-coordinate of the distance between ped and p
    double d = dist.Norm(); // distance between the centre of ped and point p
    Point e_ij; // x- and y-coordinate of the normalized vector between ped and p

    double tmp;
    double bla;
    Point r;
    Point pinE; // vorher x1, y1
    const Ellipse E{model.Av, model.AMin, model.BMax, model.BMin};

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
    pinE = p.TransformToEllipseCoordinates(ped.pos, ped.orientation.x, ped.orientation.y);
    const auto v0 = model.v0;
    // Punkt auf der Ellipse
    r = E.PointOnEllipse(pinE, model.speed / v0, ped.pos, model.speed, ped.orientation);
    // interpolierte Kraft
    F_rep = ForceInterpolation(v0, K_ij, e_ij, vn, d, (r - ped.pos).Norm(), l);
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
    double smax = l - maxGeometryInteractionDistance; // max overlapping radius
    double dist_intpol_left = l + maxGeometryInteractionDistance; // r_eps
    double dist_intpol_right = maxGeometryRepulsionForce - maxGeometryInteractionDistance;

    double dist_eff = d - r;

    //         smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    double px = 0; // value of the interpolated function
    double tmp1 = maxGeometryRepulsionForce;
    double tmp2 = dist_intpol_right;
    double tmp3 = dist_intpol_left;
    double tmp5 = smax + r;

    if(dist_eff >= tmp1) { // 1
        // F_rep = Point(0.0, 0.0);
        return F_rep;
    }

    if(dist_eff <= tmp5) { // 5
        F_rep = e * (-maxGeometryInterpolationDistance);
        return F_rep;
    }

    if(dist_eff > tmp2) { // 2
        f = -nominator / dist_intpol_right;
        f1 = -f / dist_intpol_right; // nominator / (dist_intpol_right^2) = derivativ of f
        px = hermite_interp(dist_eff, dist_intpol_right, maxGeometryRepulsionForce, f, 0, f1, 0);
        F_rep = e * px;
    } else if(dist_eff >= tmp3) { // 3
        f = -nominator / fabs(dist_eff); // devided by abs f the effective distance
        F_rep = e * f;
    } else { // 4 d > smax FIXME
        f = -nominator / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, maxGeometryInterpolationDistance * f, f, 0, f1);
        F_rep = e * px;
    }
    return F_rep;
}
double GeneralizedCentrifugalForceModel::AgentToAgentSpacing(
    const GenericAgent& agent1,
    const GenericAgent& agent2) const
{
    const auto& model1 = std::get<GeneralizedCentrifugalForceModelData>(agent1.model);
    const auto& model2 = std::get<GeneralizedCentrifugalForceModelData>(agent2.model);
    const Ellipse E1{model1.Av, model1.AMin, model1.BMax, model1.BMin};
    const Ellipse E2{model2.Av, model2.AMin, model2.BMax, model2.BMin};
    const auto v0_1 = model1.v0;
    const auto v0_2 = model2.v0;
    return E1.EffectiveDistanceToEllipse(
        E2,
        agent1.pos,
        agent2.pos,
        model1.speed / v0_1,
        model2.speed / v0_2,
        model1.speed,
        model2.speed,
        agent1.orientation,
        agent2.orientation);
}
