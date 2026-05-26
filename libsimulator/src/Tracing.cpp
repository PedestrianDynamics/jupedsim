#include "Tracing.hpp"

#include "Logger.hpp"

#include <perfetto.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

namespace
{
perfetto::TraceConfig buildDefaultTraceConfig()
{
    perfetto::TraceConfig cfg;

    auto* buffer = cfg.add_buffers();
    buffer->set_size_kb(8192);

    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");

    return cfg;
}
} // namespace

void Profiler::createSession()
{
    if(tracing_session) {
        return;
    }

    if(!perfetto::Tracing::IsInitialized()) {
        perfetto::TracingInitArgs args;
        args.backends |= perfetto::kInProcessBackend;
        perfetto::Tracing::Initialize(args);
    }

    perfetto::TrackEvent::Register();

    tracing_session = perfetto::Tracing::NewTrace();
    tracing_session->Setup(buildDefaultTraceConfig());
    tracing_session->StartBlocking();
}

void Profiler::writeAndResetSession(const std::string& filename)
{
    if(!tracing_session) {
        return;
    }

    perfetto::TrackEvent::Flush();
    tracing_session->StopBlocking();
    const auto trace_data = tracing_session->ReadTraceBlocking();
    if(filename.empty()) {
        tracing_session.reset();
        return;
    }
    std::ofstream output(filename, std::ios::binary | std::ios::trunc);
    if(output.is_open()) {
        output.write(trace_data.data(), static_cast<std::streamsize>(trace_data.size()));
        output.flush();
    } else {
        LOG_ERROR("Failed to open Perfetto output file: {}", filename);
    }

    tracing_session.reset();
}

void Profiler::enable()
{
    auto& instance = Profiler::instance();
    if(instance.enabled) {
        return;
    }

    instance.createSession();
    instance.enabled = true;
}

void Profiler::disable()
{
    auto& instance = Profiler::instance();
    if(!instance.enabled && !instance.tracing_session) {
        return;
    }

    instance.writeAndResetSession("");
    instance.enabled = false;
}

void Profiler::dumpAndReset(const std::string& filename)
{
    auto& instance = Profiler::instance();
    instance.writeAndResetSession(filename);
    instance.enabled = false;
}

Profiler Profiler::profiler{};