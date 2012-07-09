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
    ~FileHandler();
    void write(string str);
};

class TraVisToHandler : public OutputHandler {
private:
    TraVisToClient* client;

public:
    TraVisToHandler();
    ~TraVisToHandler();
    void write(string str);

    //Some tags are broken
    vector<string> brokentags;
};




#endif /*OUTPUT_HANDLER_H_*/

