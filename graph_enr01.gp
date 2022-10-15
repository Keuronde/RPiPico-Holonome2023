set title "Observation des 3 axes\nT_{acq} : 5 ms"

set xlabel "Temps (s)"
set ylabel "Vitesse (°/s)"


plot "./acq_5ms_env_50ms_enr_01.csv" using 1:2 w l title "Vitesse X"
pause -1
plot "./acq_5ms_env_50ms_enr_01.csv" using 1:3 w l title "Vitesse Y"
pause -1
plot "./acq_5ms_env_50ms_enr_01.csv" using 1:4 w l title "Vitesse Z"
pause -1



