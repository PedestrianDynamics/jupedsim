import glob

DIR = "../Andjiana/90deg/Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell"

files = glob.glob("%s/hybrid_hall_traj*.dat"%DIR)

for f in files:
    ff = f.split(".dat")[0]
    cmd = python plot_voronoi.py  -f DIR -n ff -x1 -2 -x2 14 -y1 -2  -y2 14
    subprocess.call(["python", ])

    
