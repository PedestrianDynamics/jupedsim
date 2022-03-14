#pragma once

#include "Enum.hpp"

#include <fmt/format.h>
enum class DoorState { OPEN, CLOSE, TEMP_CLOSE };

template <>
DoorState from_string(const std::string &);

namespace fmt
{
template <>
struct formatter<DoorState> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext & ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(DoorState state, FormatContext & ctx)
    {
        switch(state) {
            case DoorState::OPEN:
                return format_to(ctx.out(), "open");
            case DoorState::CLOSE:
                return format_to(ctx.out(), "close");
            case DoorState::TEMP_CLOSE:
                return format_to(ctx.out(), "temp_close");
        }
    }
};
} // namespace fmt
