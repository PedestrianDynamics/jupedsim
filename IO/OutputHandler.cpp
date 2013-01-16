/**
 * File:   OutputHandler.cpp
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

#include "OutputHandler.h"

#include <cstdlib>
#include <cstdio>
#include <cstdarg>

using namespace std;


void OutputHandler::Write(string str) {
	if (this != NULL)
		cout << str << endl;
}

void OutputHandler::Write(const char* string,...) {
	char msg[CLENGTH];
	va_list ap;
	va_start (ap, string);
	vsprintf (msg,string ,ap);
	va_end (ap);
	cout<<msg<<endl;
}

void STDIOHandler::Write(string str) {
	if (this != NULL)
		cout << str << endl;
}

FileHandler::FileHandler(const char *fn) {
    pfp.open(fn);
    if (!fn) {
        char tmp[CLENGTH];
        sprintf(tmp, "Error!!! File [%s] could not be opened!", fn);
        cerr << tmp << endl;
        exit(0);
    }
}

FileHandler::~FileHandler() {
    pfp.close();
}

void FileHandler::Write(string str) {
    if (this != NULL) {
        pfp << str << endl;
        pfp.flush();
    }
}

void FileHandler::Write(const char* string,...) {
	char msg[CLENGTH];
	va_list ap;
	va_start (ap, string);
	vsprintf (msg,string ,ap);
	va_end (ap);
	pfp<<msg<<endl;
	pfp.flush();
}

