set output 'moving_speed_single_node_best_aodv.png'
set terminal pngcairo size 640,480 enhanced font "Verdana,10"
set title 'Plano de lado 800, velocidad variable, sin pausa, misma semilla, mejor AODV'

set datafile separator ","
set ylabel 'Cantidad de paquetes recibidos'
set xlabel 'Velocidad de los nodos [m/s]'

plot  "moving_speed_single_node_best_aodv.dat" using 2 title 'AODV' with lines, \
       "moving_speed_single_node_best_aodv.dat" using 3 title 'OLSR' with lines
