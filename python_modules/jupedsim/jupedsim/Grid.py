import numpy as np
from math import sqrt


class Grid:
    """Class to save points and check for neighbours within a radius

    box : an Axis Aligned Bounding Box where the Grid will be able to save points
    agent_radius : radius in which points are searched for
    """
    def __init__(self, box, agent_radius):
        self.box = box
        self.a_r = agent_radius
        width, height = box[1][0] - box[0][0], box[1][1] - box[0][1]
        # Cell side length
        self._c_s_l = agent_radius / np.sqrt(2)
        # Number of cells in the x- and y-directions of the grid
        self._nx, self._ny = int(width / self._c_s_l) + 1, int(height / self._c_s_l) + 1
        # A list of coordinates in the grid of cells
        self.coords_list = [(ix, iy) for ix in range(self._nx) for iy in range(self._ny)]
        # Initialize the dictionary of cells: each key is a cell's coordinates, the
        # corresponding value is the index of that cell's point's coordinates in the
        # samples list (or None if the cell is empty).
        self.cells = {coords: None for coords in self.coords_list}
        self.samples = []

    def append_point(self, pt):
        cell_coords = self.get_cell_coords(pt)
        self.cells[cell_coords] = pt
        self.samples.append(pt)

    def get_samples(self):
        """returns a copy of the samples saved"""
        return self.samples[:]

    def get_cell_coords(self, pt):
        """ Get the coordinates of the cell that pt = (x,y) falls in.
            box is bounding box containing the minimal/maximal x and y values"""
        return int((pt[0] - self.box[0][0]) // self._c_s_l), int((pt[1] - self.box[0][1]) // self._c_s_l)

    def no_neighbours_in_distance(self, pt):
        coords = self.get_cell_coords(pt)
        return not self.has_neighbour_in_distance(pt, coords)

    def has_neighbour_in_distance(self, pt, coords):
        """"returns true if there is any point in grid with lt or equal the distance `agent radius` to `pt`"""
        dxdy = [(-1, -2), (0, -2), (1, -2), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1),
                (-2, 0), (-1, 0), (1, 0), (2, 0), (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1),
                (-1, 2), (0, 2), (1, 2), (0, 0)]
        for dx, dy in dxdy:
            neighbour_coords = coords[0] + dx, coords[1] + dy
            if not (0 <= neighbour_coords[0] < self._nx and
                    0 <= neighbour_coords[1] < self._ny):
                # Points are not on the grid
                continue
            neighbour = self.cells[neighbour_coords]
            if neighbour is not None:
                # Inside a Cell close the the point is a potential neighbour
                dif_y, dif_x = neighbour[1] - pt[1], neighbour[0] - pt[0]
                distance = sqrt((dif_x**2) + (dif_y**2))
                if distance < self.a_r:
                    return True
        return False

