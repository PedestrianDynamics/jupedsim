#pragma once

#include <sstream>
#include <string>

std::string ver_string(int a, int b, int c);

const std::string true_cxx =
#ifdef __clang__
    "clang++";
#elif defined(__GNUC__)
    "g++";
#elif defined(__MINGW32__)
    "MinGW";
#elif defined(_MSC_VER)
    "Visual Studio";
#else
    "Compiler not identified";
#endif

const std::string true_cxx_ver =
#ifdef __clang__
    ver_string(__clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
    ver_string(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__MINGW32__)
    ver_string(__MINGW32__, __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
#elif defined(_MSC_VER)
    ver_string(_MSC_VER, _MSC_FULL_VER, _MSC_BUILD);
#else
    "";
#endif
