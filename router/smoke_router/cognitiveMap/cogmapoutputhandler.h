#pragma once

#include "IO/OutputHandler.h"

#include <string>

class CogMapOutputHandler : public FileHandler
{
public:
    CogMapOutputHandler(const char *fn);
    ~CogMapOutputHandler();
    void WriteToFileHeader(const int &id, const double &fps);
    void WriteToFile(std::string& string);
    void WriteToFileFooter();
};
