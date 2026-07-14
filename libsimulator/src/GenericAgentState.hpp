// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

// Forward declaration only — UniqueID uses GenericAgent as a phantom tag, the
// complete type is not needed here.
struct GenericAgent;

#include "OperationalModels/CustomModel/FormatAny.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

#include <fmt/core.h>

#include <any>
#include <variant>

// ---------------------------------------------------------------------------
// Per-agent state structs — one per operational model.
// These are defined here (not nested in the model class) so that
// OperationalModel.hpp and GenericAgent.hpp can both include this file without
// creating a circular dependency with the model class headers.
//
// Each model class header does:  using Agent = <FlatName>;
// Callers that write  std::get<ModelClass::Agent>(state)  continue to work
// because the type alias resolves to the same underlying type.
// ---------------------------------------------------------------------------

struct GcfmState {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    Point orientation{1.0, 0.0};
    double speed{};
    Point e0{};
    int orientationDelay{};
    double mass{1.0};
    double tau{0.5};
    double v0{1.2};
    double Av{1.0};
    double AMin{0.2};
    double BMin{0.2};
    double BMax{0.4};
    double strengthNeighborRepulsion{0.3};
    double strengthGeometryRepulsion{0.2};
    double maxNeighborInteractionDistance{2};
    double maxGeometryInteractionDistance{2};
    double maxNeighborInterpolationDistance{0.1};
    double maxGeometryInterpolationDistance{0.1};
    double maxNeighborRepulsionForce{9};
    double maxGeometryRepulsionForce{3};
};

struct CfsmState {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    Point orientation{0.0, 0.0};
    double timeGap{1};
    double v0{1.2};
    double radius{0.2};
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double strengthGeometryRepulsion{5.0};
    double rangeGeometryRepulsion{0.02};
};

struct CfsmV2State {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    Point orientation{0.0, 0.0};
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double strengthGeometryRepulsion{5.0};
    double rangeGeometryRepulsion{0.02};
    double timeGap{1};
    double v0{1.2};
    double radius{0.2};
};

struct CfsmV3State {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    Point orientation{1.0, 0.0};
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double strengthGeometryRepulsion{5.0};
    double rangeGeometryRepulsion{0.02};
    double rangeXScale{20.0};
    double rangeYScale{8.0};
    double thetaMaxUpperBound{1.57};
    double agentBuffer{0.0};
    double timeGap{1};
    double v0{1.2};
    double radius{0.2};
    double headingAngle{0.0};
};

struct AvmState {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    Point orientation{0.0, 0.0};
    double strengthNeighborRepulsion{8.0};
    double rangeNeighborRepulsion{0.1};
    double wallBufferDistance{0.1};
    double anticipationTime{1.0};
    double reactionTime{0.3};
    Point velocity{};
    double timeGap{1.06};
    double v0{1.2};
    double radius{0.2};
    double pushoutStrength{0.3};
};

struct SfmState {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    Point velocity{};
    double mass{80.0};
    double desiredSpeed{0.8};
    double reactionTime{0.5};
    double agentScale{2000.0};
    double obstacleScale{2000.0};
    double forceDistance{0.08};
    double radius{0.3};
    double bodyForce{120000};
    double friction{240000};
};

struct WdmState {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};
    Point position{};
    Point orientation{0.0, 0.0};
    double radius{0.15};
    double v0{1.2};
    double stuckTime{0.0};
    double anchorX{0.0};
    double anchorY{0.0};
    double detourTime{0.0};
    int detourSide{1};
    double timeHorizon{2.0};
    double stepSize{0.5};
    double timeUncertainty{0.5};
    double velocityUncertaintyX{0.2};
    double velocityUncertaintyY{0.2};
    int numSamples{20};
};

