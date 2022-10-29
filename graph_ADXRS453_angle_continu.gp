set title "ADXRS453 - T_{acq} : 2 ms\nCalibration : 10 secondes\nGyroscope continuellement sous tension"

set xlabel "Temps (s)"
set ylabel "Angle (°)"
set yrange [-1:1]
set xrange [10:100]
set grid ytics 

set term png size 1200,600

plot "./acq_ADXRS453_angle_continu_enr01.csv" using 1:2 w l title "Enr 01",  \
"./acq_ADXRS453_angle_continu_enr02.csv" using 1:2 w l title "Enr 02",  \
"./acq_ADXRS453_angle_continu_enr03.csv" using 1:2 w l title "Enr 03", \
"./acq_ADXRS453_angle_continu_enr04.csv" using 1:2 w l title "Enr 04", \
"./acq_ADXRS453_angle_continu_enr05.csv" using 1:2 w l title "Enr 05"

pause -1