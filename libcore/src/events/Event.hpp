#pragma once


#include "Enum.hpp"
#include "geometry/Point.hpp"

#include <chrono>
#include <fmt/format.h>
#include <memory>
#include <variant>
#include <vector>

class Pedestrian;

class CreatePedestrianEvent
{
public:
    std::chrono::nanoseconds execute_at;
    Point _position;

    // general values from StartDistribution::GenerateAgent
    int _final_destination;
    int _group_id;
    int _router_id;
    double _premovement_time;

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

    CreatePedestrianEvent(Pedestrian const * agent, std::chrono::nanoseconds min_time);
};

struct DoorEvent {
    enum class Type { OPEN, CLOSE, TEMP_CLOSE, RESET };
    std::chrono::nanoseconds execute_at;
    int doorId;
    Type type;
};

template <>
DoorEvent::Type from_string<DoorEvent::Type>(const std::string & str);

using Event = std::variant<CreatePedestrianEvent, DoorEvent>;

std::vector<Event> CreateEventsFromAgents(
    std::vector<std::unique_ptr<Pedestrian>> const & agents,
    std::chrono::nanoseconds t);
