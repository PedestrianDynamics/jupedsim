// SPDX-License-Identifier: LGPL-3.0-or-later
#include "WarpDriverModelBuilder.hpp"

#include "SimulationError.hpp"

WarpDriverModelBuilder::WarpDriverModelBuilder(
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
    , _sigma(sigma)
    , _timeUncertainty(timeUncertainty)
    , _velocityUncertaintyX(velocityUncertaintyX)
    , _velocityUncertaintyY(velocityUncertaintyY)
    , _numSamples(numSamples)
    , _rngSeed(rngSeed)
{
}

WarpDriverModel WarpDriverModelBuilder::Build()
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
    if(_numSamples <= 0) {
        throw SimulationError(
            "WarpDriverModelBuilder: numSamples must be > 0, got {}", _numSamples);
    }

    return WarpDriverModel(
        _timeHorizon,
        _stepSize,
        _sigma,
        _timeUncertainty,
        _velocityUncertaintyX,
        _velocityUncertaintyY,
        _numSamples,
        _rngSeed);
}
