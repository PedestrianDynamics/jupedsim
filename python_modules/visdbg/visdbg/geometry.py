import sys

import py_jupedsim.experimental as jpex
from PySide6.QtCore import QObject, Signal
from visdbg.config import Colors, ZLayers
from vtkmodules.vtkCommonCore import vtkCommand, vtkIntArray, vtkPoints
from vtkmodules.vtkCommonDataModel import (
    vtkCellArray,
    vtkLine,
    vtkPolyData,
    vtkTriangle,
)
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleUser
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCellPicker,
    vtkPolyDataMapper,
    vtkRenderer,
)


class Geometry:
    def __init__(self, navi: jpex.RoutingEngine):
        self.navi = navi
        triangle_points = vtkPoints()
        graph_edge_points = vtkPoints()
        triangles = vtkCellArray()
        graph_edges = vtkCellArray()
        triangle_data = vtkIntArray()

        for idx, tri in enumerate(self.navi.mesh()):
            triangle_points.InsertNextPoint(tri[0][0], tri[0][1], ZLayers.geo)
            triangle_points.InsertNextPoint(tri[1][0], tri[1][1], ZLayers.geo)
            triangle_points.InsertNextPoint(tri[2][0], tri[2][1], ZLayers.geo)
            triangle = vtkTriangle()
            triangle.GetPointIds().SetId(0, idx * 3)
            triangle.GetPointIds().SetId(1, idx * 3 + 1)
            triangle.GetPointIds().SetId(2, idx * 3 + 2)
            triangles.InsertNextCell(triangle)
            triangle_data.InsertNextValue(idx)

            for edge_from, edge_to in self.navi.edges_for(idx):
                if edge_from == edge_to:
                    continue
                from_idx = graph_edge_points.InsertNextPoint(
                    edge_from[0], edge_from[1], ZLayers.graph_edges
                )
                to_idx = graph_edge_points.InsertNextPoint(
                    edge_to[0], edge_to[1], ZLayers.graph_edges
                )
                line = vtkLine()
                line.GetPointIds().SetId(0, from_idx)
                line.GetPointIds().SetId(1, to_idx)
                graph_edges.InsertNextCell(line)

        triangle_poly_data = vtkPolyData()
        triangle_poly_data.SetPoints(triangle_points)
        triangle_poly_data.SetPolys(triangles)
        triangle_poly_data.GetCellData().AddArray(triangle_data)

        triangle_mapper = vtkPolyDataMapper()
        triangle_mapper.SetInputData(triangle_poly_data)

        edges_poly_data = vtkPolyData()
        edges_poly_data.SetPoints(graph_edge_points)
        edges_poly_data.SetLines(graph_edges)
        edge_poly_mapper = vtkPolyDataMapper()
        edge_poly_mapper.SetInputData(edges_poly_data)

        actor = vtkActor()
        actor.SetMapper(triangle_mapper)
        actor.GetProperty().SetColor(Colors.c)
        actor.GetProperty().EdgeVisibilityOn()
        actor.GetProperty().SetEdgeColor(Colors.a)
        self.actor = actor

        edge_actor = vtkActor()
        edge_actor.SetMapper(edge_poly_mapper)
        edge_actor.GetProperty().SetColor(0, 0, 1)
        edge_actor.GetProperty().EdgeVisibilityOn()
        edge_actor.GetProperty().SetLineWidth(3)
        self.edge_actor = edge_actor

    def get_actors(self):
        return [self.edge_actor, self.actor]

    def get_bounds(self):
        xmin = sys.maxsize
        ymin = sys.maxsize
        xmax = ~sys.maxsize
        ymax = ~sys.maxsize
        for actor in self.get_actors():
            bounds = actor.GetBounds()
            xmin = min(xmin, bounds[0])
            ymin = min(ymin, bounds[1])
            xmax = max(xmax, bounds[2])
            ymax = max(ymax, bounds[3])
        return (xmin, ymin, xmax, ymax)


class HoverInfo(QObject):
    hoveredTriangle = Signal(str)

    def __init__(
        self,
        renderer: vtkRenderer,
        interactor_style: vtkInteractorStyleUser,
    ):
        QObject.__init__(self)
        self.renderer = renderer
        self.picker = vtkCellPicker()
        self.picker.PickFromListOn()
        self.geo = None
        interactor_style.AddObserver(
            vtkCommand.MouseMoveEvent, self.on_mouse_move
        )

    def set_geo(self, geo: Geometry):
        self.geo = geo
        self.picker.InitializePickList()
        self.picker.AddPickList(self.geo.actor)

    def on_mouse_move(self, obj, evt):
        if not self.geo:
            return
        interactor = obj.GetInteractor()
        pos = interactor.GetEventPosition()
        self.picker.Pick(pos[0], pos[1], 0, self.renderer)
        cell_id = self.picker.GetCellId()
        self.hoveredTriangle.emit(str(cell_id) if cell_id != -1 else "")
