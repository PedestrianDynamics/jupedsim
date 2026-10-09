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

namespace
{
/// How far to ask for walls. The force reaches max_geometry_interaction_distance beyond the
/// agent's ellipse, 3.4 m with the default parameters. Use the 4m default of previous impl.
constexpr double wall_search_radius = 4.0;
} // namespace

GeneralizedCentrifugalForceModel::GeneralizedCentrifugalForceModel(
    double strength_neighbor_repulsion,
    double strength_geometry_repulsion,
    double max_neighbor_interaction_distance,
    double max_geometry_interaction_distance,
    double max_neighbor_interpolation_distance,
    double max_geometry_interpolation_distance,
    double max_neighbor_repulsion_force,
    double max_geometry_repulsion_force)
    : _strength_neighbor_repulsion(strength_neighbor_repulsion)
    , _strength_geometry_repulsion(strength_geometry_repulsion)
    , _max_neighbor_interaction_distance(max_neighbor_interaction_distance)
    , _max_geometry_interaction_distance(max_geometry_interaction_distance)
    , _max_neighbor_interpolation_distance(max_neighbor_interpolation_distance)
    , _max_geometry_interpolation_distance(max_geometry_interpolation_distance)
    , _max_neighbor_repulsion_force(max_neighbor_repulsion_force)
    , _max_geometry_repulsion_force(max_geometry_repulsion_force)
{
}

OperationalModelType GeneralizedCentrifugalForceModel::type() const
{
    return OperationalModelType::GeneralizedCentrifugalForce;
}

Point GeneralizedCentrifugalForceModel::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    const auto& current_state = std::get<State>(current);
    const auto neighborhood = step.other_agents_in_range(
        _cut_off_radius, [&step](const NeighborView& n) { return step.no_geometry_between(n); });
    Point f_rep;
    for(const auto& neighbor : neighborhood) {
        f_rep += force_rep_ped(current_state, neighbor);
    }

    // force_driv leaves e0 untouched when the agent has practically arrived; the default
    // constructed value is what gets stored then.
    Point e0{};
    // repulsive forces to the walls and transitions that are not my target
    Point repwall{};
    for(const auto& wall : step.walls_in_range(wall_search_radius)) {
        repwall += force_rep_wall(current_state, wall);
    }

    const Point fd = force_driv(
        current_state,
        step.route_orientation(),
        current_state.mass,
        current_state.tau,
        step.dt(),
        e0);
    const Point acc = (fd + f_rep + repwall) / current_state.mass;

    const Point velocity = (current_state.orientation * current_state.speed) + acc * step.dt();

    auto& next_model = std::get<State>(next);
    next_model.e0 = e0;
    ++next_model.orientation_delay;
    next_model.orientation = velocity.normalized();
    next_model.speed = velocity.norm();
    return velocity * step.dt();
}

void GeneralizedCentrifugalForceModel::check_model_constraint(
    const GenericAgent& agent,
    const AgentView& view) const
{
    const auto& current_state = std::get<State>(agent.state);

    if(!current_state.orientation.is_unit_length()) {
        throw SimulationError(
            "Orientation is invalid: {}. Length should be 1.", current_state.orientation);
    }

    const auto mass = current_state.mass;
    constexpr double mass_min = 1.;
    constexpr double mass_max = 100.;
    validate_constraint(mass, mass_min, mass_max, "mass");

    const auto tau = current_state.tau;
    constexpr double tau_min = 0.1;
    constexpr double tau_max = 10.;
    validate_constraint(tau, tau_min, tau_max, "tau");

    const auto v0 = current_state.v0;
    constexpr double v0_min = 0.;
    constexpr double v0_max = 10.;
    validate_constraint(v0, v0_min, v0_max, "v0");

    const auto av = current_state.av;
    constexpr double av_min = 0.;
    constexpr double av_max = 10.;
    validate_constraint(av, av_min, av_max, "Av");

    const auto a_min = current_state.a_min;
    constexpr double a_min_min = 0.1;
    constexpr double a_min_max = 1.;
    validate_constraint(a_min, a_min_min, a_min_max, "AMin");

    const auto b_min = current_state.b_min;
    constexpr double b_min_min = 0.1;
    constexpr double b_min_max = 1.;
    validate_constraint(b_min, b_min_min, b_min_max, "BMin");

    const auto b_max = current_state.b_max;
    const double b_max_min = b_min;
    constexpr double b_max_max = 2.;
    validate_constraint(b_max, b_max_min, b_max_max, "BMax");

    const auto neighbors = view.other_agents_in_range(2.0);
    for(const auto& neighbor : neighbors) {
        const auto contanct_dist = agent_to_agent_spacing(current_state, neighbor);
        const auto distance = neighbor.relative_position.norm();
        if(contanct_dist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent at {} too close to agent at {}: distance {}, "
                "contactDist {}, "
                "effective distance {}",
                agent.location.xy(),
                agent.location.xy() + neighbor.relative_position,
                distance,
                contanct_dist,
                distance - contanct_dist);
        }
    }

    const auto max_radius = std::max(a_min, b_max) / 2.;
    if(!view.walls_in_range(max_radius).empty()) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance < {}",
            agent.location.xy(),
            max_radius);
    }
}

