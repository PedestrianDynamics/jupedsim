/**
 * @file:   OutputHandler.cpp
 *
 * @date Created on 20. November 2010, 15:20
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

#include "OutputHandler.h"

#include <stdio.h>
#include <stdarg.h>
#include <cstdlib>

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

void OutputHandler::Write(string str)
{
     if (this != NULL)
          cout << str << endl;
}

void OutputHandler::ProgressBar(double TotalPeds, double NowPeds)
{
     // based on this answer:
     // https://stackoverflow.com/questions/1637587/c-libcurl-console-progress-bar
     // how wide you want the progress meter to be
     int totaldotz=40;
     double fraction = NowPeds / TotalPeds;
     // part of the progressmeter that's already "full"
     int dotz = round(fraction * totaldotz);

     // create the "meter"
     int ii=0;
     printf("Evacuation: %3.0f%% [",fraction*100);
     // part  that's full already
     for ( ; ii < dotz; ii++) {
          printf("=");
     }
     printf(">");
     // remaining part (spaces)
     for ( ; ii < totaldotz;ii++) {
          printf(" ");
     }
     // and back to line begin - do not forget the fflush to avoid output buffering problems!
     printf("]\r");
     fflush(stdout);
}

void OutputHandler::Write(const char* message,...)
 {
    char msg[CLENGTH];
    va_list ap;
    va_start(ap, message);
    vsprintf(msg, message, ap);
    va_end(ap);

    string str(msg);

    if (str.find("ERROR") != string::npos)
    {
        cerr << msg << endl;
        cerr.flush();
        incrementErrors();
    }
    else if (str.find("WARNING") != string::npos)
    {
        cerr << msg << endl;
        cerr.flush();
        incrementWarnings();
    }
    else
    { // infos
        cout << msg << endl;
        cout.flush();
    }
}

void STDIOHandler::Write(string str)
{
    if (str.find("ERROR") != string::npos)
       {
           cerr << str << endl;
           cerr.flush();
           incrementErrors();
       }
       else if (str.find("WARNING") != string::npos)
       {
           cerr << str << endl;
           cerr.flush();
           incrementWarnings();
       }
       else
       { // infos
           cout << str << endl;
           cout.flush();
       }
}

FileHandler::FileHandler(const char *fn)
{
     _pfp.open(fn);
     if (!fn) {
          char tmp[CLENGTH];
          sprintf(tmp, "Error!!! File [%s] could not be opened!", fn);
          cerr << tmp << endl;
          exit(0);
     }
}

FileHandler::~FileHandler()
{
     _pfp.close();
}

void FileHandler::Write(string str)
{
     if (this != NULL) {
          _pfp << str << endl;
          _pfp.flush();
     }
}

void FileHandler::Write(const char* string,...)
{
     char msg[CLENGTH];
     va_list ap;
     va_start (ap, string);
     vsprintf (msg,string ,ap);
     va_end (ap);
     _pfp<<msg<<endl;
     _pfp.flush();
}

TraVisToHandler::TraVisToHandler(string host, int port)
{
     client = new TraVisToClient(host, port);
     brokentags.push_back("<trajectories>");
     brokentags.push_back("</trajectories>");
     brokentags.push_back("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
}

TraVisToHandler::~TraVisToHandler()
{
     delete client;
}

void TraVisToHandler::Write(string str)
{

     vector<string>::iterator str_it;

     //There are a few broken tags which need to be checked for and removed.
     for (str_it = brokentags.begin(); str_it != brokentags.end(); ++str_it) {
          int tagstart = str.find(*str_it);
          if (tagstart != (int) string::npos) {
               str.erase(str.begin() + tagstart, str.begin() + tagstart + (*str_it).size());
          }
     }
     client->sendData(str.c_str());
}
