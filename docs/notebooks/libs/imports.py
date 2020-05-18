import os
import re # str_to_array help function 
import glob # glob over result files

import numpy as np
import pandas as pd

#---------------- for ploting and animating ------------------------
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import matplotlib.animation as animation
from matplotlib.animation import FuncAnimation
from mpl_toolkits.axes_grid1.inset_locator import inset_axes #colorbar
from matplotlib.patches import Polygon as ppolygon # polygons
from mpl_toolkits.axes_grid1 import make_axes_locatable # profiles
from matplotlib.collections import PatchCollection

#-----------------------------------------------------------------

# Visualisation
from IPython.display import HTML

# parsing xml files
from xml.dom.minidom import parse

# Widgets 
import ipywidgets as widgets

# a colection of helper functions 
from utils import * 






