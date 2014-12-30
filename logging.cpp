#include <iostream>
#include "logging.h"

#ifdef  TRACE_LOGGING
void _printDebugLine(const std::string& fileName, int lineNumber)
{
unsigned found = fileName.find_last_of("/\\");
std::cerr  << "["<< lineNumber  << "]: ---"<< fileName.substr(found+1)<< " ---"<<std::endl;
}
#endif












