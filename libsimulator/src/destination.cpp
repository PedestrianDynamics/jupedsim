// SPDX-License-Identifier: LGPL-3.0-or-later
#include "destination.hpp"

#include "floorfield_routing_engine.hpp"

bool Destination::contains(const Location& where) const
{
    return _engine->contains(where, _id);
}

Point Destination::orientation(const Location& from) const
{
    return _engine->get_orientation(from, _id);
}

TravelTimeGrid Destination::travel_times(std::size_t region) const
{
    return _engine->travel_times(region, _id);
}

SpeedGrid Destination::speed(std::size_t region) const
{
    return _engine->speed(region, _id);
}

bool Destination::has_speed_override(std::size_t region) const
{
    return _engine->has_speed_override(region, _id);
}

void Destination::set_speed_override(std::size_t region, const std::vector<double>& values) const
{
    _engine->set_speed_override(region, _id, values);
}

void Destination::clear_speed_override(std::size_t region) const
{
    _engine->clear_speed_override(region, _id);
}
