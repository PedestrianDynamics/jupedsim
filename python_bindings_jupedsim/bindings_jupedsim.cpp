// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/jupedsim.h"
#include <Unreachable.hpp>

#include <algorithm>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// TODO(kkratz):
//  * write python doc
//  * write python test that exercises all bindings
//  * double check python object ownership

namespace py = pybind11;

#define OWNED_WRAPPER(cls)                                                                         \
    struct cls##_Wrapper {                                                                         \
        cls handle;                                                                                \
        cls##_Wrapper(cls h) : handle(h)                                                           \
        {                                                                                          \
        }                                                                                          \
        ~cls##_Wrapper()                                                                           \
        {                                                                                          \
            cls##_Free(handle);                                                                    \
        }                                                                                          \
        cls##_Wrapper(const cls##_Wrapper&) = delete;                                              \
        cls##_Wrapper& operator=(const cls##_Wrapper&) = delete;                                   \
        cls##_Wrapper(cls##_Wrapper&&) = delete;                                                   \
        cls##_Wrapper& operator=(cls##_Wrapper&&) = delete;                                        \
    }

#define WRAPPER(cls)                                                                               \
    struct cls##_Wrapper {                                                                         \
        cls handle;                                                                                \
        cls##_Wrapper(cls h) : handle(h)                                                           \
        {                                                                                          \
        }                                                                                          \
        ~cls##_Wrapper() = default;                                                                \
        cls##_Wrapper(const cls##_Wrapper&) = delete;                                              \
        cls##_Wrapper& operator=(const cls##_Wrapper&) = delete;                                   \
        cls##_Wrapper(cls##_Wrapper&&) = delete;                                                   \
        cls##_Wrapper& operator=(cls##_Wrapper&&) = delete;                                        \
    }

// Public types
OWNED_WRAPPER(JPS_Geometry);
OWNED_WRAPPER(JPS_GeometryBuilder);
OWNED_WRAPPER(JPS_OperationalModel);
OWNED_WRAPPER(JPS_CollisionFreeSpeedModelBuilder);
OWNED_WRAPPER(JPS_CollisionFreeSpeedModelV2Builder);
OWNED_WRAPPER(JPS_GeneralizedCentrifugalForceModelBuilder);
OWNED_WRAPPER(JPS_JourneyDescription);
OWNED_WRAPPER(JPS_Transition);
OWNED_WRAPPER(JPS_Simulation);
OWNED_WRAPPER(JPS_AgentIterator);
OWNED_WRAPPER(JPS_AgentIdIterator);
OWNED_WRAPPER(JPS_RoutingEngine);
OWNED_WRAPPER(JPS_NotifiableQueueProxy);
OWNED_WRAPPER(JPS_WaitingSetProxy);
OWNED_WRAPPER(JPS_WaypointProxy);
OWNED_WRAPPER(JPS_ExitProxy);
WRAPPER(JPS_Agent);
WRAPPER(JPS_GeneralizedCentrifugalForceModelState);
WRAPPER(JPS_CollisionFreeSpeedModelState);
WRAPPER(JPS_CollisionFreeSpeedModelV2State);

class LogCallbackOwner
{
public:
    using LogCallback = std::function<void(const std::string&)>;

    LogCallback debug{};
    LogCallback info{};
    LogCallback warning{};
    LogCallback error{};

public:
    static LogCallbackOwner& Instance()
    {
        static LogCallbackOwner instance;
        return instance;
    }
};

static std::tuple<double, double> intoTuple(const JPS_Point& p)
{
    return std::make_tuple(p.x, p.y);
}

static std::vector<std::tuple<double, double>> intoTuple(const std::vector<JPS_Point>& p)
{
    std::vector<std::tuple<double, double>> res;
    res.reserve(p.size());
    std::transform(
        std::begin(p), std::end(p), std::back_inserter(res), [](auto&& x) { return intoTuple(x); });
    return res;
}

static std::vector<std::tuple<double, double>> intoTuple(const JPS_Point* beg, const JPS_Point* end)
{
    std::vector<std::tuple<double, double>> res;
    res.reserve(end - beg);
    std::transform(beg, end, std::back_inserter(res), [](auto&& x) { return intoTuple(x); });
    return res;
}

static JPS_Point intoJPS_Point(const std::tuple<double, double> p)
{
    return JPS_Point{std::get<0>(p), std::get<1>(p)};
};

static std::vector<JPS_Point> intoJPS_Point(const std::vector<std::tuple<double, double>>& p)
{
    std::vector<JPS_Point> res;
    res.reserve(p.size());
    std::transform(std::begin(p), std::end(p), std::back_inserter(res), [](auto&& x) {
        return intoJPS_Point(x);
    });
    return res;
}

