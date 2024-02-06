# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
from jupedsim_visualizer.config import Colors, ZLayers
from vtkmodules.vtkCommonCore import vtkPoints
from vtkmodules.vtkCommonDataModel import vtkPolyData
from vtkmodules.vtkFiltersCore import vtkGlyph2D
from vtkmodules.vtkFiltersSources import vtkRegularPolygonSource
from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper

from jupedsim.internal.aabb import AABB
from jupedsim.recording import Recording, RecordingFrame


def to_vtk_points(frame: RecordingFrame) -> vtkPoints:
    points = vtkPoints()
    for agent in frame.agents:
        points.InsertNextPoint(
            agent.position[0], agent.position[1], ZLayers.agents
        )
    return points


def clamp(value: int, min_value: int, max_value: int) -> int:
    return max(min_value, min(max_value, value))


class Trajectory:
    def __init__(self, rec: Recording) -> None:
        self.rec = rec
        self.current_index = 0
        self.num_frames = rec.num_frames
        polydata = vtkPolyData()

        polydata.SetPoints(to_vtk_points(rec.frame(0)))
        self.polydata = polydata

        # Create anything you want here, we will use a polygon for the demo.
        polygonSource = vtkRegularPolygonSource()
        polygonSource.SetRadius(0.15)
        polygonSource.SetNumberOfSides(24)

        glyph2D = vtkGlyph2D()
        glyph2D.SetSourceConnection(polygonSource.GetOutputPort())
        glyph2D.SetInputData(polydata)
        glyph2D.Update()
        self.glyph2D = glyph2D

        mapper = vtkPolyDataMapper()
        mapper.SetInputConnection(glyph2D.GetOutputPort())
        mapper.Update()

        actor = vtkActor()
        actor.SetMapper(mapper)
        actor.GetProperty().SetColor(Colors.agent)
        self.actor = actor

    def get_actors(self) -> list[vtkActor]:
        return [self.actor]

    def get_bounds(self) -> AABB:
        return self.rec.bounds()

    def advance_frame(self, offset: int):
        self.current_index = clamp(
            self.current_index + offset, 0, self.num_frames - 1
        )
        self.polydata.SetPoints(
            to_vtk_points(self.rec.frame(self.current_index))
        )
        self.glyph2D.Update()

    def goto_frame(self, index: int):
        self.current_index = clamp(index, 0, self.num_frames - 1)
        self.polydata.SetPoints(
            to_vtk_points(self.rec.frame(self.current_index))
        )
        self.glyph2D.Update()
