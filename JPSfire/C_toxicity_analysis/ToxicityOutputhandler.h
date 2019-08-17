#pragma once

#include "general/Filesystem.h"
#include "IO/OutputHandler.h"

#include <string>

class ToxicityOutputHandler : public FileHandler
{
public:
    ToxicityOutputHandler(const fs::path& file);
    ~ToxicityOutputHandler() override;
    void WriteToFileHeader();
    void WriteToFile(const std::string& string);
    void WriteToFileFooter();
};
