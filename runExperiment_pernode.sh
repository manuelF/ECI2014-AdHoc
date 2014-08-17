#! /bin/bash

RUNS=100
DEFAULT_STATS_FILENAME=manet-routing.stats.csv
FOLDER=runs

SPEED=20
PAUSE=0
MAXSPEED=50
BOXSIDE=800

mkdir -p $FOLDER
AODVFILENAME=$DEFAULT_STATS_FILENAME
AODVFILENAME+=".AODV."
OLSRFILENAME=$DEFAULT_STATS_FILENAME
OLSRFILENAME+=".OLSR."

CURRENT=OLSR

for(( i=0; i<$RUNS; i++ ))
do

  rm -f reception_stats
  echo "IP, ReceivedMessages, TotalSentMessagesToIt" > reception_stats

  for(( q=1; q<$MAXSPEED; q++))
  do
      SPEED=$q
      NS_LOG=OnOffApplication=level_info NS_GLOBAL_VALUE="RngRun=$i" ./waf --run "$CURRENT --nodeSpeed=$SPEED --nodePause=$PAUSE boundingBoxSide=$BOXSIDE" > out 2>&1
      mv $DEFAULT_STATS_FILENAME $FOLDER/$AODVFILENAME$i ;

  #para calcular el porcentaje de mensajes recibidos por cada nodo
      for((j=1; j<11; j++ ))
      do
        IP="10.1.1.$j"
        MENSAJESENVIADOSAESTA=$( grep -w $IP  out | grep -c "on-off")
        MENSAJESRECIBIDOS=$( grep -w $IP out | grep -c "received")
        echo "$IP,$MENSAJESRECIBIDOS,$MENSAJESENVIADOSAESTA" >> reception_stats
      done
  done
  mv reception_stats runs/$CURRENT_reception_stats_seed_$i
done
