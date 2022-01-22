#pragma once
#include <string>
/// This file contains a few common string operations that are not present in the stl.

/// Trim string from left in place.
/// @param str, the string to trim in place
std::string & trim_left(std::string & str);

/// Trim string from right in place.
/// @param str, the string to trim in place
std::string & trim_right(std::string & str);

/// Trim string from both ends in place
/// @param str, the string to trim in place
std::string & trim(std::string & str);
