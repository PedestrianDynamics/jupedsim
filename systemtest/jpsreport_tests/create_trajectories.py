# Script for creating artifical trajectories for testing purposes.
# Pedestrians move with a constant speed und do not interact with each other.
# They are aligned in a grid that is moved along x-axis for each frame.

# numPedsX: number of columns of pedestrians in the grid
# startPosX, startPosY: start position, upper left corner of the grid
# numPedsY: number of rows of pedestrians in the grid
# ped_distance: distance between the pedestrians (vertically and horizontally)
def write_trajectory_grid_to_file(numPedsX, numPedsY, startPosX, startPosY, ped_distance):
    v = 1  # x-velocity in m/s
    sim_time = 20  # simulation time in s
    fps = 12  # frames per second
    file_name = "trajectory_grid_" + str(numPedsX) + "x" + str(numPedsY) + ".txt"
    geometry_file = "geometry.xml"

    # define fixed ellipse parameters to allow visualization with jpsvis
    a = 0.2
    b = 0.2
    angle = 0.0
    color = 220

    sim_frames = sim_time * fps

    f = open(file_name, "w")
    # write header

    f.write("#framerate: {:.2f}\n#geometry: {}".format(fps, geometry_file))

    f.write(
        "#ID: the agent ID \n#FR: the current frame\n#X,Y,Z: the agents coordinates (in metres)\n#A, B: semi-axes of the ellipse\n#ANGLE: orientation of the ellipse\n#COLOR: color of the ellipse\n\n#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\n")

    for frame in range(sim_frames):

        # calc position of peds in grid based on start pos
        for i in range(numPedsX):
            for j in range(numPedsY):
                # calc id based on current row and column
                id = numPedsX * j + i + 1

                xPos = startPosX + i * ped_distance
                yPos = startPosY - j * ped_distance

                f.write(
                    "{:6d}\t{:6d}\t{:.6f}\t{:.6f}\t{:.6f}\t{:.2f}\t{:.2f}\t{:.2f}\t{:d}\n".format(id, frame + 1, xPos,
                                                                                                  yPos,
                                                                                                  0.0, a, b,
                                                                                                  angle, color))

        # move grid to the right for next frame
        startPosX += v / fps

    f.close()
