// SPDX-License-Identifier: LGPL-3.0-or-later
#include "logger.hpp"

#include <string>
#include <vector>

namespace logging
{
namespace
{
/// Where messages logged on this thread are collected, if anywhere.
thread_local std::vector<Message>* collected = nullptr;
} // namespace

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::set_debug_callback(LogCallback&& cb)
{
    _debug_msg_cb = cb;
}

void Logger::clear_debug_callback()
{
    _debug_msg_cb = {};
}

void Logger::log_debug_message(const std::string& msg)
{
    if(collected != nullptr) {
        collected->push_back({Level::Debug, msg});
        return;
    }
    if(_debug_msg_cb) {
        _debug_msg_cb(msg);
    }
}

void Logger::set_info_callback(LogCallback&& cb)
{
    _info_msg_cb = cb;
}

void Logger::clear_info_callback()
{
    _info_msg_cb = {};
}

void Logger::log_info_message(const std::string& msg)
{
    if(collected != nullptr) {
        collected->push_back({Level::Info, msg});
        return;
    }
    if(_info_msg_cb) {
        _info_msg_cb(msg);
    }
}

void Logger::set_warning_callback(LogCallback&& cb)
{
    _warning_msg_cb = cb;
}

void Logger::clear_warning_callback()
{
    _warning_msg_cb = {};
}

void Logger::log_warning_message(const std::string& msg)
{
    if(collected != nullptr) {
        collected->push_back({Level::Warning, msg});
        return;
    }
    if(_warning_msg_cb) {
        _warning_msg_cb(msg);
    }
}

void Logger::set_error_callback(LogCallback&& cb)
{
    _error_msg_cb = cb;
}

void Logger::clear_error_callback()
{
    _error_msg_cb = {};
}

void Logger::log_error_message(const std::string& msg)
{
    if(collected != nullptr) {
        collected->push_back({Level::Error, msg});
        return;
    }
    if(_error_msg_cb) {
        _error_msg_cb(msg);
    }
}

void Logger::clear_all_callbacks()
{
    clear_debug_callback();
    clear_info_callback();
    clear_warning_callback();
    clear_error_callback();
}

void Logger::log(const Message& message)
{
    switch(message.level) {
        case Level::Debug:
            log_debug_message(message.text);
            break;
        case Level::Info:
            log_info_message(message.text);
            break;
        case Level::Warning:
            log_warning_message(message.text);
            break;
        case Level::Error:
            log_error_message(message.text);
            break;
        case Level::Off:
            break;
    }
}

Logger::Collect::Collect(std::vector<Message>& into) : _previous(collected)
{
    collected = &into;
}

Logger::Collect::~Collect()
{
    collected = _previous;
}
} // namespace logging
