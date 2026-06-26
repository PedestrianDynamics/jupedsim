#include "Tracing.hpp"

#include "Logger.hpp"

#include <perfetto.h>

#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

namespace
{
std::string makeTempTracePath()
{
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    std::ostringstream name;
    name << "jupedsim_trace_" << ts << ".pftrace";
    return (std::filesystem::temp_directory_path() / name.str()).string();
}

perfetto::TraceConfig buildDefaultTraceConfig(const std::string& output_path)
{
    perfetto::TraceConfig cfg;

    auto* buffer = cfg.add_buffers();
    buffer->set_size_kb(8192);

    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");

    cfg.set_write_into_file(true);
    cfg.set_output_path(output_path);
    // How often accumulated trace data is written from the in-memory ring buffer
    // to the file. Lower values reduce memory pressure; higher values reduce I/O.
    cfg.set_file_write_period_ms(100);
    // Maximum time to wait for a flush to complete before giving up.
    cfg.set_flush_timeout_ms(5000);

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

    temp_trace_path = makeTempTracePath();

    tracing_session = perfetto::Tracing::NewTrace();
    tracing_session->Setup(buildDefaultTraceConfig(temp_trace_path));
    tracing_session->StartBlocking();
}

void Profiler::writeAndResetSession(const std::string& filename)
{
    if(!tracing_session) {
        return;
    }

    perfetto::TrackEvent::Flush();
    tracing_session->StopBlocking();
    tracing_session.reset();

    if(!temp_trace_path.empty()) {
        std::error_code ec;
        if(filename.empty()) {
            std::filesystem::remove(temp_trace_path, ec);
        } else {
            std::filesystem::rename(temp_trace_path, filename, ec);
            if(ec) {
                // rename fails across devices — copy then delete
                std::filesystem::copy_file(
                    temp_trace_path,
                    filename,
                    std::filesystem::copy_options::overwrite_existing,
                    ec);
                std::filesystem::remove(temp_trace_path);
                if(ec) {
                    LOG_ERROR("Failed to save Perfetto trace to: {}", filename);
                }
            }
        }
        temp_trace_path.clear();
    }
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
