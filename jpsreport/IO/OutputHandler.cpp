/**
 * \file        OutputHandler.cpp
 * \date        Nov 20, 2010
 * \version     v0.8
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


#include "OutputHandler.h"

#include <cmath>
#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>

using namespace std;

void OutputHandler::incrementWarnings()
{
    _nWarnings += 1;
}

int OutputHandler::GetWarnings()
{
    return _nWarnings;
}

void OutputHandler::incrementErrors()
{
    _nErrors += 1;
}

int OutputHandler::GetErrors()
{
    return _nErrors;
}

void OutputHandler::Write(const string & str)
{
    cout << str << endl;
}

void OutputHandler::ProgressBar(double TotalPeds, double NowPeds)
{
    // based on this answer:
    // https://stackoverflow.com/questions/1637587/c-libcurl-console-progress-bar
    // how wide you want the progress meter to be
    int totaldotz   = 40;
    double fraction = NowPeds / TotalPeds;
    // part of the progressmeter that's already "full"
    int dotz = round(fraction * totaldotz);

    // create the "meter"
    int ii = 0;
    printf("Evacuation: %3.0f%% [", fraction * 100);
    // part  that's full already
    for(; ii < dotz; ii++) {
        printf("=");
    }
    printf(">");
    // remaining part (spaces)
    for(; ii < totaldotz; ii++) {
        printf(" ");
    }
    // and back to line begin - do not forget the fflush to avoid output buffering problems!
    printf("]\r");
    fflush(stdout);
}

void OutputHandler::Write(const char * message, ...)
{
    char msg[CLENGTH] = "";
    va_list ap;
    va_start(ap, message);
    vsprintf(msg, message, ap);
    va_end(ap);

    string str(msg);

    if(str.find("ERROR") != string::npos) {
        cerr << msg << endl;
        cerr.flush();
        incrementErrors();
    } else if(str.find("WARNING") != string::npos) {
        cerr << msg << endl;
        cerr.flush();
        incrementWarnings();
    } else { // infos
        cout << msg << endl;
        cout.flush();
    }
}

void STDIOHandler::Write(const char * message, ...)
{
    char msg[CLENGTH] = "";
    va_list ap;
    va_start(ap, message);
    vsprintf(msg, message, ap);
    va_end(ap);

    string str(msg);

    if(str.find("ERROR") != string::npos) {
        cerr << msg << endl;
        cerr.flush();
        incrementErrors();
    } else if(str.find("WARNING") != string::npos) {
        cerr << msg << endl;
        cerr.flush();
        incrementWarnings();
    } else { // infos
        cout << msg << endl;
        cout.flush();
    }
}

void STDIOHandler::Write(const string & str)
{
    if(str.find("ERROR") != string::npos) {
        cerr << str << endl;
        cerr.flush();
        incrementErrors();
    } else if(str.find("WARNING") != string::npos) {
        cerr << str << endl;
        cerr.flush();
        incrementWarnings();
    } else { // infos
        cout << str << endl;
        cout.flush();
    }
}

FileHandler::FileHandler(const char * fn)
{
    if(fn == nullptr) {
        char tmp[CLENGTH];
        sprintf(tmp, "Error!!! File could not be opened!");
        cerr << tmp << endl;
        exit(0);
    } else {
        _pfp.open(fn);
    }
}

FileHandler::~FileHandler()
{
    _pfp.close();
}

void FileHandler::Write(const string & str)
{
    _pfp << str << endl;
    _pfp.flush();

    if(str.find("ERROR") != string::npos) {
        incrementErrors();
    } else if(str.find("WARNING") != string::npos) {
        incrementWarnings();
    }
}

void FileHandler::Write(const char * str_msg, ...)
{
    char msg[CLENGTH] = "";
    va_list ap;
    va_start(ap, str_msg);
    vsprintf(msg, str_msg, ap);
    va_end(ap);
    _pfp << msg << endl;
    _pfp.flush();

    string str(msg);
    if(str.find("ERROR") != string::npos) {
        incrementErrors();
    } else if(str.find("WARNING") != string::npos) {
        incrementWarnings();
    }
}

#ifdef _SIMULATOR

SocketHandler::SocketHandler(const string & host, int port)
{
    client = new TraVisToClient(host, port);
    brokentags.push_back("<trajectories>");
    brokentags.push_back("</trajectories>");
    brokentags.push_back("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
}

SocketHandler::~SocketHandler()
{
    delete client;
}

void SocketHandler::Write(const string & stringRef)
{
    vector<string>::iterator str_it;
    string str = stringRef;

    //There are a few broken tags which need to be checked for and removed.
    for(str_it = brokentags.begin(); str_it != brokentags.end(); ++str_it) {
        int tagstart = str.find(*str_it);
        if(tagstart != (int) string::npos) {
            str.erase(str.begin() + tagstart, str.begin() + tagstart + (*str_it).size());
        }
    }
    client->sendData(str.c_str());
}

#endif
