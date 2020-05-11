# Create trajectories with low density

import os
from sys import path

path.append(os.path.dirname(os.path.dirname(path[0])))
from create_trajectories import write_trajectory_grid_to_file

numPedsX = 1
numPedsY = 1
startPosX = 0.25 #starts later to avoid intersection with geometry
startPosY = 5.0
ped_distance = 0

write_trajectory_grid_to_file(numPedsX, numPedsY, startPosX, startPosY, ped_distance)
