# Create trajectories with low density

import os
from sys import path

path.append(os.path.dirname(os.path.dirname(path[0])))
from create_trajectories import write_trajectory_grid_to_file

numPedsX = 6
numPedsY = 6
startPosX = 0.0
startPosY = 9.0
ped_distance = 1.5

write_trajectory_grid_to_file(numPedsX, numPedsY, startPosX, startPosY, ped_distance)
