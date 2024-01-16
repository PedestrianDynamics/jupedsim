#pragma once

#include "Point.hpp" // is currently included to find fmt
struct SocialForceModelData {
    double test_value;
};

template <>
struct fmt::formatter<SocialForceModelData> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const SocialForceModelData& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "SFM[test_val={}])", m.test_value);
    }
};