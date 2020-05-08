# Create trajectories with high density

import os
from sys import path

path.append(os.path.dirname(os.path.dirname(path[0])))
from create_trajectories import write_trajectory_grid_to_file

numPedsX = 6
numPedsY = 6
startPosX = 0.0
startPosY = 3.0
ped_distance = 0.5

write_trajectory_grid_to_file(numPedsX, numPedsY, startPosX, startPosY, ped_distance)