Point GeneralizedCentrifugalForceModel::force_driv(
    const State& current_state,
    Point orientation_to_target,
    double mass,
    double tau,
    double delta_t,
    Point& e0update) const
{
    Point f_driv;
    if(orientation_to_target != Point{}) {
        // expect this to never trigger as stage system should cover it.
        const Point e0 = mollify_e0(
            orientation_to_target, delta_t, current_state.orientation_delay, current_state.e0);
        e0update = e0;
        f_driv =
            ((e0 * current_state.v0 - (current_state.orientation * current_state.speed)) * mass) /
            tau;
    } else {
        const Point e0 = current_state.e0;
        f_driv =
            ((e0 * current_state.v0 - (current_state.orientation * current_state.speed)) * mass) /
            tau;
    }
    return f_driv;
}

Point GeneralizedCentrifugalForceModel::force_rep_ped(
    const State& current_state,
    const NeighborView& neighbor) const
{
    const auto& neighbor_state = std::get<State>(*neighbor.state);
    Point f_rep;
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = neighbor.relative_position;
    const Point vp1 = (current_state.orientation * current_state.speed); // v Ped1
    const Point vp2 = (neighbor_state.orientation * neighbor_state.speed); // v Ped2
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    double tmp, tmp2;
    double v_ij;
    double k_ij;
    double nom; // nominator of Frep
    double px; // hermite Interpolation value
    const auto dist_eff = agent_to_agent_spacing(current_state, neighbor);
    const auto agent1_mass = current_state.mass;

    //          smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //       ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    // If the pedestrian is outside the cutoff distance, the force is zero.
    if(dist_eff >= _max_neighbor_interaction_distance) {
        f_rep = Point(0.0, 0.0);
        return f_rep;
    }

    const double mindist =
        0.5; // for performance reasons, it is assumed that this distance is about 50 cm
    const double dist_intpol_left =
        mindist + _max_neighbor_interpolation_distance; // lower cut-off for Frep (modCFM)
    const double dist_intpol_right =
        _max_neighbor_interaction_distance -
        _max_neighbor_interpolation_distance; // upper cut-off for Frep (modCFM)
    const double smax = mindist - _max_neighbor_interpolation_distance; // max overlapping
    double f = 0.0f; // fuction value
    double f1 = 0.0f; // derivative of function value

    // todo: runtime normsquare?
    if(distp12.norm() >= j_eps) {
        ep12 = distp12.normalized();

    } else {
        LOG_WARNING(
            "Distance between two pedestrians is small ({}<{}). Force can not be calculated.",
            distp12.norm(),
            j_eps);
        return f_rep; // Parameter values are not chosen wisely --> unrealistic overlaping ...
                      // ignore.
    }
    // calculate the parameter (whatever dist is)
    tmp = (vp1 - vp2).scalar_product(ep12); // < v_ij , e_ij >
    v_ij = 0.5 * (tmp + fabs(tmp));
    tmp2 = vp1.scalar_product(ep12); // < v_i , e_ij >

    // todo: runtime normsquare?
    if(vp1.norm() < j_eps) { // if(norm(v_i)==0)
        k_ij = 0;
    } else {
        double bla = tmp2 + fabs(tmp2);
        k_ij = 0.25 * bla * bla / vp1.scalar_product(vp1); // squared

        if(k_ij < j_eps * j_eps) {
            f_rep = Point(0.0, 0.0);
            return f_rep;
        }
    }

    const auto v0_1 = current_state.v0;
    nom = _strength_neighbor_repulsion * v0_1 + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
    nom *= nom;

    k_ij = sqrt(k_ij);
    if(dist_eff <= smax) { // 5
        f = -agent1_mass * k_ij * nom / dist_intpol_left;
        f_rep = ep12 * _max_neighbor_repulsion_force * f;
        return f_rep;
    }

    //          smax    dist_intpol_left           dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //           5   |     4       |            3             |      2       | 1
    if(dist_eff >= dist_intpol_right) { // 2
        f = -agent1_mass * k_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
        f1 = -f / dist_intpol_right;
        px = hermite_interp(
            dist_eff, dist_intpol_right, _max_neighbor_interaction_distance, f, 0, f1, 0);
        f_rep = ep12 * px;
    } else if(dist_eff >= dist_intpol_left) { // 3
        f = -agent1_mass * k_ij * nom / fabs(dist_eff); // abs(NR-Dv(i)+Sa)
        f_rep = ep12 * f;
    } else { // 4
        f = -agent1_mass * k_ij * nom / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, _max_neighbor_repulsion_force * f, f, 0, f1);
        f_rep = ep12 * px;
    }
    if(f_rep.x != f_rep.x || f_rep.y != f_rep.y) {
        LOG_ERROR(
            "NAN return distp12 {} Frepx={:f} Frepy={:f} K_ij={:f}",
            distp12,
            f_rep.x,
            f_rep.y,
            k_ij);
    }
    return f_rep;
}

