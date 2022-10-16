set title "Observation des 3 axes\nT_{acq} : 5 ms - filtre 15 secondes"

set xlabel "Temps (s)"
set ylabel "Vitesse (°/s)"
set grid ytics 

set term png size 1200,600

plot "./acq_5ms_env_50ms_enr_02.csv" using 1:2 w l title "Vitesse X"

