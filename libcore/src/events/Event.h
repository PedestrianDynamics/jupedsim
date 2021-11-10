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
    std::chrono::nanoseconds MinTime() const { return _min_time; }
};


class CreatePedestrianEvent : public BaseEvent
{
    Point _position;

    // general values from StartDistribution::GenerateAgent
    int _final_destination;
    int _group_id;
    int _router_id;
    int _room_id;
    int _subroom_id;
    int _subroom_uid;
    double _patience_time;
    double _premovement_time;
    double _risk_tolerance;

    // Ellipse specific
    double _ellipse_a_v;
    double _ellipse_a_min;
    double _ellipse_b_max;
    double _ellipse_b_min;
    bool _ellipse_do_stretch;

    double _tau;
    double _T;

    double _v0;
    double _v0_up_stairs;
    double _v0_down_stairs;
    double _v0_escalator_up;
    double _v0_escalator_down;

    double _smooth_factor_up_stairs;
    double _smooth_factor_down_stairs;
    double _smooth_factor_escalator_up;
    double _smooth_factor_escalator_down;

public:
    CreatePedestrianEvent(Pedestrian const * agent, std::chrono::nanoseconds min_time);
};

/// This is just a Dummy to avoid special solutions for a single type variant
class DummyEvent : public BaseEvent
{
};


using Event = std::variant<CreatePedestrianEvent, DummyEvent>;


std::vector<Event> CreateEventsFromAgents(
    std::vector<std::unique_ptr<Pedestrian>> const & agents,
    Simulation const & sim);

Event CreateEventFromAgent(Pedestrian const * agent);