inline Point GeneralizedCentrifugalForceModel::force_rep_wall(
    const State& current_state,
    const WallView& wall) const
{
    Point f = Point(0.0, 0.0);
    const auto& w = wall.segment;

    if(w.length_square() < 0.01) { // ignore walls smaller than 10 cm
        return f;
    }
    // Kraft soll nur orthgonal wirken
    // ???
    if(fabs((w.p1 - w.p2).scalar_product(Point{} - wall.closest_point)) > j_eps) {
        return f;
    }
    double mind = 0.5; // for performance reasons this distance is assumed to be constant
    double vn = w.normal_comp(
        current_state.orientation *
        current_state.speed); // normal component of the velocity on the wall
    f = force_rep_stat_point(current_state, wall.closest_point, mind, vn);

    return f; // line --> l != 0
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
Point GeneralizedCentrifugalForceModel::force_rep_stat_point(
    const State& current_state,
    const Point& p,
    double l,
    double vn) const
{
    Point f_rep = Point(0.0, 0.0);
    // TODO(kkratz): this will fail for speed 0.
    // I think the code can be rewritten to account for orientation and speed separately
    const Point v = current_state.orientation * current_state.speed;
    Point dist = p; // p is relative to the agent, so it already is the distance vector
    double d = dist.norm(); // distance between the centre of ped and point p
    Point e_ij; // x- and y-coordinate of the normalized vector between ped and p

    double tmp;
    double bla;
    Point r;
    Point pin_e; // vorher x1, y1
    const Ellipse e{
        current_state.av, current_state.a_min, current_state.b_max, current_state.b_min};

    if(d < j_eps)
        return Point(0.0, 0.0);
    e_ij = dist / d;
    tmp = v.scalar_product(e_ij); // < v_i , e_ij >;
    bla = (tmp + fabs(tmp));
    if(!bla) // Fussgaenger nicht im Sichtfeld
        return Point(0.0, 0.0);
    if(fabs(v.x) < j_eps && fabs(v.y) < j_eps) // v==0)
        return Point(0.0, 0.0);
    double k_ij;
    k_ij = 0.5 * bla / v.norm(); // K_ij
    // Punkt auf der Ellipse
    pin_e = p.transform_to_ellipse_coordinates(
        Point{}, current_state.orientation.x, current_state.orientation.y);
    const auto v0 = current_state.v0;
    // Punkt auf der Ellipse
    r = e.point_on_ellipse(
        pin_e, current_state.speed / v0, Point{}, current_state.speed, current_state.orientation);
    // interpolierte Kraft
    f_rep = force_interpolation(v0, k_ij, e_ij, vn, d, r.norm(), l);
    return f_rep;
}

Point GeneralizedCentrifugalForceModel::force_interpolation(
    double v0,
    double k_ij,
    const Point& e,
    double vn,
    double d,
    double r,
    double l) const
{
    Point f_rep;
    double nominator = _strength_geometry_repulsion * v0 + vn;
    nominator *= nominator * k_ij;
    double f = 0, f1 = 0; // function value and its derivative at the interpolation point
    double smax = l - _max_geometry_interpolation_distance; // max overlapping radius
    double dist_intpol_left = l + _max_geometry_interpolation_distance; // r_eps
    double dist_intpol_right =
        _max_geometry_interaction_distance - _max_geometry_interpolation_distance;

    double dist_eff = d - r;

    //         smax    dist_intpol_left      dist_intpol_right       dist_eff_max
    //           ----|-------------|--------------------------|--------------|----
    //       5   |     4       |            3             |      2       | 1

    double px = 0; // value of the interpolated function
    double tmp1 = _max_geometry_interaction_distance;
    double tmp2 = dist_intpol_right;
    double tmp3 = dist_intpol_left;
    double tmp5 = smax + r;

    if(dist_eff >= tmp1) { // 1
        // f_rep = Point(0.0, 0.0);
        return f_rep;
    }

    if(dist_eff <= tmp5) { // 5
        f_rep = e * (-_max_geometry_repulsion_force);
        return f_rep;
    }

    if(dist_eff > tmp2) { // 2
        f = -nominator / dist_intpol_right;
        f1 = -f / dist_intpol_right; // nominator / (dist_intpol_right^2) = derivativ of f
        px = hermite_interp(
            dist_eff, dist_intpol_right, _max_geometry_interaction_distance, f, 0, f1, 0);
        f_rep = e * px;
    } else if(dist_eff >= tmp3) { // 3
        f = -nominator / fabs(dist_eff); // devided by abs f the effective distance
        f_rep = e * f;
    } else { // 4 d > smax FIXME
        f = -nominator / dist_intpol_left;
        f1 = -f / dist_intpol_left;
        px = hermite_interp(
            dist_eff, smax, dist_intpol_left, _max_geometry_repulsion_force * f, f, 0, f1);
        f_rep = e * px;
    }
    return f_rep;
}
double GeneralizedCentrifugalForceModel::agent_to_agent_spacing(
    const State& current_state,
    const NeighborView& neighbor) const
{
    const auto& neighbor_state = std::get<State>(*neighbor.state);
    const Ellipse e1{
        current_state.av, current_state.a_min, current_state.b_max, current_state.b_min};
    const Ellipse e2{
        neighbor_state.av, neighbor_state.a_min, neighbor_state.b_max, neighbor_state.b_min};
    const auto v0_1 = current_state.v0;
    const auto v0_2 = neighbor_state.v0;
    // Avoid division by zero by setting scale to 1 when v0 is 0
    const double scale1 = (v0_1 == 0.0) ? 1.0 : current_state.speed / v0_1;
    const double scale2 = (v0_2 == 0.0) ? 1.0 : neighbor_state.speed / v0_2;

    // The ellipse distance is translation invariant, so we evaluate it in the frame of the
    // agent that asked, which sits at the origin.
    return e1.effective_distance_to_ellipse(
        e2,
        Point{0.0, 0.0},
        neighbor.relative_position,
        scale1,
        scale2,
        current_state.speed,
        neighbor_state.speed,
        current_state.orientation,
        neighbor_state.orientation);
}
