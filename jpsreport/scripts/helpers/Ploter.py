import os
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from mpl_toolkits.axes_grid1 import make_axes_locatable
from matplotlib.patches import Polygon as ppolygon # polygons
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
    
import numpy as np
import glob
from xml.dom.minidom import parse
from Utilities import read_obstacle
from Utilities import read_subroom_walls
from Utilities import geo_limits

def plot_nt(measurement_id, files):
    """plot N(t) curves (Method A)

    :param measurement_id: measurement area id
    :param files: list of files -> Fundamental_Diagram/FlowVelocity/Flow_NT_*.dat

    """
    legend = False #show legend if more than one measurement_id is plotted
    fig = plt.figure(dpi=300)
    ax1 = fig.add_subplot(111, aspect='auto')
    for f in files:
        data = np.loadtxt(f)
        Id = int(f.split("id_")[-1].split(".")[0])
        if measurement_id == -1:
            plt.plot(data[:, 0], data[:, 1], label="{}".format(Id))
            legend = True
            continue

        if Id == measurement_id:
            plt.plot(data[:, 0], data[:, 1])

    plt.xlabel("t")
    plt.ylabel("N")
    if legend:
        plt.legend()

    figname = "Nt_{}.png".format(measurement_id)
    plt.savefig(figname)
    print("---> ", figname)

def get_profiles(Id, field_dir, geo_filename, beginsteady=None, endsteady=None):
    """ Plot density, velocity and flow

    Note: J = rho*v

    :param Id: measurement area
    :param field_dir:
    :param beginsteady: From frame. If None, then take first frame
    :param endsteady: End frame. If None, then take last frame
    :param geo_filename: geometry file as parsed from the inifile
    :returns: png-plot
    :rtype:

    """
     
    density_files = os.path.join(field_dir, "density",  "Prf_*id_{}_*".format(Id))
    velocity_files = os.path.join(field_dir, "velocity",  "Prf_*id_{}_*".format(Id))
    v_Voronoi = glob.glob(velocity_files)
    f_Voronoi = glob.glob(density_files)
    # get frames
    frames = []
    for f in v_Voronoi:
        frame = int(f.split("_")[-1].split(".")[0]) #Prf_d_*_id_%d_%.5d.dat
        frames.append(frame)

    frames = sorted(frames)
    # correct arguments if necessary
    if beginsteady is None:
        beginsteady = frames[0]
    elif beginsteady < frames[0]:
        beginsteady = frames[0]

    if endsteady is None:
        endsteady = frames[-1]
    elif endsteady > frames[-1]:
        endsteady = frames[-1]

    beginsteady -= frames[0]
    endsteady -= frames[0]

    # get the shape of the matrices
    shape = np.shape(np.loadtxt(f_Voronoi[0]))
    density = np.zeros(shape)
    velocity = np.zeros(shape)
    xml_datei = open(geo_filename, "r")
    geo_xml = parse(xml_datei)
    xml_datei.close()
    geominX, geomaxX, geominY, geomaxY = geo_limits(geo_xml)

    geometry_wall = read_subroom_walls(geo_xml)
    geometry_obst = read_obstacle(geo_xml)
#  -------- density
    for density_file in f_Voronoi[beginsteady:endsteady+1]:
        if os.path.exists(density_file):
            density += np.loadtxt(density_file)

    density = density / (endsteady-beginsteady)
