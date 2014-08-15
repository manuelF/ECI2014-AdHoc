set output 'moving_speed_800_size.png'
set terminal pngcairo size 640,480 enhanced font "Verdana,10"
set title 'Plano de lado 800, velocidad variable, sin pausa'

set datafile separator ","
set ylabel '% todos los paquetes recibidos'
set xlabel 'Velocidad de los nodos [m/s]'

plot  "moving_speed_800.dat" using 2 title 'AODV' with lines, \
       "moving_speed_800.dat" using 3 title 'OLSR' with lines
