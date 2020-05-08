import re
import pandas as pd
import numpy as np
from xml.dom.minidom import parse

def read_IFD(IFD_filename):
    file_IFD = '{0}.dat'.format(IFD_filename)
    df = pd.read_csv('{0}'.format(file_IFD),
                     comment='#',sep='\t',
                     names=['Frame','PersID','x/m','y/m','z/m','rho','vel','Voronoi_Polygon'],
                     index_col=False)
    return df

def str_to_array(p):
    """
    convert jpsreport polygon into <np.array>
    --> can be converted to <Polygon.Polygon>
    """

    if not isinstance(p, str):
        raise TypeError('str_to_Array argument must be str')

    pat = re.compile(r'''(-*\d+\.?\d*, -*\d+\.?\d*),*''')
    matches = pat.findall(p)
    lst = []
    if matches:
        lst = [tuple(map(float, m.split(","))) for m in matches]
    else:
        print("WARNING: could not convert str to list")

    return np.array(lst)

def read_obstacle(xml_doc):
    # Initialization of a dictionary with obstacles
    return_dict = {}
    # read in obstacles and combine them into an array for polygon representation
    for o_num, o_elem in enumerate(xml_doc.getElementsByTagName('obstacle')):

        N_polygon = len(o_elem.getElementsByTagName('polygon'))

        if len(o_elem.getElementsByTagName('polygon')) == 1:
            pass
        else:
            array_temp = np.zeros((N_polygon, 2))

        for p_num, p_elem in enumerate(o_elem.getElementsByTagName('polygon')):

            N_vertex = len(p_elem.getElementsByTagName('vertex'))
            if len(p_elem.getElementsByTagName('vertex')) == 2:

                array_temp[p_num, 0] = p_elem.getElementsByTagName('vertex')[0].attributes['px'].value
                array_temp[p_num, 1] = p_elem.getElementsByTagName('vertex')[0].attributes['py'].value
            else:
                array_temp = np.zeros((N_vertex, 2))

                for v_num, v_elem in enumerate(p_elem.getElementsByTagName('vertex')):
                    array_temp[v_num, 0] = p_elem.getElementsByTagName('vertex')[v_num].attributes['px'].value
                    array_temp[v_num, 1] = p_elem.getElementsByTagName('vertex')[v_num].attributes['py'].value

        return_dict[o_num] = array_temp

    return return_dict


def read_subroom_walls(xml_doc):
    dict_polynom_wall = {}
    n_wall = 0
    for s_num, s_elem in enumerate(xml_doc.getElementsByTagName('subroom')):
        for p_num, p_elem in enumerate(s_elem.getElementsByTagName('polygon')):
            if p_elem.getAttribute('caption') == "wall":
                n_wall = n_wall + 1
                n_vertex = len(p_elem.getElementsByTagName('vertex'))
                vertex_array = np.zeros((n_vertex, 2))
                
                for v_num, v_elem in enumerate(p_elem.getElementsByTagName('vertex')):
                    vertex_array[v_num, 0] = p_elem.getElementsByTagName('vertex')[v_num].attributes['px'].value
                    vertex_array[v_num, 1] = p_elem.getElementsByTagName('vertex')[v_num].attributes['py'].value
                
                
                dict_polynom_wall[n_wall] = vertex_array

    return dict_polynom_wall




        
