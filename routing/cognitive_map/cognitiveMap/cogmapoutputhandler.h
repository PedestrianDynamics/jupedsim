#ifndef COGMAPOUTPUTHANDLER_H
#define COGMAPOUTPUTHANDLER_H

#include "../../../IO/OutputHandler.h"
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

#endif // COGMAPOUTPUTHANDLER_H
