#pragma once

#include "IO/OutputHandler.h"

#include <string>

class ToxicityOutputHandler : public FileHandler
{
public:
    ToxicityOutputHandler(const char *fn);
    ~ToxicityOutputHandler();
    void WriteToFileHeader();
    void WriteToFile(std::string& string);
    void WriteToFileFooter();
};
