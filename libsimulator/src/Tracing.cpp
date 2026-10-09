#include "Tracing.hpp"

#include "Logger.hpp"

#include <fmt/core.h>
#include <perfetto.h>

#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

namespace
{
std::string make_temp_trace_path()
{
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string name = fmt::format("jupedsim_trace{}.pftrace", ts);
    return (std::filesystem::temp_directory_path() / name).string();
}

perfetto::TraceConfig build_default_trace_config(const std::string& output_path)
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

void Profiler::create_session()
{
    if(_tracing_session) {
        return;
    }

    if(!perfetto::Tracing::IsInitialized()) {
        perfetto::TracingInitArgs args;
        args.backends |= perfetto::kInProcessBackend;
        perfetto::Tracing::Initialize(args);
    }

    perfetto::TrackEvent::Register();

    _temp_trace_path = make_temp_trace_path();

    _tracing_session = perfetto::Tracing::NewTrace();
    _tracing_session->Setup(build_default_trace_config(_temp_trace_path));
    _tracing_session->StartBlocking();
}

void Profiler::write_and_reset_session(const std::string& filename)
{
    if(!_tracing_session) {
        return;
    }

    perfetto::TrackEvent::Flush();
    _tracing_session->StopBlocking();
    _tracing_session.reset();

    if(!_temp_trace_path.empty()) {
        std::error_code ec;
        if(filename.empty()) {
            std::filesystem::remove(_temp_trace_path, ec);
        } else {
            std::filesystem::rename(_temp_trace_path, filename, ec);
            if(ec) {
                // rename fails across devices — copy then delete
                std::filesystem::copy_file(
                    _temp_trace_path,
                    filename,
                    std::filesystem::copy_options::overwrite_existing,
                    ec);
                std::filesystem::remove(_temp_trace_path);
                if(ec) {
                    LOG_ERROR("Failed to save Perfetto trace to: {}", filename);
                }
            }
        }
        _temp_trace_path.clear();
    }
}

void Profiler::enable()
{
    auto& instance = Profiler::instance();
    if(instance._enabled) {
        return;
    }

    instance.create_session();
    instance._enabled = true;
}

void Profiler::disable()
{
    auto& instance = Profiler::instance();
    if(!instance._enabled && !instance._tracing_session) {
        return;
    }

    instance.write_and_reset_session("");
    instance._enabled = false;
}

void Profiler::dump_and_reset(const std::string& filename)
{
    auto& instance = Profiler::instance();
    instance.write_and_reset_session(filename);
    instance._enabled = false;
}

Profiler Profiler::profiler{};
