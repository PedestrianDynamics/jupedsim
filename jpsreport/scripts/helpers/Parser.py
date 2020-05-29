import argparse

def get_parser_args_Nt():
    """parse arguments for plot_Nt.py

    :returns: argument parser
    :rtype: 

    """
    parser = argparse.ArgumentParser(description='Plot N-t graph measured by method A')
    parser.add_argument("-f", "--inifile", help='jpsreport inifile (required)', required=True)
    parser.add_argument("-i", "--id", help='id of the measurement area (optional)', nargs='?', default=-1, type=int)
    args = parser.parse_args()
    return args


def get_parser_args_prf():
    """parse arguments for plot_profiles.py

    :returns: argument parser
    :rtype: 

    """
    parser = argparse.ArgumentParser(description='Plot Profiles of density, velocity and flow')
    parser.add_argument("-f", "--inifile", help='jpsreport inifile (required)', required=True)
    parser.add_argument("-i", "--id", help='id of the measurement area (optional)', nargs='?', default=-1, type=int)
    parser.add_argument("-b", "--beginsteady", type=int, required=False, help='the frame for the beginning of '
                        'steady state')
    parser.add_argument("-e", "--endsteady", type=int, required=False, help='the frame for the ending of '
                        'steady state')
    args = parser.parse_args()
    return args


def get_parser_args_poly():
    """parse arguments for plot_polygons.py

    :returns: argument parser
    :rtype:

    """
    parser = argparse.ArgumentParser(description='Plot Voronoi polygons')
    parser.add_argument("-f", "--inifile", help='jpsreport inifile (required)', required=True)
    parser.add_argument("-i", "--id", help='id of the measurement area (optional)', nargs='?', default=-1, type=int)
    parser.add_argument("-b", "--beginsteady", type=int, required=False, help='the frame for the beginning of '
                        'steady state (optional)')
    parser.add_argument("-e", "--endsteady", type=int, required=False, help='the frame for the ending of '
                        'steady state (optional)')
    args = parser.parse_args()
    return args

def get_parser_args_profile2():
    """parse arguments for plot_profiles2.py

    :returns: argument parser
    :rtype: 

    """
    parser = argparse.ArgumentParser(description='Plot Profiles of density and velocity with the geometry')
    parser.add_argument("-f", "--inifile", help='jpsreport inifile with full path (required)', required=True)
    parser.add_argument("-b", "--beginFrame", type=int, required=False, help='Frame start')
    parser.add_argument("-e", "--endFrame", type=int, required=False, help='Frame end')
    parser.add_argument("-x", "--size", type=float, default=0.2, required=False, help='Discretisation size')

    args = parser.parse_args()
    return args
