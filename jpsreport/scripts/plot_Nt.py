# Usage:
# plot_Nt.py --help
#------------------
import sys
import os
from xml.dom.minidom import parse
import glob

if "helpers" not in sys.path:
    sys.path.append("helpers")

from Parser import get_parser_args_Nt
from Ploter import plot_nt
from Utilities import get_ids_nt_files

    
if __name__ == '__main__':
    args = get_parser_args_Nt()
    jpsreport_inifile = args.inifile
    Id = args.id
    if not os.path.exists(jpsreport_inifile):
        sys.exit("{} does not exist".format(jpsreport_inifile))
        
    d = parse(jpsreport_inifile)
    # is method A being used?
    method_A =  d.getElementsByTagName('method_A')[0].attributes.items()[0][1]
    if method_A.lower() == "false":
        sys.exit("method A ist false. Nothing to plot") 

    # get Nt-files
    output_dir =  d.getElementsByTagName('output')[0].attributes.items()[0][1]
    jpsreport_ini_dir = os.path.dirname(jpsreport_inifile)
    flow_nt_files = os.path.join(jpsreport_ini_dir, output_dir, "Fundamental_Diagram", "FlowVelocity", "Flow_NT_*.dat")
    nt_files = glob.glob(flow_nt_files)
    if not nt_files:
        sys.exit("No Flow_NT files found in <{}>".format(flow_nt_files))

    ids = get_ids_nt_files(nt_files)
    if not Id in ids:
        sys.exit("{} not in {}".format(Id, ", ".join(map(str,ids))))
    
    plot_nt(Id, nt_files)
    
