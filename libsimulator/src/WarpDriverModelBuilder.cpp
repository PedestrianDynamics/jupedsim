// SPDX-License-Identifier: LGPL-3.0-or-later
#include "WarpDriverModelBuilder.hpp"

#include "SimulationError.hpp"

WarpDriverModelBuilder::WarpDriverModelBuilder(
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
{
}

WarpDriverModel WarpDriverModelBuilder::Build()
{
    if(_timeHorizon <= 0.0) {
        throw SimulationError("WarpDriverModelBuilder: timeHorizon must be > 0, got {}", _timeHorizon);
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
    if(_velocityUncertainty < 0.0) {
        throw SimulationError(
            "WarpDriverModelBuilder: velocityUncertainty must be >= 0, got {}",
            _velocityUncertainty);
    }
    if(_numSamples <= 0) {
        throw SimulationError(
            "WarpDriverModelBuilder: numSamples must be > 0, got {}", _numSamples);
    }
    if(_jamSpeedThreshold < 0.0) {
        throw SimulationError(
            "WarpDriverModelBuilder: jamSpeedThreshold must be >= 0, got {}", _jamSpeedThreshold);
    }
    if(_jamStepCount <= 0) {
        throw SimulationError(
            "WarpDriverModelBuilder: jamStepCount must be > 0, got {}", _jamStepCount);
    }

    return WarpDriverModel(
        _timeHorizon,
        _stepSize,
        _sigma,
        _timeUncertainty,
        _velocityUncertainty,
        _numSamples,
        _jamSpeedThreshold,
        _jamStepCount);
}
