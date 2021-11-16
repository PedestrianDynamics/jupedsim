#pragma once


#include "geometry/Point.h"

#include <chrono>
#include <memory>
#include <variant>
#include <vector>

class Simulation;
class Pedestrian;

class BaseEvent
{
protected:
    /// (min) time to trigger event
    std::chrono::nanoseconds _min_time;

public:
    explicit BaseEvent(std::chrono::nanoseconds min_time);
    std::chrono::nanoseconds MinTime() const { return _min_time; }
};


class CreatePedestrianEvent : public BaseEvent
{
public:
    Point Position;

    // general values from StartDistribution::GenerateAgent
    int FinalDestination;
    int GroupId;
    int RouterId;
    double PatienceTime;
    double PremovementTime;
    double RiskTolerance;

    // Ellipse specific
    double EllipsisA_V;
    double EllipsisAMin;
    double EllipsisBMax;
    double EllipsisBMin;
    bool EllipsisDoStretch;

    double Tau;
    double T;

    double V0;
    double V0UpStairs;
    double V0DownStairs;
    double V0EscalatorUp;
    double V0EscalatorDown;

    double SmoothFactorUpStairs;
    double SmoothFactorDownStairs;
    double SmoothFactorEscalatorUp;
    double SmoothFactorEscalatorDown;

    CreatePedestrianEvent(Pedestrian const * agent, std::chrono::nanoseconds min_time);
};

/// This is just a Dummy to avoid special solutions for a single type variant
class DummyEvent : public BaseEvent
{
};


using Event = std::variant<CreatePedestrianEvent, DummyEvent>;


std::vector<Event> CreateEventsFromAgents(
    std::vector<std::unique_ptr<Pedestrian>> const & agents,
    std::chrono::nanoseconds t);
