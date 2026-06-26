// SPDX-License-Identifier: LGPL-3.0-or-later
#include "WarpDriverModelBuilder.hpp"

#include "SimulationError.hpp"
#include "WarpDriverModel.hpp"

namespace
{
// Internal sampling and RNG defaults. Not exposed through the public
// API: the trajectory sample count trades cost for accuracy and the
// seed is fixed for reproducibility of the symmetry-breaking
// perturbation.
constexpr int kDefaultNumSamples = 20;
constexpr uint64_t kDefaultRngSeed = 42;
} // namespace

WarpDriverModelBuilder::WarpDriverModelBuilder(
    double timeHorizon,
    double stepSize,
    double sigma,
    double timeUncertainty,
    double velocityUncertaintyX,
    double velocityUncertaintyY)
    : _timeHorizon(timeHorizon)
    , _stepSize(stepSize)
    , _sigma(sigma)
    , _timeUncertainty(timeUncertainty)
    , _velocityUncertaintyX(velocityUncertaintyX)
    , _velocityUncertaintyY(velocityUncertaintyY)
{
}

std::unique_ptr<OperationalModel> WarpDriverModelBuilder::Build()
{
    if(_timeHorizon <= 0.0) {
        throw SimulationError(
            "WarpDriverModelBuilder: timeHorizon must be > 0, got {}", _timeHorizon);
    }
    if(_stepSize <= 0.0) {
        throw SimulationError("WarpDriverModelBuilder: stepSize must be > 0, got {}", _stepSize);
    }
    if(_sigma <= 0.0) {
        throw SimulationError("WarpDriverModelBuilder: sigma must be > 0, got {}", _sigma);
    }
    if(_timeUncertainty < 0.0) {
        throw SimulationError(
            "WarpDriverModelBuilder: timeUncertainty must be >= 0, got {}", _timeUncertainty);
    }
    if(_velocityUncertaintyX < 0.0) {
        throw SimulationError(
            "WarpDriverModelBuilder: velocityUncertaintyX must be >= 0, got {}",
            _velocityUncertaintyX);
    }
    if(_velocityUncertaintyY < 0.0) {
        throw SimulationError(
            "WarpDriverModelBuilder: velocityUncertaintyY must be >= 0, got {}",
            _velocityUncertaintyY);
    }

    return std::make_unique<WarpDriverModel>(
        _timeHorizon,
        _stepSize,
        _sigma,
        _timeUncertainty,
        _velocityUncertaintyX,
        _velocityUncertaintyY,
        kDefaultNumSamples,
        kDefaultRngSeed);
}
