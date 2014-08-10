Redes moviles Ad-hoc
Manuel Ferreria
====================

Cambios realizados
==================

Metodologia
===========

Para realizar el analisis de performance de los distintos protocolos de enrutamiento
primero armamos un framework de pruebas. En este caso, es un script (*runExperiment.sh*)
que se encarga de correr ambos protocolos usando la misma disposicion inicial de nodos
y movimientos y capturar las salidas en archivos apropiados. Esto se hace fijando
la semilla del RNG de cada corrida de modo que se pueda recrear todos los mismos
movimientos de los nodos usando las mismas posiciones iniciales (por semilla).

Hemos fijado en 50 nodos la cantidad que vamos a utilizar para los analisis, al ser
este un numero no trivial de nodos que interactuen. A su vez, mantendremos las
asignaciones de tareas que se plantearon en la guia orientativa, donde los nodos
0 a 9 recibien mensajes de los nodos 10 a 19, y los demas solo sirven en la red para
retransmitir.

Parametros
=========

Para cambiar la densidad de los nodos, cambiamos el tama\~no del bounding box
que los contiene. Para hacer eso, debemos alterar los metodos que inicializan
el ObjectFactory. Para simplificar el problema, nos importaran solamente
cuadrados, no rectangulos.

Definimos como red poco densa al cuadrado de lado 800 y como red muy densa
al cuadrado de lado 100.

Para cambiar la movilidad de los nodos, podemos alterar dos parametros del modelo.
Estos son, nodeSpeed y nodePause. Segun el modelo de RandomWaypoint, ambos
determinan el comportamiento de los nodos con respecto a su posicion en el
espacio (en nuestro caso, el plano Z=0). La velocidad del nodo altera el tiempo
que el nodo va a estar desplazandose hacia un punto elegido aleatoriamente;
la pausa es el tiempo que va a esperar al llegar a destino hasta empezar a
desplazarse nuevamente.

Estudiaremos el problema fijando la pausa en 1s y variando la velocidad de los
nodos desde 1m/s (velocidad de un ser humano caminando) hasta 50m/s
(por ejemplo, automoviles).


