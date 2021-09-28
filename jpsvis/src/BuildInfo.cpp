#include "BuildInfo.h"

std::string ver_string(int a, int b, int c)
{
    std::ostringstream ss;
    ss << a << '.' << b << '.' << c;
    return ss.str();
}
