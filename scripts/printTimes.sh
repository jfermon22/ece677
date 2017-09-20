#!/bin/bash

if ! [ -z "$1" ]; then
   if [ -d ../outputs/$1 ]; then
      cd ../outputs/$1
   else 
      echo "No directory ../output/$1 exists"
      exit 1
   fi
fi

for EXEC_NAME in "matrix_multiply" "mean_filter"
do
   echo "$EXEC_NAME Serial: "  `cat $EXEC_NAME\_serial.txt | tail -1 | awk '{print $2}'`

   for NODES in 02 04 08 16 32 64
   do 
      echo "$EXEC_NAME $NODES: "  `cat $EXEC_NAME\_out_$NODES.txt | tail -1 | awk '{print $2}'` `cat  $EXEC_NAME\_status_$NODES.txt`
   done
   
done
