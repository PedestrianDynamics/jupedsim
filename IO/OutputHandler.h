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
using namespace std;


#include "../IO/TraVisToClient.h"
#include "../general/Macros.h"

class OutputHandler {
public:
    virtual void write(string str);
    virtual ~OutputHandler(){};
};

class STDIOHandler : public OutputHandler {
public:
    void write(string str);
};

class FileHandler : public OutputHandler {
private:
    ofstream pfp;

public:
    FileHandler(const char *fn);
    virtual ~FileHandler();
    void write(string str);
};

class TraVisToHandler : public OutputHandler {
private:
    TraVisToClient* client;

public:
    TraVisToHandler();
    virtual ~TraVisToHandler();
    void write(string str);

    //Some tags are broken
    vector<string> brokentags;
};




#endif /*OUTPUT_HANDLER_H_*/

