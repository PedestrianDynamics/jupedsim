# SPDX-License-Identifier: LGPL-3.0-or-later
from jupedsim.internal.aabb import AABB
from jupedsim.recording import Recording, RecordingFrame
from vtkmodules.vtkCommonCore import vtkPoints
from vtkmodules.vtkCommonDataModel import vtkPolyData
from vtkmodules.vtkFiltersCore import vtkGlyph2D
from vtkmodules.vtkFiltersSources import vtkRegularPolygonSource
from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper

from jupedsim_visualizer.config import Colors, ZLayers

GS_SCALING_FACTOR = 0.26 / (2 * 0.3 * 1.65)


def to_vtk_points(frame: RecordingFrame) -> vtkPoints:
    points = vtkPoints()
    for agent in frame.agents:
        points.InsertNextPoint(
            agent.position[0], agent.position[1], ZLayers.agents
        )
    return points


def to_vtk_gs_points(frame: RecordingFrame) -> vtkPoints:
    points = vtkPoints()
    for agent in frame.agents:
        if agent.ground_support_position is not None:
            points.InsertNextPoint(
                agent.ground_support_position[0],
                agent.ground_support_position[1],
                ZLayers.agents,
            )
    return points


def build_link_polydata(frame: RecordingFrame) -> vtkPolyData:
    from vtkmodules.vtkCommonDataModel import vtkCellArray, vtkLine

    points = vtkPoints()
    lines = vtkCellArray()
    for agent in frame.agents:
        if agent.ground_support_position is not None:
            idx = points.InsertNextPoint(
                agent.position[0], agent.position[1], ZLayers.agents
            )
            idx2 = points.InsertNextPoint(
                agent.ground_support_position[0],
                agent.ground_support_position[1],
                ZLayers.agents,
            )
            line = vtkLine()
            line.GetPointIds().SetId(0, idx)
            line.GetPointIds().SetId(1, idx2)
            lines.InsertNextCell(line)
    polydata = vtkPolyData()
    polydata.SetPoints(points)
    polydata.SetLines(lines)
    return polydata


def clamp(value: int, min_value: int, max_value: int) -> int:
    return max(min_value, min(max_value, value))


class Trajectory:
    def __init__(self, rec: Recording) -> None:
        self.rec = rec
        self.current_index = 0
        self.num_frames = rec.num_frames
        self._has_ipp = rec.has_ipp_columns

        first_frame = rec.frame(0)

        # Upper body circles
        polydata = vtkPolyData()
        polydata.SetPoints(to_vtk_points(first_frame))
        self.polydata = polydata

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

        # IPP ground support circles and links
        self.gs_actor = None
        self.link_actor = None
        self.gs_glyph2D = None
        self.gs_polydata = None
        self._link_mapper = None

        if self._has_ipp:
            self._init_ipp(first_frame)

    def _init_ipp(self, first_frame: RecordingFrame):
        # Ground support circles (smaller, different color)
        gs_polydata = vtkPolyData()
        gs_polydata.SetPoints(to_vtk_gs_points(first_frame))
        self.gs_polydata = gs_polydata

        gs_radius = 0.15 * GS_SCALING_FACTOR * 1.65
        gs_polygon = vtkRegularPolygonSource()
        gs_polygon.SetRadius(gs_radius)
        gs_polygon.SetNumberOfSides(24)

        gs_glyph = vtkGlyph2D()
        gs_glyph.SetSourceConnection(gs_polygon.GetOutputPort())
        gs_glyph.SetInputData(gs_polydata)
        gs_glyph.Update()
        self.gs_glyph2D = gs_glyph

        gs_mapper = vtkPolyDataMapper()
        gs_mapper.SetInputConnection(gs_glyph.GetOutputPort())
        gs_mapper.Update()

        gs_actor = vtkActor()
        gs_actor.SetMapper(gs_mapper)
        gs_actor.GetProperty().SetColor(Colors.e)
        self.gs_actor = gs_actor

        # Connection lines — mapper swaps polydata each frame
        link_mapper = vtkPolyDataMapper()
        link_mapper.SetInputData(build_link_polydata(first_frame))
        self._link_mapper = link_mapper

        link_actor = vtkActor()
        link_actor.SetMapper(link_mapper)
        link_actor.GetProperty().SetColor(Colors.d)
        link_actor.GetProperty().SetLineWidth(1.5)
        self.link_actor = link_actor

    def get_actors(self) -> list[vtkActor]:
        actors = [self.actor]
        if self.gs_actor is not None:
            actors.append(self.gs_actor)
        if self.link_actor is not None:
            actors.append(self.link_actor)
        return actors

    def get_bounds(self) -> AABB:
        return self.rec.bounds()

    def _update_frame(self, frame: RecordingFrame):
        self.polydata.SetPoints(to_vtk_points(frame))
        self.glyph2D.Update()

        if self._has_ipp:
            self.gs_polydata.SetPoints(to_vtk_gs_points(frame))
            self.gs_glyph2D.Update()
            self._link_mapper.SetInputData(build_link_polydata(frame))

    def advance_frame(self, offset: int):
        self.current_index = clamp(
            self.current_index + offset, 0, self.num_frames - 1
        )
        self._update_frame(self.rec.frame(self.current_index))

    def goto_frame(self, index: int):
        self.current_index = clamp(index, 0, self.num_frames - 1)
        self._update_frame(self.rec.frame(self.current_index))
