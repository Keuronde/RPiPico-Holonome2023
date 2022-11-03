set title "Observation des 3 axes\nT_{acq} : 5 ms - filtre 1 seconde"

set xlabel "Temps (s)"
set ylabel "Vitesse (°/s)"
set grid ytics 

#set term png size 1200,600

plot "./acq_5ms_env_50ms_enr_04.csv" using 1:2 w l title "Vitesse X", "./acq_5ms_env_50ms_enr_04.csv" using 1:3 w l title "Vitesse Y", "./acq_5ms_env_50ms_enr_04.csv" using 1:4 w l title "Vitesse Z"

pause -1