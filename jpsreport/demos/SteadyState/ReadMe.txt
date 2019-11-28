python ../../scripts/SteadyState.py 
-f ./rho_v_Voronoi_traj_AO_b240.txt_id_1.dat  # file name with the data
-a
-c 0 1 2                                      # column indexes. 0 for the frame column (first).    
-rs 240 240                                   # start references 
-re 640 640                                   # end references
-xl "t"                                       # xlabel. alternative: "frame" 
-yl "\rho\;  / 1/m"  "v\;  m/s" -p yes        # ylabels for every column (optionally with unit)
