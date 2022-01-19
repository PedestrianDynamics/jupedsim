#include "string_utils.h"

#include <algorithm>

std::string & trim_left(std::string & str)
{
    str.erase(std::begin(str), std::find_if(std::begin(str), std::end(str), [](const auto ch) {
                  return !std::isspace(ch);
              }));
    return str;
}

std::string & trim_right(std::string & str)
{
    str.erase(
        std::find_if(str.rbegin(), std::rend(str), [](const auto ch) { return !std::isspace(ch); })
            .base(),
        std::end(str));
    return str;
}

std::string & trim(std::string & str)
{
    return trim_right(trim_left(str));
}
