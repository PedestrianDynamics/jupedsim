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


// we don't need this when compiling with cmake
// http://www.vtk.org/Wiki/VTK/VTK_6_Migration/Factories_now_require_defines
/* #define vtkRenderingCore_AUTOINIT
 * 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL) */
/* #define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL) */

#include "Pedestrian.h"
#include "SyncData.h"
#include "TrailPlotter.h"
#include "train.h"

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTensorGlyph.h>
#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// external variables
/// define the speed/rate/pace at which  the trajectories are displayed.
/// 1 is the normal default playing rate
int extern_update_step = 1;


/// visualizing 2D or 3 D
/// true for 3D, false for 2D
bool extern_is_3D = true;

bool extern_shutdown_visual_thread = false;
bool extern_recording_enable       = false;
bool extern_is_pause               = false;
bool extern_launch_recording       = false;
bool extern_fullscreen_enable      = false;
bool extern_take_screenshot        = false;
/// states whether a setting has been altered
/// and force the system to update
bool extern_force_system_update = false;

/// enables of disables tracking.
/// With this enable, moving pedestrians will leave a
/// trail behind them
PointPlotter * extern_trail_plotter = NULL;

/// relative scale from pedestrian to the geometry (environment)
double extern_scale            = 0.1;
double extern_scale_pedestrian = 0.1;

// At most three pedestrians groups can be loaded
/// The first pedestrian group
Pedestrian ** extern_pedestrians_firstSet        = NULL;
vtkSmartPointer<vtkSphereSource> extern_mysphere = nullptr;
std::map<std::string, std::shared_ptr<TrainType>> extern_trainTypes;
std::map<int, std::shared_ptr<TrainTimeTable>> extern_trainTimeTables;


// vtkSmartPointer<vtkTensorGlyph> extern_glyphs_pedestrians=NULL;
// vtkSmartPointer<vtkTensorGlyph> extern_glyphs_pedestrians_3D=NULL;

vtkTensorGlyph * extern_glyphs_pedestrians    = NULL;
vtkTensorGlyph * extern_glyphs_pedestrians_3D = NULL;
vtkTensorGlyph * extern_glyphs_directions     = NULL;
vtkActor2D * extern_pedestrians_labels        = NULL;
vtkActor * extern_glyphs_pedestrians_actor_2D = NULL;
vtkActor * extern_glyphs_pedestrians_actor_3D = NULL;
vtkActor * extern_glyphs_directions_actor     = NULL;

// VTK_CREATE (vtkTensorGlyph, extern_glyphs_pedestrians);
// VTK_CREATE (vtkTensorGlyph, extern_glyphs_pedestrians_3D);
// VTK_CREATE (vtkActor2D, extern_pedestrians_labels);
// VTK_CREATE (vtkActor, extern_glyphs_pedestrians_actor_2D);
// VTK_CREATE (vtkActor, extern_glyphs_pedestrians_actor_3D);


// and here the corresponding dataset
/// The first dataset
SyncData extern_trajectories_firstSet;

// states if the datasets are loaded.
bool extern_first_dataset_loaded = false;

// states whether the loaded datasets are visible
bool extern_first_dataset_visible = false;


#endif /* EXTERN_VAR_H_ */
