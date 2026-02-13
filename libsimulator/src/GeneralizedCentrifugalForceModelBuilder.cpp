// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralizedCentrifugalForceModelBuilder.hpp"

#include "GeneralizedCentrifugalForceModel.hpp"

#include <stdexcept>

GeneralizedCentrifugalForceModelBuilder::GeneralizedCentrifugalForceModelBuilder(
    double nuped,
    double nuwall,
    double dist_effPed,
    double dist_effWall,
    double intp_widthped,
    double intp_widthwall,
    double maxfped,
    double maxfwall)
    : _nuped(nuped)
    , _nuwall(nuwall)
    , _dist_effPed(dist_effPed)
    , _dist_effWall(dist_effWall)
    , _intp_widthped(intp_widthped)
    , _intp_widthwall(intp_widthwall)
    , _maxfped(maxfped)
    , _maxfwall(maxfwall)
{
}

GeneralizedCentrifugalForceModel GeneralizedCentrifugalForceModelBuilder::Build()
{
    if(_dist_effPed <= _intp_widthped) {
        throw std::invalid_argument(
            "max_neighbor_interaction_distance must be greater "
            "than max_neighbor_interpolation_distance");
    }
    if(_dist_effWall <= _intp_widthwall) {
        throw std::invalid_argument(
            "max_geometry_interaction_distance must be greater "
            "than max_geometry_interpolation_distance");
    }
    return GeneralizedCentrifugalForceModel(
        _nuped,
        _nuwall,
        _dist_effPed,
        _dist_effWall,
        _intp_widthped,
        _intp_widthwall,
        _maxfped,
        _maxfwall);
}
