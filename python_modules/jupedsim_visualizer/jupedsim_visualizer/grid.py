# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import math

from jupedsim_visualizer.config import Colors, ZLayers
from vtkmodules.vtkCommonCore import vtkCommand
from vtkmodules.vtkFiltersSources import vtkPlaneSource
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCamera,
    vtkPolyDataMapper,
    vtkRenderer,
)


class Grid:
    def __init__(self, renderer: vtkRenderer, cam: vtkCamera):
        cam.AddObserver(vtkCommand.ModifiedEvent, self.__on_camera_update)
        self.renderer = renderer
        self.cam_scale = cam.GetParallelScale()
        self.div_used = 1
        self.cam_pos = cam.GetPosition()
        self.plane_source = vtkPlaneSource()

        scale_used_y, self.div_used = self.__compute_scale(self.cam_scale)
        ap = self.renderer.GetAspect()
        aspect = ap[0] / ap[1]
        scale_used_x, _ = self.__compute_scale(self.cam_scale * aspect)
        self.plane_source.SetOrigin(-scale_used_x, -scale_used_y, ZLayers.grid)
        self.plane_source.SetResolution(
            2 * scale_used_x // self.div_used,
            2 * scale_used_y // self.div_used,
        )
        self.plane_source.SetPoint1(scale_used_x, -scale_used_y, ZLayers.grid)
        self.plane_source.SetPoint2(-scale_used_x, scale_used_y, ZLayers.grid)
        self.plane_source.Update()

        cp = (
            (self.cam_pos[0] // self.div_used) * self.div_used,
            (self.cam_pos[1] // self.div_used) * self.div_used,
        )
        self.plane_source.SetCenter(cp[0], cp[1], ZLayers.grid)
        self.plane_source.Update()
        plane = self.plane_source.GetOutput()
        mapper = vtkPolyDataMapper()
        mapper.SetInputData(plane)
        actor = vtkActor()
        actor.SetMapper(mapper)
        actor.GetProperty().SetColor(Colors.e)
        actor.GetProperty().SetRepresentationToWireframe()
        renderer.AddActor(actor)
        self._actor = actor

    def show(self, state: bool):
        self._actor.SetVisibility(state)
        self._actor.Modified()

    @staticmethod
    def __compute_scale(scale):
        pow2_scale = math.ceil(math.log2(scale))
        scale = int(2**pow2_scale)
        div = int(2 ** (max(1, pow2_scale - 4)))
        return (scale, div)

    def __on_camera_update(self, obj, evt):
        new_scale = obj.GetParallelScale()
        new_pos = obj.GetPosition()

        if new_scale != self.cam_scale:
            self.cam_scale = new_scale
            scale_used_y, self.div_used = self.__compute_scale(self.cam_scale)
            ap = self.renderer.GetAspect()
            aspect = ap[0] / ap[1]
            scale_used_x, _ = self.__compute_scale(self.cam_scale * aspect)
            self.plane_source.SetOrigin(
                -scale_used_x, -scale_used_y, ZLayers.grid
            )
            self.plane_source.SetResolution(
                2 * scale_used_x // self.div_used,
                2 * scale_used_y // self.div_used,
            )
            self.plane_source.SetPoint1(
                scale_used_x, -scale_used_y, ZLayers.grid
            )
            self.plane_source.SetPoint2(
                -scale_used_x, scale_used_y, ZLayers.grid
            )
            self.plane_source.Update()

        self.cam_pos = new_pos

        cp = (
            (self.cam_pos[0] // self.div_used) * self.div_used,
            (self.cam_pos[1] // self.div_used) * self.div_used,
        )
        self.plane_source.SetCenter(cp[0], cp[1], ZLayers.grid)
        self.plane_source.Update()
