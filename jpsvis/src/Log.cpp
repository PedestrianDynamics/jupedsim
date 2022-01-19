/**
 * @file Debug.cpp
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * \brief This class defines a debug output with different debug levels (similar to python).
 * The message sent to the output are printed or not depending on the debug level set.
 *
 *
 *  Created on: 31.04.2010
 *
 */


#include "Log.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

using namespace std;

std::ostream & Log::os     = std::cerr;
Log::LEVEL Log::debugLevel = Log::ALL;
int Log::INFO_Count        = 0;
int Log::ERR_Count         = 0;
int Log::WAR_Count         = 0;

Log::Log() {}

Log::~Log() {}


void Log::setOutputStream(std::ostream & osl)
{
    os.rdbuf(osl.rdbuf());
}

void Log::setDebugLevel(Log::LEVEL level)
{
    debugLevel = level;
}

void Log::Messages(const char * format, ...)
{
    switch(debugLevel) {
        case ALL:
        case INFO: {
            char msg[256];
            va_list ap;
            va_start(ap, format);
            vsprintf(msg, format, ap);
            va_end(ap);

            os << msg << endl;
        } break;

        case ERROR:
        case WARNING:
        case NONE:
            break;
    }
}

void Log::Info(const char * format, ...)
{
    switch(debugLevel) {
        case ALL:
        case INFO: {
            INFO_Count++;
            char msg[256];
            va_list ap;
            va_start(ap, format);
            vsprintf(msg, format, ap);
            va_end(ap);

            os << "Info [" << std::setw(3) << INFO_Count << "]: " << msg << endl;
        } break;

        case ERROR:
        case WARNING:
        case NONE:
            break;
    }
}

void Log::Warning(const char * format, ...)
{
    switch(debugLevel) {
        case WARNING:
        case ALL: {
            WAR_Count++;
            char msg[256];
            va_list ap;
            va_start(ap, format);
            vsprintf(msg, format, ap);
            va_end(ap);
            os << "Warning [" << std::setw(3) << WAR_Count << "]: " << msg << endl;
        } break;

        case ERROR:
        case INFO:
        case NONE:
            break;
    }
}


void Log::Error(const char * format, ...)
{
    switch(debugLevel) {
        case WARNING:
        case ERROR:
        case ALL: {
            ERR_Count++;
            char msg[256];
            va_list ap;
            va_start(ap, format);
            vsprintf(msg, format, ap);
            va_end(ap);
            os << "Error [" << std::setw(3) << ERR_Count << "]: " << msg << endl;
        } break;

        case INFO:
        case NONE:
            break;
    }
}
