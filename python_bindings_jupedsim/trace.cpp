// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Tracing.hpp"
#include "conversion.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_trace(py::module_& m)
{
    m.def(
        "enable_tracing",
        &Profiler::enable,
        cleanDoc(R"(
        Enable the profiler.

        Starts a tracing session that records events from the C++ core and
        from Python into a temporary trace file. Does nothing if the profiler
        is already enabled.
    )")
            .c_str());
    m.def(
        "disable_tracing",
        &Profiler::disable,
        cleanDoc(R"(
        Disable the profiler.

        The recorded trace is discarded; use :func:`dump_traces` to keep it.
    )")
            .c_str());
    m.def(
        "is_tracing_enabled",
        [] { return Profiler::instance().isEnabled(); },
        cleanDoc(R"(
        Check if the profiler is enabled.

        Returns:
            ``True`` while a tracing session is running.
        )")
            .c_str());
    // Hard-coded "Python" category for trace events
    m.def(
        "start_trace_event",
        [](const char* name) { TRACE_EVENT_BEGIN("Python", perfetto::DynamicString{name}); },
        py::arg("name"),
        cleanDoc(R"(
        Starts a named trace event.

        Close it with :func:`end_trace_event`; events nest.

        Args:
            name: Name of the trace event.
        )")
            .c_str());
    m.def(
        "end_trace_event",
        [] { TRACE_EVENT_END("Python"); },
        cleanDoc("Ends the last started trace event on the calling thread.").c_str());
    m.def(
        "dump_traces",
        &Profiler::dumpAndReset,
        py::arg("filename"),
        cleanDoc(R"(
        Dump traces to file.

        Stops the tracing session, saves the trace to ``filename`` and resets
        the profiler so a new session can be started with
        :func:`enable_tracing`. Writes nothing if tracing is not enabled. If
        the trace cannot be saved, the trace is lost and the failure is
        reported through the error callback (see :func:`set_error_callback`);
        without one the failure is silent.

        Args:
            filename: Path of the trace file to write.
        )")
            .c_str());
}
