#!/bin/bash

#V="1 2 3 4 5 6 7 8 9 10"
V="5"
START="1"
STOP="10"

echo Starting experiment...

for v in $V
do
  for (( run=$START; run<=$STOP; run++ ))
  do
    echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    echo x   Run = $run, Speed=$v
    echo x   "./waf --run \"multi-run --nodeSpeed=$v --startRngRun=$START --currentRngRun=$run --stopRngRun=$STOP --externalRngRunControl=1\""
    echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    echo "Start time:"
    date
    ./waf --run "multi-run --nodeSpeed=$v --startRngRun=$START --currentRngRun=$run --stopRngRun=$STOP --externalRngRunControl=1"
    echo "StopTime:"
    date
    echo --------------------------------------------------------------------------
  done
done



