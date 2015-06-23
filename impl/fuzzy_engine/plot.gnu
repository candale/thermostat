#
# Demonstrate use of 4th data column to color a 3D surface.
# Also demonstrate use of the pseudodata special file '++'.
# This plot is nice for exploring the effect of the 'l' and 'L' hotkeys.
#
set view 49, 28, 1, 1.48
set xrange [ -0.1 : 2.5 ] noreverse nowriteback
set yrange [ -10 : 110] noreverse nowriteback
set zrange [ -2 : 2 ] noreverse nowriteback
set ticslevel 0
set format cb "%4.1f"
set colorbox user size .03, .6 noborder
set cbtics scale 0

set xtics 0.2
set ytics 10
set xtics 0.2

set samples 25, 25
set isosamples 50, 50

set title "4D data (3D Heat Map)"\
          ."\nFuzzy logic engine decision map"  offset 0,1
set xlabel "x" offset 3, 0, 0
set ylabel "y" offset -5, 0, 0
set zlabel "z" offset 2, 0, 0
set pm3d implicit at s


splot 'experiment.dat' using 1:2:3:4 with pm3d title "4 data columns temp_err/hummidity/temp_trend/decision"