#  --------- velocity
    for velocity_file in v_Voronoi[beginsteady:endsteady+1]:
        if os.path.exists(velocity_file):
            velocity += np.loadtxt(velocity_file)

    velocity = velocity / (endsteady-beginsteady)
    flow = density * velocity
   
 # plot
    figs, axs = plt.subplots(3, 1)
    axs[0].set_aspect('equal')

    for i in range(3):
        plot_geometry(axs[i], geometry_obst)
        plot_geometry(axs[i], geometry_wall)
        
    vmax_density = np.mean(density)+np.std(density)
    vmax_velocity = np.mean(velocity)+np.std(velocity)

    im1 = axs[0].imshow(density,
                        cmap=cm.jet,
                        interpolation='nearest', origin='lower',
                        vmin=0, vmax=vmax_density,
                        extent=[geominX, geomaxX, geominY, geomaxY])

    im2 = axs[1].imshow(velocity,
                        cmap=cm.jet,
                        interpolation='nearest', origin='lower',
                        vmin=0, vmax=vmax_velocity,
                        extent=[geominX, geomaxX, geominY, geomaxY])

    im3 = axs[2].imshow(flow,
                        cmap=cm.jet, interpolation='nearest', origin='lower',
                        vmin=0, vmax=vmax_density*vmax_velocity,
                        extent=[geominX, geomaxX, geominY, geomaxY])

    axs[0].set_xlabel("x [m]")
    axs[0].set_ylabel("y [m]")
    divider1 = make_axes_locatable(axs[0])
    cax1 = divider1.append_axes("right", size="3.5%", pad=0.3)
    cb1 = plt.colorbar(im1, cax=cax1)
    cb1.set_label('Density [$m^{-2}$]')

    axs[1].set_xlabel("x [m]")
    axs[1].set_ylabel("y [m]")
    divider2 = make_axes_locatable(axs[1])
    cax2 = divider2.append_axes("right", size="3.5%", pad=0.3)
    cb2 = plt.colorbar(im2, cax=cax2)
    cb2.set_label('Velocity [$m/s$]')

    axs[2].set_xlabel("x [m]")
    axs[2].set_ylabel("y [m]")
    divider3 = make_axes_locatable(axs[2])
    cax3 = divider3.append_axes("right", size="3.5%", pad=0.3)
    cb3 = plt.colorbar(im3, cax=cax3)
    cb3.set_label('Flow [$1/m \cdot s$]')
    plt.tight_layout()
    figname = "profiles_{}.png".format(Id) # todo: depend on trajectories name
    plt.plot(figname)
    plt.savefig(figname)
    print("--->", figname)

def IFD_plot_polygon_rho(Id, frame, geo_filename):
    """Plot Voronoi polygons with measurement areas
    
    :param Id: 
    :param frame: 
    :returns: 
    :rtype: 

    """

    m =  d.getElementsByTagName('measurement_areas')
    verteces = []
    for o_num, o_elem in enumerate(m[0].getElementsByTagName('area_B')): 
        if o_elem.getAttribute('id') == str(Id):
            n_vertex = len(o_elem.getElementsByTagName('vertex'))
            verteces = np.zeros((n_vertex, 2))
            
            for v_num, v_elem in enumerate(o_elem.getElementsByTagName('vertex')):
                    verteces[v_num, 0] = o_elem.getElementsByTagName('vertex')[v_num].attributes['x'].value
                    verteces[v_num, 1] = o_elem.getElementsByTagName('vertex')[v_num].attributes['y'].value
            
    xml_datei = open(geo_filename, "r")
    geo_xml = parse(xml_datei)
    xml_datei.close()

    geometry_wall = read_subroom_walls(geo_xml)
    geometry_obst = read_obstacle(geo_xml)
    
    files = glob.glob(os.path.join(
        jpsreport_ini_dir, 
        output_dir, 
        "Fundamental_Diagram", 
        "IndividualFD", 
        "*id_{}.dat".format(Id)))
    
    df_poly = pd.read_csv(files[0],
                          comment='#',sep='\t',
                          names=['Frame','PersID','x','y','z','rho','vel','poly'],
                          index_col=False)
    
    # set the index to be this and don't drop
    df_poly.set_index(keys=['Frame'], drop=False, inplace=True)

    fig, ax = plt.subplots()
    ax.set_aspect('equal')

    for g in geometry_obst.keys():
        ax.add_patch(ppolygon(geometry_obst[g], color='gray'))

    for gw in geometry_wall.keys():
        ax.plot(geometry_wall[gw][:, 0], geometry_wall[gw][:, 1], color='black', lw=2)
        
    sm = cm.ScalarMappable(cmap = cm.get_cmap('rainbow'))
 
    df_0 = df_poly[df_poly.Frame == frame]
    polys = df_0['poly']
    rhos = df_0['rho']
    X = df_0['x']
    Y = df_0['y']
    sm.set_clim(vmin=0, vmax=6) # todo max rho
    for x, y, rho, poly in zip(X, Y, rhos, polys):
        p = str_to_array(poly)/10000 
        patch = ppolygon(p,
                     fc= sm.to_rgba(rho),
                     ec= 'white',
                     lw=1)
        ax.add_patch(patch)

# plot measurement area
    plt.plot(verteces[:, 0], verteces[:,1],'-r')

    plt.axis('off')    

    axins = inset_axes(ax,
                   width="5.5%",  # width = 5% of parent_bbox width
                   height="100%",  # height : 50%
                   loc='lower left',
                   bbox_to_anchor=(1, 0., 1, 1),
                   bbox_transform=ax.transAxes,
                   borderpad=0,
                   )
    fig.colorbar(sm, ax=ax, cax=axins)    
