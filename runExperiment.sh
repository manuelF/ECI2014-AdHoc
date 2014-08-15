#! /bin/bash

RUNS=15
DEFAULT_STATS_FILENAME=manet-routing.stats.csv
FOLDER=runs

SPEED=20
PAUSE=0
MAXSPEED=50
BOXSIDE=200

mkdir -p $FOLDER
AODVFILENAME=$DEFAULT_STATS_FILENAME
AODVFILENAME+=".AODV."
OLSRFILENAME=$DEFAULT_STATS_FILENAME
OLSRFILENAME+=".OLSR."

rm -f olsr_moving_speed
for(( q=1; q<$MAXSPEED; q++))
do

  SPEED=$q
  rm -f reception_stats
  echo "IP, ReceivedMessages, TotalSentMessagesToIt" > reception_stats
  for(( i=0; i<$RUNS; i++ ))
  do
      NS_LOG=OnOffApplication=level_info NS_GLOBAL_VALUE="RngRun=$i" ./waf --run "AODV --nodeSpeed=$SPEED --nodePause=$PAUSE boundingBoxSide=$BOXSIDE" > out 2>&1
      mv $DEFAULT_STATS_FILENAME $FOLDER/$AODVFILENAME$i ;
  #    NS_GLOBAL_VALUE="RngRun=$i" ./waf --run "OLSR --nodeSpeed=$SPEED --nodePause=$PAUSE"
  #    mv $DEFAULT_STATS_FILENAME $FOLDER/$OLSRFILENAME$i
  # NS_LOG=OnOffApplication=level_info  << de aca parsear


  #para calcular el porcentaje de mensajes recibidos por cada nodo
      for((j=1; j<11; j++ ))
      do
        IP="10.1.1.$j"
        MENSAJESENVIADOSAESTA=$( grep -w $IP  out | grep -c "on-off")
        MENSAJESRECIBIDOS=$( grep -w $IP out | grep -c "received")
        echo "$IP,$MENSAJESRECIBIDOS,$MENSAJESENVIADOSAESTA" >> reception_stats
      done
  done

  echo $q $(cat reception_stats | tr ',' '\t' | awk '{REC+=$2; SNT+=$3;}END {print REC,  SNT}')  >> olsr_moving_speed

done
