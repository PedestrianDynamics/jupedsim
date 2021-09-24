/**
 * @file main.h
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.5
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * @brief This is what it does... :)
 *
 * Created on: 05.05.2009
 *
 */

/**
 * @mainpage
 *
 * \date 7.6.2010
 *
 * JuPedSim stands for Jülich Pedestrians Simulator and is currently
 * developed at the Forschungszentrum Jülich in Germany.
 *
 * @image html travisto.png " "
 *
 * Some useful links:
 *
 *       1: <a href="http://www.jupedsim.org">www.jupedsim.org</a> <br>
 *       2: <a href="http://www.vtk.org">www.vtk.org</a> <br>
 *       3: <a href="http://www.trolltech.com">www.trolltech.com</a> <br>
 *       4: <a href="http://www.fz-juelich.de">www.fz-juelich.de</a> <br>
 *
 */

#include "Log.h"
#include "MainWindow.h"

#include <QApplication>
#include <QDir>
#include <locale.h>
#include <sstream>

// for compiling a standalone windows exe with VS
#ifdef _MSC_VER
#ifdef NDEBUG
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#else
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif
#endif
std::string ver_string(int a, int b, int c)
{
    std::ostringstream ss;
    ss << a << '.' << b << '.' << c;
    return ss.str();
}

std::string true_cxx =
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

std::string true_cxx_ver =
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

int main(int argc, char * argv[])
{
    Log::Info("\n----\nJuPedSim - JPSvis\n");
    Log::Info("Current date   : %s %s", __DATE__, __TIME__);
    Log::Info("Version        : %s", JPSVIS_VERSION);
    Log::Info("Compiler       : %s (%s)", true_cxx.c_str(), true_cxx_ver.c_str());
    Log::Info("Commit hash    : %s", GIT_COMMIT_HASH);
    Log::Info("Commit date    : %s", GIT_COMMIT_DATE);
    Log::Info("Branch         : %s\n----\n", GIT_BRANCH);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication a(argc, argv);
    // To ensure that decimal numbers use a "point" instead of "colon" on some
    // systems
    // setlocale(LC_NUMERIC, "en_US" );
    setlocale(LC_NUMERIC, "C");

    // force the application to first looks for privated libs
    a.addLibraryPath(QApplication::applicationDirPath() + QDir::separator() + "lib");

    MainWindow w;
    w.show();
    return a.exec();
}
