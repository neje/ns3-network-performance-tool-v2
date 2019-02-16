#!/bin/bash

V="1 2 3 4 5 6 7 8 9 10"
START="1"
STOP="400"

echo Starting experiment...

for v in $V
do
  for run in {$START..$STOP}
  do
    echo 
    echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    echo x   Run = $run, Speed=$v
    echo x   "./waf --run \"multi-run --nodeSpeed=$v --startRngRun=$START --currentRngRun=$run --stopRngRun=$STOP --externalRngRunControl=1\""
    echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    echo 
    ./waf --run "multi-run --nodeSpeed=$v --startRngRun=$START --currentRngRun=$run --stopRngRun=$STOP --externalRngRunControl=1"
  done
done