class CustomModelState
{
public:
    /// State position cache, kept outside the type-erased payload so the framework can read
    /// it without touching the payload (for GilSafePyObject payloads: without acquiring the
    /// GIL). The owning model must keep it in sync with the payload's own position state.
    Point position{};
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

private:
    std::any value{};
    FormatFn format{};

public:
    template <typename T>
        requires(!std::is_same_v<std::decay_t<T>, CustomModelState>)
    CustomModelState(T&& value) : value(std::forward<T>(value)), format(makeFormatFn<T>())
    {
        using Stored = std::decay_t<T>;
        static_assert(
            std::is_copy_constructible_v<Stored>,
            "CustomModel::State payloads must be copy-constructible");
    }

    template <typename T>
    T& Get()
    {
        return std::any_cast<T&>(value);
    }

    template <typename T>
    const T& Get() const
    {
        return std::any_cast<const T&>(value);
    }

    template <typename T>
    void Set(T&& newValue)
    {
        using Stored = std::decay_t<T>;
        std::any_cast<Stored&>(value) = std::forward<T>(newValue);
    }

    friend struct fmt::formatter<CustomModelState>;
};

// The variant that represents the per-agent operational model state.
using GenericAgentModel = std::variant<
    GcfmState,
    CfsmState,
    CfsmV2State,
    CfsmV3State,
    AvmState,
    SfmState,
    WdmState,
    CustomModelState>;

// ---------------------------------------------------------------------------
// fmt formatters for each agent state type
// ---------------------------------------------------------------------------

template <>
struct fmt::formatter<GcfmState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const GcfmState& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "GCFM[orientation={}, speed={}])", m.orientation, m.speed);
    }
};

template <>
struct fmt::formatter<CfsmState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const CfsmState& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModel[orientation={}, timeGap={}, v0={}, radius={}])",
            m.orientation,
            m.timeGap,
            m.v0,
            m.radius);
    }
};

template <>
struct fmt::formatter<CfsmV2State> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const CfsmV2State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelV2[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "timeGap={}, v0={}, radius={}])",
            m.orientation,
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.strengthGeometryRepulsion,
            m.rangeGeometryRepulsion,
            m.timeGap,
            m.v0,
            m.radius);
    }
};

template <>
struct fmt::formatter<CfsmV3State> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const CfsmV3State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "CollisionFreeSpeedModelV3[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, strengthGeometryRepulsion={}, rangeGeometryRepulsion={}, "
            "rangeXScale={}, rangeYScale={}, thetaMaxUpperBound={}, agentBuffer={}, "
            "timeGap={}, v0={}, radius={}, headingAngle={}])",
            m.orientation,
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.strengthGeometryRepulsion,
            m.rangeGeometryRepulsion,
            m.rangeXScale,
            m.rangeYScale,
            m.thetaMaxUpperBound,
            m.agentBuffer,
            m.timeGap,
            m.v0,
            m.radius,
            m.headingAngle);
    }
};

template <>
struct fmt::formatter<AvmState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const AvmState& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AnticipationVelocityModel[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, wallBufferDistance={}, "
            "timeGap={}, v0={}, radius={}, reactionTime={}, anticipationTime={}, velocity={}])",
            m.orientation,
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.wallBufferDistance,
            m.timeGap,
            m.v0,
            m.radius,
            m.reactionTime,
            m.anticipationTime,
            m.velocity);
    }
};

template <>
struct fmt::formatter<SfmState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const SfmState& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "SFM[velocity={}, m={}, v0={}, tau={}, A_ped={}, A_obst={}, B={}, r={}])",
            m.velocity,
            m.mass,
            m.desiredSpeed,
            m.reactionTime,
            m.agentScale,
            m.obstacleScale,
            m.forceDistance,
            m.radius);
    }
};

template <>
struct fmt::formatter<CustomModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(const CustomModelState& value, fmt::format_context& ctx) const
    {
        return value.format(value.value, ctx);
    }
};

template <>
struct fmt::formatter<WdmState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const WdmState& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "WarpDriver[orientation={}, radius={}, v0={}]",
            m.orientation,
            m.radius,
            m.v0);
    }
};
