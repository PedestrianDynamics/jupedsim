/**
* @file extern_var.h
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
* \brief   contains the declaration of all extern variables
*
*  Created on: 08.06.2009
*/

#ifndef EXTERN_VAR_H_
#define EXTERN_VAR_H_ 1

#include "SyncData.h"
#include "Pedestrian.h"

#include <vtkGlyph3D.h>
#include <vtkTensorGlyph.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>


#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//external variables
/// define the speed/rate/pace at which  the trajectories are displayed.
/// 1 is the normal default playing rate
int  extern_update_step=1;


///visualizing 2D or 3 D
/// true for 3D, false for 2D
bool extern_is_3D=true;

bool extern_shutdown_visual_thread=false;
bool extern_recording_enable=false;
bool extern_is_pause =false;
bool extern_launch_recording=false;
bool extern_offline_mode = true;
bool extern_fullscreen_enable = false;
bool extern_take_screenshot = false;
/// states whether a setting has been altered
/// and force the system to update
bool extern_force_system_update=false;

/// enables of disables tracking.
/// With this enable, moving pedestrians will leave a
/// trail behind them
bool extern_tracking_enable=false;

///relative scale from pedestrian to the geometry (environment)
double extern_scale=0.1;
double extern_scale_pedestrian=0.1;

// At most three pedestrians groups can be loaded
///The first pedestrian group
Pedestrian** extern_pedestrians_firstSet=NULL;
///The second pedestrian group
Pedestrian** extern_pedestrians_secondSet=NULL;
///The third pedestrian group
Pedestrian** extern_pedestrians_thirdSet=NULL;


VTK_CREATE (vtkGlyph3D, extern_glyphs_pedestrians);
//VTK_CREATE (vtkTensorGlyph, extern_glyphs_pedestrians);

// and here the corresponding dataset

///The first dataset
SyncData extern_trajectories_firstSet;
///The second dataset
SyncData extern_trajectories_secondSet;
///The third dataset
SyncData extern_trajectories_thirdSet;


//states if the datasets are loaded.
bool extern_first_dataset_loaded=false;
bool extern_second_dataset_loaded=false;
bool extern_third_dataset_loaded=false;

//states whether the loaded datasets are visible
bool extern_first_dataset_visible=false;
bool extern_second_dataset_visible=false;
bool extern_third_dataset_visible=false;


#endif /* EXTERN_VAR_H_ */
