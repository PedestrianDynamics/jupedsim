// SPDX-License-Identifier: LGPL-3.0-or-later
#include "logging.hpp"

#include "conversion.hpp"

#include <Logger.hpp>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// TODO(kkratz): I think this can now be replaced by lifetime annotations, i.e. py::keep_alive...
LogCallbackOwner& LogCallbackOwner::Instance()
{
    static LogCallbackOwner instance;
    return instance;
}

void init_logging(py::module_& m)
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
        Logging::Logger::Instance().SetDebugCallback(
            [](const std::string& msg) { LogCallbackOwner::Instance().debug(msg); });
    });
    m.def("set_info_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().info = callback;
        Logging::Logger::Instance().SetInfoCallback(
            [](const std::string& msg) { LogCallbackOwner::Instance().info(msg); });
    });
    m.def("set_warning_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().warning = callback;
        Logging::Logger::Instance().SetWarningCallback(
            [](const std::string& msg) { LogCallbackOwner::Instance().warning(msg); });
    });
    m.def("set_error_callback", [](LogCallbackOwner::LogCallback callback) {
        LogCallbackOwner::Instance().error = callback;
        Logging::Logger::Instance().SetErrorCallback(
            [](const std::string& msg) { LogCallbackOwner::Instance().error(msg); });
    });
}
