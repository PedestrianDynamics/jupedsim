/**
 * \file        OutputHandler.h
 * \date        Nov 20, 2010
 * \version     v0.5
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/


#ifndef OUTPUT_HANDLER_H_
#define OUTPUT_HANDLER_H_

#include "../general/Macros.h"

#include <fstream>
#include <iostream>
#include <vector>

#ifdef _SIMULATOR
#include "../IO/TraVisToClient.h"
#endif


class OutputHandler
{
protected:
    int _nWarnings;
    int _nErrors;

public:
    OutputHandler()
    {
        _nWarnings = 0;
        _nErrors   = 0;
    };
    virtual ~OutputHandler(){};

    int GetWarnings();
    void incrementWarnings();
    int GetErrors();
    void incrementErrors();
    void ProgressBar(double TotalPeds, double NowPeds);

    virtual void Write(const std::string & str);
    virtual void Write(const char * string, ...);
};

class STDIOHandler : public OutputHandler
{
public:
    void Write(const std::string & str);
    void Write(const char * string, ...);
};

class FileHandler : public OutputHandler
{
private:
    std::ofstream _pfp;

public:
    FileHandler(const char * fn);
    virtual ~FileHandler();
    void Write(const std::string & str);
    void Write(const char * string, ...);
};

#ifdef _SIMULATOR

class SocketHandler : public OutputHandler
{
private:
    TraVisToClient * client;

public:
    SocketHandler(const std::string & host, int port);
    virtual ~SocketHandler();
    void Write(const std::string & str);

    //Some tags are broken
    std::vector<std::string> brokentags;
};

#endif

#endif /*OUTPUT_HANDLER_H_*/
