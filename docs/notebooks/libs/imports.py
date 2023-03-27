import glob  # glob over result files
import os
import re  # str_to_array help function
# parsing xml files
from xml.dom.minidom import parse

# Widgets 
import ipywidgets as widgets
import matplotlib.animation as animation
import matplotlib.cm as cm
#---------------- for ploting and animating ------------------------
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
# Visualisation
from IPython.display import HTML
from matplotlib.animation import FuncAnimation
from matplotlib.collections import PatchCollection
from matplotlib.patches import Polygon as ppolygon  # polygons
from mpl_toolkits.axes_grid1 import make_axes_locatable  # profiles
from mpl_toolkits.axes_grid1.inset_locator import inset_axes  # colorbar
# a colection of helper functions 
from utils import *

#-----------------------------------------------------------------