PYBIND11_MODULE(py_jupedsim, m)
{
    auto atexit = py::module_::import("atexit");
    atexit.attr("register")(py::cpp_function([]() {
        auto& owner = LogCallbackOwner::Instance();
        owner.debug = {};
        owner.info = {};
        owner.warning = {};
        owner.error = {};
    }));
    m.doc() = "JuPedSim Python bindings";
    m.def("set_debug_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().debug = callback;
        JPS_Logging_SetDebugCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().debug(msg); }, nullptr);
    });
    m.def("set_info_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().info = callback;
        JPS_Logging_SetInfoCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().info(msg); }, nullptr);
    });
    m.def("set_warning_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().warning = callback;
        JPS_Logging_SetWarningCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().warning(msg); }, nullptr);
    });
    m.def("set_error_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().error = callback;
        JPS_Logging_SetErrorCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().error(msg); }, nullptr);
    });
    py::class_<JPS_BuildInfo>(m, "BuildInfo")
        .def_readonly("git_commit_hash", &JPS_BuildInfo::git_commit_hash)
        .def_readonly("git_commit_date", &JPS_BuildInfo::git_commit_date)
        .def_readonly("git_branch", &JPS_BuildInfo::git_branch)
        .def_readonly("compiler", &JPS_BuildInfo::compiler)
        .def_readonly("compiler_version", &JPS_BuildInfo::compiler_version)
        .def_readonly("library_version", &JPS_BuildInfo::library_version);
    m.def("get_build_info", []() { return JPS_GetBuildInfo(); });
    py::class_<JPS_Trace>(m, "Trace")
        .def_readonly("iteration_duration", &JPS_Trace::iteration_duration)
        .def_readonly("operational_level_duration", &JPS_Trace::operational_level_duration)
        .def("__repr__", [](const JPS_Trace& t) {
            return fmt::format(
                "Trace( Iteration: {:d}us, OperationalLevel {:d}us)",
                t.iteration_duration,
                t.operational_level_duration);
        });
    py::class_<JPS_GeneralizedCentrifugalForceModelAgentParameters>(
        m, "GeneralizedCentrifugalForceModelAgentParameters")
        .def(
            py::init([](double speed,
                        std::tuple<double, double> e0,
                        std::tuple<double, double> position,
                        std::tuple<double, double> orientation,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        double mass,
                        double tau,
                        double v0,
                        double a_v,
                        double a_min,
                        double b_min,
                        double b_max) {
                return JPS_GeneralizedCentrifugalForceModelAgentParameters{
                    speed,
                    intoJPS_Point(e0),
                    intoJPS_Point(position),
                    intoJPS_Point(orientation),
                    journey_id,
                    stage_id,
                    mass,
                    tau,
                    v0,
                    a_v,
                    a_min,
                    b_min,
                    b_max};
            }),
            py::kw_only(),
            py::arg("speed"),
            py::arg("e0"),
            py::arg("position"),
            py::arg("orientation"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("mass"),
            py::arg("tau"),
            py::arg("v0"),
            py::arg("a_v"),
            py::arg("a_min"),
            py::arg("b_min"),
            py::arg("b_max"))
        .def("__repr__", [](const JPS_GeneralizedCentrifugalForceModelAgentParameters& p) {
            return fmt::format(
                "speed: {}, e0: {}, position: {}, orientation: {}, journey_id: {}, "
                "stage_id: {}, mass: {}, v0: {}, a_v: {}, a_min: {}, b_min: {}, b_max: {}",
                p.speed,
                intoTuple(p.e0),
                intoTuple(p.position),
                intoTuple(p.orientation),
                p.journeyId,
                p.stageId,
                p.mass,
                p.v0,
                p.a_v,
                p.a_min,
                p.b_min,
                p.b_max);
        });
    py::class_<JPS_CollisionFreeSpeedModelAgentParameters>(
        m, "CollisionFreeSpeedModelAgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        double time_gap,
                        double v0,
                        double radius,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id) {
                return JPS_CollisionFreeSpeedModelAgentParameters{
                    intoJPS_Point(position), journey_id, stage_id, time_gap, v0, radius};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("time_gap"),
            py::arg("v0"),
            py::arg("radius"),
            py::arg("journey_id"),
            py::arg("stage_id"))
        .def("__repr__", [](const JPS_CollisionFreeSpeedModelAgentParameters& p) {
            return fmt::format(
                "position: {}, journey_id: {}, stage_id: {}, "
                "time_gap: {}, v0: {}, radius: {}",
                intoTuple(p.position),
                p.journeyId,
                p.stageId,
                p.time_gap,
                p.v0,
                p.radius);
        });
    py::class_<JPS_CollisionFreeSpeedModelV2AgentParameters>(
        m, "CollisionFreeSpeedModelV2AgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        double time_gap,
                        double v0,
                        double radius,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double rangeGeometryRepulsion) {
                return JPS_CollisionFreeSpeedModelV2AgentParameters{
                    intoJPS_Point(position),
                    journey_id,
                    stage_id,
                    time_gap,
                    v0,
                    radius,
                    strengthNeighborRepulsion,
                    rangeNeighborRepulsion,
                    strengthGeometryRepulsion,
                    rangeGeometryRepulsion};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("time_gap"),
            py::arg("v0"),
            py::arg("radius"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"))
        .def("__repr__", [](const JPS_CollisionFreeSpeedModelV2AgentParameters& p) {
            return fmt::format(
                "position: {}, journey_id: {}, stage_id: {}, "
                "time_gap: {}, v0: {}, radius: {}, strength_neighbor_repulsion: {},"
                "range_neighbor_repulsion: {}, strength_geometry_repulsion: {}"
                "range_geometry_repulsion: {}",
                intoTuple(p.position),
                p.journeyId,
                p.stageId,
                p.time_gap,
                p.v0,
                p.radius,
                p.strengthNeighborRepulsion,
                p.rangeNeighborRepulsion,
                p.strengthGeometryRepulsion,
                p.rangeGeometryRepulsion);
        });

    py::class_<JPS_Geometry_Wrapper>(m, "Geometry")
        .def(
            "boundary",
            [](const JPS_Geometry_Wrapper& w) {
                const auto len = JPS_Geometry_GetBoundarySize(w.handle);
                const auto data = JPS_Geometry_GetBoundaryData(w.handle);
                return intoTuple(data, data + len);
            })
        .def("holes", [](const JPS_Geometry_Wrapper& w) {
            const auto holeCount = JPS_Geometry_GetHoleCount(w.handle);
            std::vector<std::vector<std::tuple<double, double>>> res{};
            res.reserve(holeCount);
            for(size_t index = 0; index < holeCount; ++index) {
                const auto len = JPS_Geometry_GetHoleSize(w.handle, index, nullptr);
                const auto data = JPS_Geometry_GetHoleData(w.handle, index, nullptr);
                res.emplace_back(intoTuple(data, data + len));
            }
            return res;
        });
    py::class_<JPS_GeometryBuilder_Wrapper>(m, "GeometryBuilder")
        .def(py::init([]() {
            return std::make_unique<JPS_GeometryBuilder_Wrapper>(JPS_GeometryBuilder_Create());
        }))
        .def(
            "add_accessible_area",
            [](const JPS_GeometryBuilder_Wrapper& w,
               std::vector<std::tuple<double, double>> polygon) {
                const auto pts = intoJPS_Point(polygon);
                JPS_GeometryBuilder_AddAccessibleArea(w.handle, pts.data(), pts.size());
            },
            "Add area where agents can move")
        .def(
            "exclude_from_accessible_area",
            [](const JPS_GeometryBuilder_Wrapper& w,
               std::vector<std::tuple<double, double>> polygon) {
                const auto pts = intoJPS_Point(polygon);
                JPS_GeometryBuilder_ExcludeFromAccessibleArea(w.handle, pts.data(), pts.size());
            },
            "Add areas where agents can not move (obstacles)")
        .def(
            "build",
            [](const JPS_GeometryBuilder_Wrapper& w) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_GeometryBuilder_Build(w.handle, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Geometry_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            "Geometry builder");
    py::class_<JPS_OperationalModel_Wrapper>(m, "OperationalModel");
    py::class_<JPS_CollisionFreeSpeedModelBuilder_Wrapper>(m, "CollisionFreeSpeedModelBuilder")
        .def(
            py::init([](double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double rangeGeometryRepulsion) {
                return std::make_unique<JPS_CollisionFreeSpeedModelBuilder_Wrapper>(
                    JPS_CollisionFreeSpeedModelBuilder_Create(
                        strengthNeighborRepulsion,
                        rangeNeighborRepulsion,
                        strengthGeometryRepulsion,
                        rangeGeometryRepulsion));
            }),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"))
        .def("build", [](JPS_CollisionFreeSpeedModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_CollisionFreeSpeedModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_CollisionFreeSpeedModelV2Builder_Wrapper>(m, "CollisionFreeSpeedModelV2Builder")
        .def(py::init([]() {
            return std::make_unique<JPS_CollisionFreeSpeedModelV2Builder_Wrapper>(
                JPS_CollisionFreeSpeedModelV2Builder_Create());
        }))
        .def("build", [](JPS_CollisionFreeSpeedModelV2Builder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_CollisionFreeSpeedModelV2Builder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });

    py::class_<JPS_GeneralizedCentrifugalForceModelBuilder_Wrapper>(
        m, "GeneralizedCentrifugalForceModelBuilder")
        .def(
            py::init([](double strengthNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double maxNeighborInteractionDistance,
                        double maxGeometryInteractionDistance,
                        double maxNeighborInterpolationDistance,
                        double maxGeometryInterpolationDistance,
                        double maxNeighborRepulsionForce,
                        double maxGeometryRepulsionForce) {
                return std::make_unique<JPS_GeneralizedCentrifugalForceModelBuilder_Wrapper>(
                    JPS_GeneralizedCentrifugalForceModelBuilder_Create(
                        strengthNeighborRepulsion,
                        strengthGeometryRepulsion,
                        maxNeighborInteractionDistance,
                        maxGeometryInteractionDistance,
                        maxNeighborInterpolationDistance,
                        maxGeometryInterpolationDistance,
                        maxNeighborRepulsionForce,
                        maxGeometryRepulsionForce));
            }),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("max_neighbor_interaction_distance"),
            py::arg("max_geometry_interaction_distance"),
            py::arg("max_neighbor_interpolation_distance"),
            py::arg("max_geometry_interpolation_distance"),
            py::arg("max_neighbor_repulsion_force"),
            py::arg("max_geometry_repulsion_force"))
        .def("build", [](JPS_GeneralizedCentrifugalForceModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_GeneralizedCentrifugalForceModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_Transition_Wrapper>(m, "Transition")
        .def_static(
            "create_fixed_transition",
            [](JPS_StageId stageId) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Transition_CreateFixedTransition(stageId, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Transition_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def_static(
            "create_round_robin_transition",
            [](const std::vector<std::tuple<JPS_StageId, uint64_t>>& stageWeights) {
                JPS_ErrorMessage errorMsg{};
                std::vector<JPS_StageId> stageIds;
                stageIds.reserve(stageWeights.size());
                std::vector<uint64_t> weights;
                weights.reserve(stageWeights.size());
                for(auto const& [stageId, weight] : stageWeights) {
                    stageIds.emplace_back(stageId);
                    weights.emplace_back(weight);
                }
                auto result = JPS_Transition_CreateRoundRobinTransition(
                    stageIds.data(), weights.data(), stageIds.size(), &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Transition_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def_static("create_least_targeted_transition", [](const std::vector<JPS_StageId>& stages) {
            JPS_ErrorMessage errorMsg{};
            std::vector<JPS_StageId> stageIds;
            stageIds.reserve(stages.size());
            std::copy(std::begin(stages), std::end(stages), std::back_inserter(stageIds));

            auto result = JPS_Transition_CreateLeastTargetedTransition(
                stageIds.data(), stageIds.size(), &errorMsg);
            if(result) {
                return std::make_unique<JPS_Transition_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });

    py::class_<JPS_JourneyDescription_Wrapper>(m, "JourneyDescription")
        .def(py::init([]() {
            return std::make_unique<JPS_JourneyDescription_Wrapper>(
                JPS_JourneyDescription_Create());
        }))
        .def(py::init([](const std::vector<JPS_StageId>& ids) {
            auto desc =
                std::make_unique<JPS_JourneyDescription_Wrapper>(JPS_JourneyDescription_Create());
            for(const auto id : ids) {
                JPS_JourneyDescription_AddStage(desc->handle, id);
            }
            return desc;
        }))
        .def(
            "add",
            [](JPS_JourneyDescription_Wrapper& w, JPS_StageId id) {
                JPS_JourneyDescription_AddStage(w.handle, id);
            })
        .def(
            "add",
            [](JPS_JourneyDescription_Wrapper& w, const std::vector<JPS_StageId>& ids) {
                for(const auto& id : ids) {
                    JPS_JourneyDescription_AddStage(w.handle, id);
                }
            })
        .def(
            "set_transition_for_stage",
            [](JPS_JourneyDescription_Wrapper& w,
               JPS_StageId stageId,
               JPS_Transition_Wrapper& transition) {
                JPS_ErrorMessage errorMsg{};
                auto success = JPS_JourneyDescription_SetTransitionForStage(
                    w.handle, stageId, transition.handle, &errorMsg);
                if(!success) {
                    auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                    JPS_ErrorMessage_Free(errorMsg);
                    throw std::runtime_error{msg};
                }
            });
    py::class_<JPS_AgentIterator_Wrapper>(m, "CollisionFreeSpeedModelAgentIterator")
        .def(
            "__iter__",
            [](JPS_AgentIterator_Wrapper& w) -> JPS_AgentIterator_Wrapper& { return w; })
        .def("__next__", [](JPS_AgentIterator_Wrapper& w) {
            const auto result = JPS_AgentIterator_Next(w.handle);
            if(result) {
                return std::make_unique<JPS_Agent_Wrapper>(result);
            }
            throw py::stop_iteration{};
        });
    py::class_<JPS_AgentIdIterator_Wrapper>(m, "AgentIdIterator")
        .def(
            "__iter__",
            [](JPS_AgentIdIterator_Wrapper& w) -> JPS_AgentIdIterator_Wrapper& { return w; })
        .def("__next__", [](JPS_AgentIdIterator_Wrapper& w) {
            const auto id = JPS_AgentIdIterator_Next(w.handle);
            if(id != 0) {
                return id;
            }
            throw py::stop_iteration{};
        });
    py::class_<JPS_GeneralizedCentrifugalForceModelState_Wrapper>(
        m, "GeneralizedCentrifugalForceModelState")
        .def_property(
            "speed",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetSpeed(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double speed) {
                JPS_GeneralizedCentrifugalForceModelState_SetSpeed(w.handle, speed);
            })
        .def_property(
            "e0",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return intoTuple(JPS_GeneralizedCentrifugalForceModelState_GetE0(w.handle));
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w,
               std::tuple<double, double> e0) {
                JPS_GeneralizedCentrifugalForceModelState_SetE0(w.handle, intoJPS_Point(e0));
            })
        .def_property(
            "mass",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetMass(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double mass) {
                JPS_GeneralizedCentrifugalForceModelState_SetMass(w.handle, mass);
            })
        .def_property(
            "tau",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetTau(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double tau) {
                JPS_GeneralizedCentrifugalForceModelState_SetTau(w.handle, tau);
            })
        .def_property(
            "v0",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetV0(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double v0) {
                JPS_GeneralizedCentrifugalForceModelState_SetV0(w.handle, v0);
            })
        .def_property(
            "a_v",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetAV(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double a_v) {
                JPS_GeneralizedCentrifugalForceModelState_SetAV(w.handle, a_v);
            })
        .def_property(
            "a_min",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetAMin(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double a_min) {
                JPS_GeneralizedCentrifugalForceModelState_SetAMin(w.handle, a_min);
            })
        .def_property(
            "b_min",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetBMin(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double b_min) {
                JPS_GeneralizedCentrifugalForceModelState_SetBMin(w.handle, b_min);
            })
        .def_property(
            "b_max",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetBMax(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double b_max) {
                JPS_GeneralizedCentrifugalForceModelState_SetBMax(w.handle, b_max);
            });
    py::class_<JPS_CollisionFreeSpeedModelState_Wrapper>(m, "CollisionFreeSpeedModelState")
        .def_property(
            "time_gap",
            [](const JPS_CollisionFreeSpeedModelState_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelState_GetTimeGap(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelState_Wrapper& w, double time_gap) {
                JPS_CollisionFreeSpeedModelState_SetTimeGap(w.handle, time_gap);
            })
        .def_property(
            "v0",
            [](const JPS_CollisionFreeSpeedModelState_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelState_GetV0(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelState_Wrapper& w, double v0) {
                JPS_CollisionFreeSpeedModelState_SetV0(w.handle, v0);
            })
        .def_property(
            "radius",
            [](const JPS_CollisionFreeSpeedModelState_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelState_GetRadius(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelState_Wrapper& w, double radius) {
                JPS_CollisionFreeSpeedModelState_SetRadius(w.handle, radius);
            });
    py::class_<JPS_CollisionFreeSpeedModelV2State_Wrapper>(m, "CollisionFreeSpeedModelV2State")
        .def_property(
            "time_gap",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetTimeGap(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double time_gap) {
                JPS_CollisionFreeSpeedModelV2State_SetTimeGap(w.handle, time_gap);
            })
        .def_property(
            "v0",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetV0(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double v0) {
                JPS_CollisionFreeSpeedModelV2State_SetV0(w.handle, v0);
            })
        .def_property(
            "radius",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetRadius(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double radius) {
                JPS_CollisionFreeSpeedModelV2State_SetRadius(w.handle, radius);
            })
        .def_property(
            "strength_neighbor_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetStrengthNeighborRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double strengthNeighborRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetStrengthNeighborRepulsion(
                    w.handle, strengthNeighborRepulsion);
            })
        .def_property(
            "range_neighbor_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetRangeNeighborRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double rangeNeighborRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetRangeNeighborRepulsion(
                    w.handle, rangeNeighborRepulsion);
            })
        .def_property(
            "strength_geometry_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetStrengthGeometryRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double strengthGeometryRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetStrengthGeometryRepulsion(
                    w.handle, strengthGeometryRepulsion);
            })
        .def_property(
            "range_geometry_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetRangeGeometryRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double rangeGeometryRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetRangeGeometryRepulsion(
                    w.handle, rangeGeometryRepulsion);
            });

    py::class_<JPS_NotifiableQueueProxy_Wrapper>(m, "NotifiableQueueProxy")
        .def(
            "count_targeting",
            [](const JPS_NotifiableQueueProxy_Wrapper& w) {
                return JPS_NotifiableQueueProxy_GetCountTargeting(w.handle);
            })
        .def(
            "count_enqueued",
            [](const JPS_NotifiableQueueProxy_Wrapper& w) {
                return JPS_NotifiableQueueProxy_GetCountEnqueued(w.handle);
            })
        .def(
            "pop",
            [](JPS_NotifiableQueueProxy_Wrapper& w, size_t count) {
                JPS_NotifiableQueueProxy_Pop(w.handle, count);
            })
        .def("enqueued", [](const JPS_NotifiableQueueProxy_Wrapper& w) {
            const JPS_AgentId* ids{};
            const auto count = JPS_NotifiableQueueProxy_GetEnqueued(w.handle, &ids);
            return std::vector<JPS_AgentId>{ids, ids + count};
        });
    py::enum_<JPS_WaitingSetState>(m, "WaitingSetState")
        .value("Active", JPS_WaitingSet_Active)
        .value("Inactive", JPS_WaitingSet_Inactive);
    py::class_<JPS_WaitingSetProxy_Wrapper>(m, "WaitingSetProxy")
        .def(
            "count_targeting",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                return JPS_WaitingSetProxy_GetCountTargeting(w.handle);
            })
        .def(
            "count_waiting",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                return JPS_WaitingSetProxy_GetCountWaiting(w.handle);
            })
        .def(
            "waiting",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                const JPS_AgentId* ids{};
                const auto count = JPS_WaitingSetProxy_GetWaiting(w.handle, &ids);
                return std::vector<JPS_AgentId>{ids, ids + count};
            })
        .def_property(
            "state",
            [](const JPS_WaitingSetProxy_Wrapper& w) {
                return JPS_WaitingSetProxy_GetWaitingSetState(w.handle);
            },
            [](JPS_WaitingSetProxy_Wrapper& w, JPS_WaitingSetState state) {
                JPS_WaitingSetProxy_SetWaitingSetState(w.handle, state);
            });
    py::class_<JPS_WaypointProxy_Wrapper>(m, "WaypointProxy")
        .def("count_targeting", [](const JPS_WaypointProxy_Wrapper& w) {
            return JPS_WaypointProxy_GetCountTargeting(w.handle);
        });
    py::class_<JPS_ExitProxy_Wrapper>(m, "ExitProxy")
        .def("count_targeting", [](const JPS_ExitProxy_Wrapper& w) {
            return JPS_ExitProxy_GetCountTargeting(w.handle);
        });
    py::class_<JPS_Agent_Wrapper>(m, "Agent")
        .def_property_readonly(
            "id", [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetId(w.handle); })
        .def_property_readonly(
            "journey_id",
            [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetJourneyId(w.handle); })
        .def_property_readonly(
            "stage_id", [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetStageId(w.handle); })
        .def_property_readonly(
            "position",
            [](const JPS_Agent_Wrapper& w) { return intoTuple(JPS_Agent_GetPosition(w.handle)); })
        .def_property_readonly(
            "orientation",
            [](const JPS_Agent_Wrapper& w) {
                return intoTuple(JPS_Agent_GetOrientation(w.handle));
            })
        .def_property_readonly(
            "model",
            [](const JPS_Agent_Wrapper& w)
                -> std::variant<
                    std::unique_ptr<JPS_GeneralizedCentrifugalForceModelState_Wrapper>,
                    std::unique_ptr<JPS_CollisionFreeSpeedModelState_Wrapper>,
                    std::unique_ptr<JPS_CollisionFreeSpeedModelV2State_Wrapper>> {
                switch(JPS_Agent_GetModelType(w.handle)) {
                    case JPS_GeneralizedCentrifugalForceModel:
                        return std::make_unique<JPS_GeneralizedCentrifugalForceModelState_Wrapper>(
                            JPS_Agent_GetGeneralizedCentrifugalForceModelState(w.handle, nullptr));
                    case JPS_CollisionFreeSpeedModel:
                        return std::make_unique<JPS_CollisionFreeSpeedModelState_Wrapper>(
                            JPS_Agent_GetCollisionFreeSpeedModelState(w.handle, nullptr));

                    case JPS_CollisionFreeSpeedModelV2:
                        return std::make_unique<JPS_CollisionFreeSpeedModelV2State_Wrapper>(
                            JPS_Agent_GetCollisionFreeSpeedModelV2State(w.handle, nullptr));
                }

                UNREACHABLE();
            });
    py::class_<JPS_Simulation_Wrapper>(m, "Simulation")
        .def(
            py::init(
                [](JPS_OperationalModel_Wrapper& model, JPS_Geometry_Wrapper& geometry, double dT) {
                    JPS_ErrorMessage errorMsg{};
                    auto result =
                        JPS_Simulation_Create(model.handle, geometry.handle, dT, &errorMsg);
                    if(result) {
                        return std::make_unique<JPS_Simulation_Wrapper>(result);
                    }
                    auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                    JPS_ErrorMessage_Free(errorMsg);
                    throw std::runtime_error{msg};
                }),
            py::kw_only(),
            py::arg("model"),
            py::arg("geometry"),
            py::arg("dt"))
        .def(
            "add_waypoint_stage",
            [](JPS_Simulation_Wrapper& w, std::tuple<double, double> position, double distance) {
                JPS_ErrorMessage errorMsg{};
                const auto result = JPS_Simulation_AddStageWaypoint(
                    w.handle, intoJPS_Point(position), distance, &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_queue_stage",
            [](JPS_Simulation_Wrapper& w,
               const std::vector<std::tuple<double, double>>& positions) {
                JPS_ErrorMessage errorMsg{};
                const auto jpsPointPositions = intoJPS_Point(positions);
                const auto result = JPS_Simulation_AddStageNotifiableQueue(
                    w.handle, jpsPointPositions.data(), jpsPointPositions.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_waiting_set_stage",
            [](JPS_Simulation_Wrapper& w,
               const std::vector<std::tuple<double, double>>& positions) {
                JPS_ErrorMessage errorMsg{};
                const auto jpsPointPositions = intoJPS_Point(positions);
                const auto result = JPS_Simulation_AddStageWaitingSet(
                    w.handle, jpsPointPositions.data(), jpsPointPositions.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_exit_stage",
            [](JPS_Simulation_Wrapper& w, const std::vector<std::tuple<double, double>>& polygon) {
                JPS_ErrorMessage errorMsg{};
                const auto jpsPointPoly = intoJPS_Point(polygon);
                const auto result = JPS_Simulation_AddStageExit(
                    w.handle, jpsPointPoly.data(), jpsPointPoly.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_journey",
            [](JPS_Simulation_Wrapper& simulation, JPS_JourneyDescription_Wrapper& journey) {
                JPS_ErrorMessage errorMsg{};
                const auto result =
                    JPS_Simulation_AddJourney(simulation.handle, journey.handle, &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation,
               JPS_GeneralizedCentrifugalForceModelAgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_AddGeneralizedCentrifugalForceModelAgent(
                    simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation,
               JPS_CollisionFreeSpeedModelAgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_AddCollisionFreeSpeedModelAgent(
                    simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation,
               JPS_CollisionFreeSpeedModelV2AgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_AddCollisionFreeSpeedModelV2Agent(
                    simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "mark_agent_for_removal",
            [](JPS_Simulation_Wrapper& simulation, JPS_AgentId id) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_MarkAgentForRemoval(simulation.handle, id, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "removed_agents",
            [](const JPS_Simulation_Wrapper& simulation) {
                const JPS_AgentId* ids{};
                const auto count = JPS_Simulation_RemovedAgents(simulation.handle, &ids);
                return std::vector<JPS_AgentId>{ids, ids + count};
            })
        .def(
            "iterate",
            [](const JPS_Simulation_Wrapper& simulation) {
                JPS_ErrorMessage errorMsg{};
                bool iterate_ok = JPS_Simulation_Iterate(simulation.handle, &errorMsg);
                if(iterate_ok) {
                    return;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "switch_agent_journey",
            [](const JPS_Simulation_Wrapper& w,
               JPS_AgentId agentId,
               JPS_JourneyId journeyId,
               JPS_StageId stageId) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_SwitchAgentJourney(
                    w.handle, agentId, journeyId, stageId, &errorMsg);
                if(result) {
                    return;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            py::kw_only(),
            py::arg("agent_id"),
            py::arg("journey_id"),
            py::arg("stage_id"))
        .def(
            "agent_count",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_AgentCount(simulation.handle);
            })
        .def(
            "elapsed_time",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_ElapsedTime(simulation.handle);
            })
        .def(
            "delta_time",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_DeltaTime(simulation.handle);
            })
        .def(
            "iteration_count",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_IterationCount(simulation.handle);
            })
        .def(
            "agents",
            [](const JPS_Simulation_Wrapper& simulation) {
                return std::make_unique<JPS_AgentIterator_Wrapper>(
                    JPS_Simulation_AgentIterator(simulation.handle));
            })
        .def(
            "agent",
            [](const JPS_Simulation_Wrapper& simulation, JPS_AgentId agentId) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_GetAgent(simulation.handle, agentId, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Agent_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            py::arg("agent_id"))
        .def(
            "agents_in_range",
            [](JPS_Simulation_Wrapper& w, std::tuple<double, double> pos, double distance) {
                return std::make_unique<JPS_AgentIdIterator_Wrapper>(
                    JPS_Simulation_AgentsInRange(w.handle, intoJPS_Point(pos), distance));
            })
        .def(
            "agents_in_polygon",
            [](JPS_Simulation_Wrapper& w, const std::vector<std::tuple<double, double>>& poly) {
                const auto ppoly = intoJPS_Point(poly);
                return std::make_unique<JPS_AgentIdIterator_Wrapper>(
                    JPS_Simulation_AgentsInPolygon(w.handle, ppoly.data(), ppoly.size()));
            })
        .def(
            "get_stage_proxy",
            [](JPS_Simulation_Wrapper& w, JPS_StageId id)
                -> std::variant<
                    std::unique_ptr<JPS_WaypointProxy_Wrapper>,
                    std::unique_ptr<JPS_NotifiableQueueProxy_Wrapper>,
                    std::unique_ptr<JPS_WaitingSetProxy_Wrapper>,
                    std::unique_ptr<JPS_ExitProxy_Wrapper>> {
                const auto type = JPS_Simulation_GetStageType(w.handle, id);
                JPS_ErrorMessage errorMessage{};
                const auto raise = [](JPS_ErrorMessage err) {
                    const auto msg = std::string(JPS_ErrorMessage_GetMessage(err));
                    JPS_ErrorMessage_Free(err);
                    throw std::runtime_error{msg};
                };

                switch(type) {
                    case JPS_NotifiableQueueType: {
                        auto ptr = std::make_unique<JPS_NotifiableQueueProxy_Wrapper>(
                            JPS_Simulation_GetNotifiableQueueProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                    case JPS_WaitingSetType: {
                        auto ptr = std::make_unique<JPS_WaitingSetProxy_Wrapper>(
                            JPS_Simulation_GetWaitingSetProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                    case JPS_WaypointType: {
                        auto ptr = std::make_unique<JPS_WaypointProxy_Wrapper>(
                            JPS_Simulation_GetWaypointProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                    case JPS_ExitType: {
                        auto ptr = std::make_unique<JPS_ExitProxy_Wrapper>(
                            JPS_Simulation_GetExitProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                }
                UNREACHABLE();
            })
        .def(
            "set_tracing",
            [](JPS_Simulation_Wrapper& w, bool status) {
                JPS_Simulation_SetTracing(w.handle, status);
            })
        .def(
            "get_last_trace",
            [](JPS_Simulation_Wrapper& w) { return JPS_Simulation_GetTrace(w.handle); })
        .def(
            "get_geometry",
            [](const JPS_Simulation_Wrapper& w) {
                return std::make_unique<JPS_Geometry_Wrapper>(JPS_Simulation_GetGeometry(w.handle));
            })
        .def(
            "get_geometry",
            [](const JPS_Simulation_Wrapper& w) {
                return std::make_unique<JPS_Geometry_Wrapper>(JPS_Simulation_GetGeometry(w.handle));
            })
        .def("switch_geometry", [](JPS_Simulation_Wrapper& w, JPS_Geometry_Wrapper& geometry) {
            JPS_ErrorMessage errorMsg{};

            auto success =
                JPS_Simulation_SwitchGeometry(w.handle, geometry.handle, nullptr, &errorMsg);

            if(!success) {
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            }
            return success;
        });
    py::class_<JPS_RoutingEngine_Wrapper>(m, "RoutingEngine")
        .def(py::init([](const JPS_Geometry_Wrapper& geo) {
            return std::make_unique<JPS_RoutingEngine_Wrapper>(
                JPS_RoutingEngine_Create(geo.handle));
        }))
        .def(
            "compute_waypoints",
            [](const JPS_RoutingEngine_Wrapper& w,
               std::tuple<double, double> from,
               std::tuple<double, double> to) {
                auto intoJPS_Point = [](const auto p) {
                    return JPS_Point{std::get<0>(p), std::get<1>(p)};
                };
                auto waypoints = JPS_RoutingEngine_ComputeWaypoint(
                    w.handle, intoJPS_Point(from), intoJPS_Point(to));
                std::vector<std::tuple<double, double>> result;
                result.reserve(waypoints.len);
                std::transform(
                    waypoints.points,
                    waypoints.points + waypoints.len,
                    std::back_inserter(result),
                    [](const auto& p) { return std::make_tuple(p.x, p.y); });
                JPS_Path_Free(&waypoints);
                return result;
            })
        .def(
            "is_routable",
            [](const JPS_RoutingEngine_Wrapper& w, std::tuple<double, double> p) {
                return JPS_RoutingEngine_IsRoutable(w.handle, intoJPS_Point(p));
            })
        .def(
            "mesh",
            [](const JPS_RoutingEngine_Wrapper& w) {
                auto result = JPS_RoutingEngine_Mesh(w.handle);
                using Pt = std::tuple<double, double>;
                using Tri = std::tuple<Pt, Pt, Pt>;
                std::vector<Tri> mesh{};
                mesh.reserve(result.len);
                std::transform(
                    result.triangles,
                    result.triangles + result.len,
                    std::back_inserter(mesh),
                    [](const auto& t) {
                        return std::make_tuple(
                            std::make_tuple(t.points[0].x, t.points[0].y),
                            std::make_tuple(t.points[1].x, t.points[1].y),
                            std::make_tuple(t.points[2].x, t.points[2].y));
                    });
                JPS_TriangleMesh_Free(&result);
                return mesh;
            })
        .def("edges_for", [](const JPS_RoutingEngine_Wrapper& w, uint32_t id) {
            auto res = JPS_RoutingEngine_EdgesFor(w.handle, id);
            using Pt = std::tuple<double, double>;
            using Line = std::tuple<Pt, Pt>;
            std::vector<Line> lines{};
            lines.reserve(res.len);
            std::transform(
                res.lines, res.lines + res.len, std::back_inserter(lines), [](const auto& l) {
                    return std::make_tuple(
                        std::make_tuple(l.points[0].x, l.points[0].y),
                        std::make_tuple(l.points[1].x, l.points[1].y));
                });
            JPS_Lines_Free(&res);
            return lines;
        });
}
