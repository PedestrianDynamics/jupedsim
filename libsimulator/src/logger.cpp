// SPDX-License-Identifier: LGPL-3.0-or-later
#include "logger.hpp"

#include <string>

namespace logging
{

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

} // namespace logging
