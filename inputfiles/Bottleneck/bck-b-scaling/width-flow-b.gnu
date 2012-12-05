set term postscript enh 18 color
set size square 

set output "width-flow-b.eps"

set xrange[0:2.6]
set yrange[0:6]
set xlabel "b [m]"
set ylabel "J [1/s]"
set key top left

plot \
"seyfried-j-b.dat" u ($1):($4) t 'Seyfried' w p pt 8 lc 7,\
"kretz-j-b.dat" u ($1):2 title 'Kretz'  w p pt 2 lc 3,\
"mueller-bg-32-No.dat" u ($1):($2/$3) title 'Mueller b_g=3.2m' w p pt 1 lc 4,\
"mueller-bg-26-No.dat" u ($1):($2/$3) title 'Mueller b_g=2.6m' w p pt 10 lc 2,\
"flow-Str2-meth2-len250-A0.2.dat" u ($1):($2) title 'Simulation' w p pt 10 lc 3,\
 1.9*(x) title "1.9 b" w l lw 2 lt 1 lc 7

