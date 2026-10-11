// SPDX-License-Identifier: LGPL-3.0-or-later
#include "logging.hpp"

#include "conversion.hpp"
#include "logger.hpp"

#include <pybind11/functional.h> // IWYU pragma: keep
#include <pybind11/pybind11.h>

#include <string>

namespace py = pybind11;

// TODO(kkratz): I think this can now be replaced by lifetime annotations, i.e. py::keep_alive...
LogCallbackOwner& LogCallbackOwner::instance()
{
    static LogCallbackOwner instance;
    return instance;
}

void init_logging(py::module_& m)
{
    auto atexit = py::module_::import("atexit");
    atexit.attr("register")(py::cpp_function([]() {
        auto& owner = LogCallbackOwner::instance();
        owner.debug = {};
        owner.info = {};
        owner.warning = {};
        owner.error = {};
    }));
    m.def(
        "set_debug_callback",
        [](LogCallbackOwner::LogCallback callback) {
            LogCallbackOwner::instance().debug = callback;
            logging::Logger::instance().set_debug_callback(
                [](const std::string& msg) { LogCallbackOwner::instance().debug(msg); });
        },
        py::arg("fn"),
        clean_doc(R"(
        Set receiver for debug messages.

        Args:
            fn: Callable that receives each message as a string.
        )")
            .c_str());
    m.def(
        "set_info_callback",
        [](LogCallbackOwner::LogCallback callback) {
            LogCallbackOwner::instance().info = callback;
            logging::Logger::instance().set_info_callback(
                [](const std::string& msg) { LogCallbackOwner::instance().info(msg); });
        },
        py::arg("fn"),
        clean_doc(R"(
        Set receiver for info messages.

        Args:
            fn: Callable that receives each message as a string.
        )")
            .c_str());
    m.def(
        "set_warning_callback",
        [](LogCallbackOwner::LogCallback callback) {
            LogCallbackOwner::instance().warning = callback;
            logging::Logger::instance().set_warning_callback(
                [](const std::string& msg) { LogCallbackOwner::instance().warning(msg); });
        },
        py::arg("fn"),
        clean_doc(R"(
        Set receiver for warning messages.

        Args:
            fn: Callable that receives each message as a string.
        )")
            .c_str());
    m.def(
        "set_error_callback",
        [](LogCallbackOwner::LogCallback callback) {
            LogCallbackOwner::instance().error = callback;
            logging::Logger::instance().set_error_callback(
                [](const std::string& msg) { LogCallbackOwner::instance().error(msg); });
        },
        py::arg("fn"),
        clean_doc(R"(
        Set receiver for error messages.

        Args:
            fn: Callable that receives each message as a string.
        )")
            .c_str());
}
