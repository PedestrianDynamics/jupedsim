#include <jupedsim/jupedsim.h>

#include <algorithm>
#include <exception>
#include <iterator>
#include <memory>
#include <vector>

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
        cls##_Wrapper(cls h) : handle(h) {}                                                        \
        ~cls##_Wrapper() { cls##_Free(handle); }                                                   \
    }

#define WRAPPER(cls)                                                                               \
    struct cls##_Wrapper {                                                                         \
        cls handle;                                                                                \
        cls##_Wrapper(cls h) : handle(h) {}                                                        \
        ~cls##_Wrapper() {}                                                                        \
    }

OWNED_WRAPPER(JPS_Geometry);
OWNED_WRAPPER(JPS_GeometryBuilder);
OWNED_WRAPPER(JPS_OperationalModel);
OWNED_WRAPPER(JPS_Areas);
OWNED_WRAPPER(JPS_AreasBuilder);
WRAPPER(JPS_Agent);
OWNED_WRAPPER(JPS_Simulation);

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
    m.def("set_debug_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().debug = callback;
        JPS_Logging_SetDebugCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().debug(msg); });
    });
    m.def("set_info_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().info = callback;
        JPS_Logging_SetInfoCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().info(msg); });
    });
    m.def("set_warning_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().warning = callback;
        JPS_Logging_SetWarningCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().warning(msg); });
    });
    m.def("set_error_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().error = callback;
        JPS_Logging_SetErrorCallback(
            [](const char* msg, void*) { LogCallbackOwner::Instance().error(msg); });
    });
    py::class_<JPS_Geometry_Wrapper>(m, "Geometry");
    py::class_<JPS_GeometryBuilder_Wrapper>(m, "GeometryBuilder")
        .def(py::init([]() {
            return std::make_unique<JPS_GeometryBuilder_Wrapper>(JPS_GeometryBuilder_Create());
        }))
        .def(
            "add_accessible_area",
            [](const JPS_GeometryBuilder_Wrapper& w, std::vector<double> points) {
                JPS_GeometryBuilder_AddAccessibleArea(w.handle, points.data(), points.size() / 2);
            },
            "")
        .def(
            "exclude_from_accssible_area",
            [](const JPS_GeometryBuilder_Wrapper& w, std::vector<double> points) {
                JPS_GeometryBuilder_ExcludeFromAccessibleArea(
                    w.handle, points.data(), points.size() / 2);
            },
            "")
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
            "");
    py::class_<JPS_OperationalModel_Wrapper>(m, "OperationalModel")
        .def_static(
            "make_velocity_model",
            [](double a_Ped, double D_Ped, double a_Wall, double D_Wall) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_OperationalModel_Create_VelocityModel(
                    a_Ped, D_Ped, a_Wall, D_Wall, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_OperationalModel_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def_static(
            "make_gcfm_model",
            [](double nu_Ped,
               double nu_Wall,
               double dist_eff_Ped,
               double dist_eff_Wall,
               double intp_width_Ped,
               double intp_width_Wall,
               double maxf_Ped,
               double maxf_Wall) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_OperationalModel_Create_GCFMModel(
                    nu_Ped,
                    nu_Wall,
                    dist_eff_Ped,
                    dist_eff_Wall,
                    intp_width_Ped,
                    intp_width_Wall,
                    maxf_Ped,
                    maxf_Wall,
                    &errorMsg);
                if(result) {
                    return std::make_unique<JPS_OperationalModel_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            });
    py::class_<JPS_Areas_Wrapper>(m, "Areas");
    py::class_<JPS_AreasBuilder_Wrapper>(m, "AreasBuilder")
        .def(py::init(
            []() { return std::make_unique<JPS_AreasBuilder_Wrapper>(JPS_AreasBuilder_Create()); }))
        .def(
            "add_area",
            [](const JPS_AreasBuilder_Wrapper& w,
               uint64_t id,
               std::vector<double> points,
               std::vector<std::string> tags) {
                std::vector<const char*> tags_as_c_str;
                tags_as_c_str.reserve(tags.size());
                std::transform(
                    tags.begin(),
                    tags.end(),
                    std::back_inserter(tags_as_c_str),
                    [](const auto& str) { return str.c_str(); });

                JPS_AreasBuilder_AddArea(
                    w.handle,
                    id,
                    points.data(),
                    points.size() / 2,
                    tags_as_c_str.data(),
                    tags.size());
            },
            "")
        .def(
            "build",
            [](const JPS_AreasBuilder_Wrapper& w) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_AreasBuilder_Build(w.handle, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Areas_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            "");
    py::class_<JPS_Agent_Wrapper>(m, "Agent")
        .def_property_readonly(
            "x", [](const JPS_Agent_Wrapper& w) { return JPS_Agent_PositionX(w.handle); })
        .def_property_readonly(
            "y", [](const JPS_Agent_Wrapper& w) { return JPS_Agent_PositionY(w.handle); })
        .def_property_readonly(
            "orientation_x",
            [](const JPS_Agent_Wrapper& w) { return JPS_Agent_OrientationX(w.handle); })
        .def_property_readonly("orientation_y", [](const JPS_Agent_Wrapper& w) {
            return JPS_Agent_OrientationY(w.handle);
        });
    py::class_<JPS_AgentParameters>(m, "AgentParameters")
        .def(py::init<>())
        .def_readwrite("x", &JPS_AgentParameters::positionX)
        .def_readwrite("y", &JPS_AgentParameters::positionY)
        .def_readwrite("orientation_x", &JPS_AgentParameters::orientationX)
        .def_readwrite("orientation_y", &JPS_AgentParameters::orientationY)
        .def_readwrite("v0", &JPS_AgentParameters::v0)
        .def_readwrite("t", &JPS_AgentParameters::T)
        .def_readwrite("tau", &JPS_AgentParameters::Tau)
        .def_readwrite("a_v", &JPS_AgentParameters::Av)
        .def_readwrite("a_min", &JPS_AgentParameters::AMin)
        .def_readwrite("b_max", &JPS_AgentParameters::BMax)
        .def_readwrite("b_min", &JPS_AgentParameters::BMin)
        .def_readwrite("destination_area_id", &JPS_AgentParameters::destinationAreaId);
    py::class_<JPS_Simulation_Wrapper>(m, "Simulation")
        .def(py::init([](JPS_OperationalModel_Wrapper& model,
                         JPS_Geometry_Wrapper& geometry,
                         JPS_Areas_Wrapper& areas,
                         double dT) {
            JPS_ErrorMessage errorMsg{};
            auto result =
                JPS_Simulation_Create(model.handle, geometry.handle, areas.handle, dT, &errorMsg);
            if(result) {
                return std::make_unique<JPS_Simulation_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        }))
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation, JPS_AgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_AddAgent(simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "read_agent",
            [](JPS_Simulation_Wrapper& simulation, JPS_AgentId id) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_ReadAgent(simulation.handle, id, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Agent_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "removed_agents",
            [](JPS_Simulation_Wrapper& simulation) {
                const JPS_AgentId* ids{};
                const auto count = JPS_Simulation_RemovedAgents(simulation.handle, &ids);
                return std::vector<std::unique_ptr<JPS_Agent_Wrapper>>{};
            })
        .def(
            "iterate",
            [](JPS_Simulation_Wrapper& simulation) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_Iterate(simulation.handle, &errorMsg);
                if(result) {
                    return;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "agent_count",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_AgentCount(simulation.handle);
            })
        .def("iteration_count", [](JPS_Simulation_Wrapper& simulation) {
            return JPS_Simulation_IterationCount(simulation.handle);
        });
}