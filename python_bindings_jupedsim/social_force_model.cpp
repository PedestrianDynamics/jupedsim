#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_social_force_model(py::module_& m)
{
    py::class_<JPS_SocialForceModelAgentParameters>(
        m, "SocialForceModelAgentParameters")
        .def(
            py::init([](double test_value) {
                return JPS_SocialForceModelAgentParameters{
                    test_value};
            }),
            py::kw_only(),
            py::arg("test_value"))
        .def("__repr__", [](const JPS_SocialForceModelAgentParameters& p) {
            return fmt::format(
                "test_value: {}",
                p.test_value);
        });
    py::class_<JPS_SocialForceModelBuilder_Wrapper>(
        m, "SocialForceModelBuilder")
        .def(
            py::init([](double test_value) {
                return std::make_unique<JPS_SocialForceModelBuilder_Wrapper>(
                    JPS_SocialForceModelBuilder_Create(
                        test_value));
            }),
            py::kw_only(),
            py::arg("test_value"))
        .def("build", [](JPS_SocialForceModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_SocialForceModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_SocialForceModelState_Wrapper>(
        m, "SocialForceModelState")
        .def_property(
            "test_value",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetTestValue(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double test_value) {
                JPS_SocialForceModelState_SetTestValue(w.handle, test_value);
            });
}
