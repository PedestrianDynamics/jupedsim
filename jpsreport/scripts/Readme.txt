_getGait.py: (savitzky_golay)
    Get gait information directly from 2D trajectory by ditecting each local peaks and valleys from the smoothed curvature information.

txt2xml.py (Also smoothing code inside(Jack & Bernhard)_zero curvature spline)
    Transfer the txt format trajectory file to xml format, which can be used for JPSreport and JPSvis

_plot_cell_rho.py:
    Plot geometry, pedestrian as well as the voronoi diagrams. The pedestrian density in each diagram is shown by different colors.

_plot_cell_v.py:
    Plot geometry, pedestrian as well as the voronoi diagrams. The velocity for each pedestrian in the diagram is shown by different colors.

_plot_FD.py:
    Plot fundamental diagrams from the output of JPSreport

_plot_N_t.py:
    Plot the N-t diagram obtained from method A.

_Plot_profiles.py
    Plot density, velocity and specific profiles based on the output from JPSreport

_SteadyState.py
    detect the begin and the end of steady state from time series of density and velocity.

txt2txt.py
    tranfer .txt format trajectory files to meet the requirement of JPSreport

2Dto1D_ID.py
    Transfer 2D trajectory information to 1D information. This script is only used for single file experiment in a ring.

correct_trajectories.py
    This programm takes a trajectory file and a geometry file and move the points outside of obstacles and away from walls.
