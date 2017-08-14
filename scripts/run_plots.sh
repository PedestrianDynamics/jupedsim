# rm log*.py
# rm *.png

#./bin/jpscore inputfiles/voronoi/voronoi_hall_ini.xml


for p in log*.py
do
    python $p
done

open *.png

         


