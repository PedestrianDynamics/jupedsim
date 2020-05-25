# Usage:
# plot_profiles.py --help
#------------------
import sys
import glob
import os
from xml.dom.minidom import parse

if "helpers" not in sys.path:
    sys.path.append("helpers")

from Parser import get_parser_args_prf
from Utilities import get_ids_prf_files
from Ploter import plot_profiles

if __name__ == '__main__':
    args = get_parser_args_prf()
    jpsreport_inifile = args.inifile
    beginsteady = args.beginsteady
    endsteady = args.endsteady
    Id = args.id
    if not os.path.exists(jpsreport_inifile):
        sys.exit("{} does not exist".format(jpsreport_inifile))

    # setup important directories/files
    d = parse(jpsreport_inifile)
    output_dir =  d.getElementsByTagName('output')[0].attributes.items()[0][1]
    jpsreport_ini_dir = os.path.dirname(jpsreport_inifile)
    field_dir = os.path.join(jpsreport_ini_dir, output_dir, "Fundamental_Diagram", "Classical_Voronoi", "field")
    geo_filename= os.path.join(os.path.dirname(jpsreport_inifile), d.getElementsByTagName('geometry')[0].attributes.items()[0][1])

    # get ids of measurement areas
    ids = get_ids_prf_files(field_dir)
    if not ids:
        sys.exits("no prf files found in <{}>".format(field_dir))

    if not Id in ids and Id != -1: # -1 is ALL
        sys.exit("{} not in {}".format(Id, ", ".join(map(str,ids))))

    if Id == -1:
        for Id in ids:
            plot_profiles(Id, field_dir, geo_filename, beginsteady, endsteady)
    else:
        plot_profiles(Id, field_dir, geo_filename, beginsteady, endsteady)
