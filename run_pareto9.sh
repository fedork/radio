#!/bin/bash
cd ~/radio
LAST_FILE=`ls -t pareto9_*.txt | head -1`
cat $LAST_FILE | ./parse_out.sh >> parsed_260.txt
NUM=`echo ${LAST_FILE:8} | cut -d "." -f 1`
NEXT_FILE="pareto9_$[$NUM + 1].txt"
echo Next file: $NEXT_FILE
./radioSbPareto parsed_260.txt | tee $NEXT_FILE

