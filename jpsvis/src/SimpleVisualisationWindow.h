/**
* @headerfile SimpleVisualisationWindow.cpp
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.1
* Copyright (C) <2009-2010>
*
* @section LICENSE
* This file is part of OpenPedSim.
*
* OpenPedSim is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* OpenPedSim is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
*
* @section DESCRIPTION
*  This is a very simple visualisation windows for
* quick visualisation
*
* @brief testing class
*
*
*  Created on: 23.07.2009
*
*/


#ifndef SIMPLEVISUALISATIONWINDOW_H_
#define SIMPLEVISUALISATIONWINDOW_H_

//forwarded classes
class vtkAssembly;
class vtkActor;



class SimpleVisualisationWindow {
public:
    ///constructor
    SimpleVisualisationWindow();
    ///destructor
    virtual ~SimpleVisualisationWindow();

    ///add an assembly actor
    void setActor(vtkAssembly* actor);

//	///add a simple (single) actor
//	void setActor(vtkActor* actor);

    ///start the visualisation
    //with the defaults parameters
    void start();

private:
    vtkAssembly* assembly;
    //vtkActor* actor;
};

#endif /* SIMPLEVISUALISATIONWINDOW_H_ */
