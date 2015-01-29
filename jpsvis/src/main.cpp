/**
* @file main.h
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.5
* Copyright (C) <2009-2010>
*
* @section LICENSE
* This file is part of OpenPedSim.
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
* 	1: <a href="http://www.openpedsim.org">www.openpedsim.org</a> <br>
* 	2: <a href="http://www.vtk.org">www.vtk.org</a> <br>
* 	3: <a href="http://www.trolltech.com">www.trolltech.com</a> <br>
* 	4: <a href="http://www.fz-juelich.de">www.fz-juelich.de</a> <br>
* 	4: <a href="http://www.jupedsim.org">www.fz-juelich.de</a> <br>
*
*/

#undef VTK_USE_FFMPEG

#include "MainWindow.h"

#include <QApplication>
#include <QDir>
#include <locale.h>


#define vtkErrorMacro (x) ()

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // To ensure that decimal numbers use a "point" instead of "colon" on some systems
    //setlocale(LC_NUMERIC, "en_US" );
    setlocale(LC_NUMERIC,"C");

    //force the application to first looks for privated libs
    a.addLibraryPath(QApplication::applicationDirPath()
                     + QDir::separator()
                     + "lib");

    MainWindow w;
    w.show();
    return a.exec();
}
