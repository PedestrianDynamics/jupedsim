/**
 * File:   OutputHandler.h
 *
 * Created on 20. November 2010, 15:20
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#ifndef OUTPUT_HANDLER_H_
#define OUTPUT_HANDLER_H_

#include <iostream>
#include <fstream>
#include <vector>


#include "../IO/TraVisToClient.h"
#include "../general/Macros.h"

class OutputHandler {
public:
    virtual void Write(std::string str);
	virtual void Write(const char *string, ...);
    virtual ~OutputHandler(){};
};

class STDIOHandler : public OutputHandler {
public:
    void Write(std::string str);
};

class FileHandler : public OutputHandler {
private:
    std::ofstream pfp;

public:
    FileHandler(const char *fn);
    virtual ~FileHandler();
    void Write(std::string str);
    void Write(const char *string,...);
};

class TraVisToHandler : public OutputHandler {
private:
    TraVisToClient* client;

public:
    TraVisToHandler(std::string host, int port);
    virtual ~TraVisToHandler();
    void Write(std::string str);

    //Some tags are broken
    std::vector<std::string> brokentags;
};




#endif /*OUTPUT_HANDLER_H_*/

