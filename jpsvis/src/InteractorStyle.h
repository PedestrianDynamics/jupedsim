/**
* @headerfile InteractorStyle.h
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
*
* @brief  Catch some mouse and keyboard events and redirect them (vtk stuff)
*
*
*  Created on: Aug 18, 2009
*/


#ifndef INTERACTORSTYLE_H_
#define INTERACTORSTYLE_H_

// Extern variables;
extern bool extern_fullscreen_enable;
extern bool extern_force_system_update;

/// This defines the interactions  between the user and the visualisation window.

class InteractorStyle: public vtkInteractorStyleTrackballCamera {

public:
    InteractorStyle();
    virtual ~InteractorStyle();

    /// static constructor
    static  InteractorStyle* New();

    ///set the actor
    void SetActor(/*vtkActor* actor*/);
    ///set the source
    void SetSource(/*some source*/);

    ///override
    virtual void OnChar();
    virtual void Rotate();
    virtual void Spin();
    virtual void Pan();
    virtual void Dolly();
    virtual void OnLeftButtonUp();




private:
    //vtkActor* mActor ;
    //vtkSphereSource* mSrc ;
};

#endif /* INTERACTORSTYLE_H_ */
