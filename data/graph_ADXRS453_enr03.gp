set title "ADXRS453 - Axe Z\nT_{acq} : 5 ms - filtre 10 s"

set xlabel "Temps (s)"
set ylabel "Vitesse (°/s)"
set grid ytics 

#set term png size 1200,600

plot "./acq_ADXRS453_5ms_filtre_0_001_enr03.csv" using 1:4 w l title "Vitesse Z"

pause -1