// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <Unreachable.hpp>
#include <jupedsim/jupedsim.h>

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
OWNED_WRAPPER(JPS_VelocityModelBuilder);
OWNED_WRAPPER(JPS_GCFMModelBuilder);
OWNED_WRAPPER(JPS_JourneyDescription);
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
WRAPPER(JPS_VelocityModelState);

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

std::tuple<double, double> intoTuple(const JPS_Point& p)
{
    return std::make_tuple(p.x, p.y);
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
    py::class_<JPS_Point>(m, "Point")
        .def(py::init())
        .def(py::init([](std::tuple<double, double> tup) {
            return JPS_Point{std::get<0>(tup), std::get<1>(tup)};
        }))
        .def_readwrite("x", &JPS_Point::x)
        .def_readwrite("y", &JPS_Point::y)
        .def(
            "__getitem__",
            [](const JPS_Point& pt, int idx) {
                switch(idx) {
                    case 0:
                        return pt.x;
                    case 1:
                        return pt.y;
                    default:
                        throw pybind11::index_error{};
                }
            })
        .def(
            "__setitem__",
            [](JPS_Point& pt, int idx, double value) {
                switch(idx) {
                    case 0:
                        pt.x = value;
                        break;
                    case 1:
                        pt.y = value;
                        break;
                    default:
                        throw pybind11::index_error{};
                }
            })
        .def(
            "__eq__",
            [](const JPS_Point& lhs, const JPS_Point& rhs) {
                return lhs.x == rhs.x && lhs.y == rhs.y;
            })
        .def(
            "__str__", [](const JPS_Point& pt) { return fmt::format("({:f}, {:f})", pt.x, pt.y); });

    py::implicitly_convertible<std::tuple<double, double>, JPS_Point>();
    py::implicitly_convertible<std::tuple<int, int>, JPS_Point>();
    py::implicitly_convertible<std::tuple<int, double>, JPS_Point>();
    py::implicitly_convertible<std::tuple<double, int>, JPS_Point>();

    py::class_<JPS_Trace>(m, "Trace")
        .def_readonly("iteration_duration", &JPS_Trace::iteration_duration)
        .def_readonly("operational_level_duration", &JPS_Trace::operational_level_duration)
        .def("__repr__", [](const JPS_Trace& t) {
            return fmt::format(
                "Trace( Iteration: {:d}us, OperationalLevel {:d}us)",
                t.iteration_duration,
                t.operational_level_duration);
        });
    py::class_<JPS_GCFMModelAgentParameters>(m, "GCFMModelAgentParameters")
        .def(py::init())
        .def_readwrite("speed", &JPS_GCFMModelAgentParameters::speed)
        .def_readwrite("e0", &JPS_GCFMModelAgentParameters::e0)
        .def_readwrite("position", &JPS_GCFMModelAgentParameters::position)
        .def_readwrite("orientation", &JPS_GCFMModelAgentParameters::orientation)
        .def_readwrite("journey_id", &JPS_GCFMModelAgentParameters::journeyId)
        .def_readwrite("profile_id", &JPS_GCFMModelAgentParameters::profileId)
        .def_readwrite("id", &JPS_GCFMModelAgentParameters::agentId)
        .def("__repr__", [](const JPS_GCFMModelAgentParameters& p) {
            return fmt::format(
                "speed: {}, e0: {}, position: {}, orientation: {}, journey_id: {}, profile_id: {}, "
                "id: {}",
                p.speed,
                intoTuple(p.e0),
                intoTuple(p.position),
                intoTuple(p.orientation),
                p.journeyId,
                p.profileId,
                p.agentId);
        });
    py::class_<JPS_VelocityModelAgentParameters>(m, "VelocityModelAgentParameters")
        .def(py::init())
        .def_readwrite("e0", &JPS_VelocityModelAgentParameters::e0)
        .def_readwrite("position", &JPS_VelocityModelAgentParameters::position)
        .def_readwrite("orientation", &JPS_VelocityModelAgentParameters::orientation)
        .def_readwrite("journey_id", &JPS_VelocityModelAgentParameters::journeyId)
        .def_readwrite("profile_id", &JPS_VelocityModelAgentParameters::profileId)
        .def_readwrite("id", &JPS_VelocityModelAgentParameters::agentId)
        .def("__repr__", [](const JPS_VelocityModelAgentParameters& p) {
            return fmt::format(
                "e0: {}, position: {}, orientation: {}, journey_id: {}, profile_id: {}, "
                "id: {}",
                intoTuple(p.e0),
                intoTuple(p.position),
                intoTuple(p.orientation),
                p.journeyId,
                p.profileId,
                p.agentId);
        });
    py::class_<JPS_Geometry_Wrapper>(m, "Geometry");
    py::class_<JPS_GeometryBuilder_Wrapper>(m, "GeometryBuilder")
        .def(py::init([]() {
            return std::make_unique<JPS_GeometryBuilder_Wrapper>(JPS_GeometryBuilder_Create());
        }))
        .def(
            "add_accessible_area",
            [](const JPS_GeometryBuilder_Wrapper& w, std::vector<JPS_Point> polygon) {
                JPS_GeometryBuilder_AddAccessibleArea(w.handle, polygon.data(), polygon.size());
            },
            "Add area where agents can move")
        .def(
            "exclude_from_accssible_area",
            [](const JPS_GeometryBuilder_Wrapper& w, std::vector<JPS_Point> polygon) {
                JPS_GeometryBuilder_ExcludeFromAccessibleArea(
                    w.handle, polygon.data(), polygon.size());
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
    py::class_<JPS_VelocityModelBuilder_Wrapper>(m, "VelocityModelBuilder")
        .def(
            py::init([](double aPed, double DPed, double aWall, double DWall) {
                return std::make_unique<JPS_VelocityModelBuilder_Wrapper>(
                    JPS_VelocityModelBuilder_Create(aPed, DPed, aWall, DWall));
            }),
            py::kw_only(),
            py::arg("a_ped"),
            py::arg("d_ped"),
            py::arg("a_wall"),
            py::arg("d_wall"))
        .def(
            "add_parameter_profile",
            [](JPS_VelocityModelBuilder_Wrapper& w,
               JPS_ModelParameterProfileId id,
               double t,
               double tau,
               double v0,
               double radius) {
                JPS_VelocityModelBuilder_AddParameterProfile(w.handle, id, t, tau, v0, radius);
            },
            py::kw_only(),
            py::arg("id"),
            py::arg("time_gap"),
            py::arg("tau"),
            py::arg("v0"),
            py::arg("radius"))
        .def("build", [](JPS_VelocityModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_VelocityModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_GCFMModelBuilder_Wrapper>(m, "GCFMModelBuilder")
        .def(
            py::init([](double nu_Ped,
                        double nu_Wall,
                        double dist_eff_Ped,
                        double dist_eff_Wall,
                        double intp_width_Ped,
                        double intp_width_Wall,
                        double maxf_Ped,
                        double maxf_Wall) {
                return std::make_unique<JPS_GCFMModelBuilder_Wrapper>(JPS_GCFMModelBuilder_Create(
                    nu_Ped,
                    nu_Wall,
                    dist_eff_Ped,
                    dist_eff_Wall,
                    intp_width_Ped,
                    intp_width_Wall,
                    maxf_Ped,
                    maxf_Wall));
            }),
            py::kw_only(),
            py::arg("nuPed"),
            py::arg("nuWall"),
            py::arg("distEffPed"),
            py::arg("distEffWall"),
            py::arg("intpWidthPed"),
            py::arg("intpWidthWall"),
            py::arg("maxfPed"),
            py::arg("maxfWall"))
        .def(
            "add_parameter_profile",
            [](JPS_GCFMModelBuilder_Wrapper& w,
               JPS_ModelParameterProfileId id,
               double mass,
               double tau,
               double v0,
               double a_v,
               double a_min,
               double b_min,
               double b_max) {
                JPS_GCFMModelBuilder_AddParameterProfile(
                    w.handle, id, mass, tau, v0, a_v, a_min, b_min, b_max);
            },
            py::kw_only(),
            py::arg("id"),
            py::arg("mass"),
            py::arg("tau"),
            py::arg("v0"),
            py::arg("a_v"),
            py::arg("a_min"),
            py::arg("b_min"),
            py::arg("b_max"))
        .def("build", [](JPS_GCFMModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_GCFMModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
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
            "append",
            [](JPS_JourneyDescription_Wrapper& w, JPS_StageId id) {
                JPS_JourneyDescription_AddStage(w.handle, id);
            })
        .def("append", [](JPS_JourneyDescription_Wrapper& w, const std::vector<JPS_StageId>& ids) {
            for(const auto& id : ids) {
                JPS_JourneyDescription_AddStage(w.handle, id);
            }
        });
    py::class_<JPS_AgentIterator_Wrapper>(m, "VelocityModelAgentIterator")
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
        .def_property_readonly(
            "speed",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetSpeed(w.handle);
            })
        .def_property_readonly(
            "e0", [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetE0(w.handle);
            });
    py::class_<JPS_VelocityModelState_Wrapper>(m, "VelocityModelState")
        .def_property_readonly("e0", [](const JPS_VelocityModelState_Wrapper& w) {
            return JPS_VelocityModelState_GetE0(w.handle);
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
            "stage_index",
            [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetStageIndex(w.handle); })
        .def_property_readonly(
            "position", [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetPosition(w.handle); })
        .def_property_readonly(
            "orientation",
            [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetOrientation(w.handle); })
        .def_property_readonly(
            "model",
            [](const JPS_Agent_Wrapper& w)
                -> std::variant<
                    std::unique_ptr<JPS_GeneralizedCentrifugalForceModelState_Wrapper>,
                    std::unique_ptr<JPS_VelocityModelState_Wrapper>> {
                switch(JPS_Agent_GetModelType(w.handle)) {
                    case JPS_GCFMModel:
                        return std::make_unique<JPS_GeneralizedCentrifugalForceModelState_Wrapper>(
                            JPS_Agent_GetGeneralizedCentrifugalForceModelState(w.handle, nullptr));
                    case JPS_VelocityModel:
                        return std::make_unique<JPS_VelocityModelState_Wrapper>(
                            JPS_Agent_GetVelocityModelState(w.handle, nullptr));
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
            [](JPS_Simulation_Wrapper& w, JPS_Point position, double distance) {
                JPS_ErrorMessage errorMsg{};
                const auto result =
                    JPS_Simulation_AddStageWaypoint(w.handle, position, distance, &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_queue_stage",
            [](JPS_Simulation_Wrapper& w, const std::vector<JPS_Point>& positions) {
                JPS_ErrorMessage errorMsg{};
                const auto result = JPS_Simulation_AddStageNotifiableQueue(
                    w.handle, positions.data(), positions.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_waiting_set_stage",
            [](JPS_Simulation_Wrapper& w, const std::vector<JPS_Point>& positions) {
                JPS_ErrorMessage errorMsg{};
                const auto result = JPS_Simulation_AddStageWaitingSet(
                    w.handle, positions.data(), positions.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_exit_stage",
            [](JPS_Simulation_Wrapper& w, const std::vector<JPS_Point>& polygon) {
                JPS_ErrorMessage errorMsg{};
                const auto result = JPS_Simulation_AddStageExit(
                    w.handle, polygon.data(), polygon.size(), &errorMsg);
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
            [](JPS_Simulation_Wrapper& simulation, JPS_GCFMModelAgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result =
                    JPS_Simulation_AddGCFMModelAgent(simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation, JPS_VelocityModelAgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result =
                    JPS_Simulation_AddVelocityModelAgent(simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "remove_agent",
            [](JPS_Simulation_Wrapper& simulation, JPS_AgentId id) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_RemoveAgent(simulation.handle, id, &errorMsg);
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
            [](const JPS_Simulation_Wrapper& simulation, size_t count) {
                JPS_ErrorMessage errorMsg{};
                bool iterate_ok = true;
                for(size_t counter = 0; counter < count && iterate_ok; ++counter) {
                    iterate_ok = JPS_Simulation_Iterate(simulation.handle, &errorMsg);
                }
                if(iterate_ok) {
                    return;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            py::arg("count") = 1)
        .def(
            "switch_agent_profile",
            [](const JPS_Simulation_Wrapper& w,
               JPS_AgentId agentId,
               JPS_ModelParameterProfileId profileId) {
                JPS_ErrorMessage errorMsg{};
                auto result =
                    JPS_Simulation_SwitchAgentProfile(w.handle, agentId, profileId, &errorMsg);
                if(result) {
                    return;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            py::kw_only(),
            py::arg("agent_id"),
            py::arg("profile_id"))
        .def(
            "switch_agent_journey",
            [](const JPS_Simulation_Wrapper& w,
               JPS_AgentId agentId,
               JPS_JourneyId journeyId,
               JPS_StageIndex stageIdx) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_SwitchAgentJourney(
                    w.handle, agentId, journeyId, stageIdx, &errorMsg);
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
            py::arg("stage_index"))
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
            "agents_in_range",
            [](JPS_Simulation_Wrapper& w, std::tuple<double, double> pos, double distance) {
                return std::make_unique<JPS_AgentIdIterator_Wrapper>(JPS_Simulation_AgentsInRange(
                    w.handle, JPS_Point{std::get<0>(pos), std::get<1>(pos)}, distance));
            })
        .def(
            "agents_in_polygon",
            [](JPS_Simulation_Wrapper& w, const std::vector<std::tuple<double, double>>& poly) {
                std::vector<JPS_Point> ppoly{};
                ppoly.reserve(poly.size());
                std::transform(
                    std::begin(poly),
                    std::end(poly),
                    std::back_inserter(ppoly),
                    [](const auto& pt) {
                        return JPS_Point{std::get<0>(pt), std::get<1>(pt)};
                    });
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
        .def("get_last_trace", [](JPS_Simulation_Wrapper& w) {
            return JPS_Simulation_GetTrace(w.handle);
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
            [](const JPS_RoutingEngine_Wrapper& w, JPS_Point p) {
                return JPS_RoutingEngine_IsRoutable(w.handle, p);
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
