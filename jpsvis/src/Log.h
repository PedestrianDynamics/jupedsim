/**
 * @headerfile Debug.h
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 1.0
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

#ifndef DEBUG_H_
#define DEBUG_H_

#include <fstream>

class Log
{
public:
    /**
     *  different debug levels
     */
    enum LEVEL {
        NONE,    //!< NONE The logger is switched off
        INFO,    //!< INFO debug messages only are output
        WARNING, //!< WARNING warning and Errors are output
        // CRITICAL,//!< CRITICAL
        ERROR, //!< ERROR Only errors are output
        ALL    //!< ALL Performs a full logging
    };

    /**
     * Constructor
     */
    Log();

    /**
     * Destructor
     */
    virtual ~Log();

    /**
     * set the debug output stream. can be a file as well
     * as one of the cout, cerr, clog, ...
     * Default output stream if std::cout.
     *
     * @param os, the output stream
     */
    static void setOutputStream(std::ostream & os);

    /**
     * set the desired debug level.
     *
     * @see LEVEL
     *
     * @param level, the desired debug level
     */
    static void setDebugLevel(Log::LEVEL level);

    static void Info(const char * string, ...);
    /**
     * send a message (information) to the output stream
     *
     * @param string, the message
     */
    static void Messages(const char * string, ...);

    /**
     * add a warning to the output stream
     *
     * @param string, the warning message
     */
    static void Warning(const char * string, ...);


    /**
     * add an error message to the output stream
     *
     * @param string, the error message
     */
    static void Error(const char * string, ...);

private:
    static std::ostream & os;
    static Log::LEVEL debugLevel;
    static int INFO_Count;
    static int ERR_Count;
    static int WAR_Count;
};

#endif /* DEBUG_H_ */
