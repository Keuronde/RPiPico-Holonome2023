set title "ADXRS453 - T_{acq} : 2 ms\nCalibration : 10 secondes"

set xlabel "Temps (s)"
set ylabel "Angle (°)"
set yrange [-0.5:0.5]
set grid ytics 

set term png size 1200,600

plot "./acq_ADXRS453_angle_enr01.csv" using 1:2 w l title "Enr 01",  \
"./acq_ADXRS453_angle_enr02.csv" using 1:2 w l title "Enr 02",  \
"./acq_ADXRS453_angle_enr03.csv" using 1:2 w l title "Enr 03", \
"./acq_ADXRS453_angle_enr04.csv" using 1:2 w l title "Enr 04", \
"./acq_ADXRS453_angle_enr05.csv" using 1:2 w l title "Enr 05", \
"./acq_ADXRS453_angle_enr06.csv" using 1:2 w l title "Enr 06", \
"./acq_ADXRS453_angle_enr07.csv" using 1:2 w l title "Enr 07"

pause -1