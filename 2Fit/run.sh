#!/bin/bash

# ./run [310-340] 300000 60

runnum=$1

totalentry=$2

kernel=1
while(( $kernel<=$3 ))
do
   ./ana $runnum `expr $kernel \* $2 / $3 - $2 / $3` `expr $kernel \* $2 / $3 - 1` &
   #echo "./ana $runnum `expr $kernel \* $2 / $3 - $2 / $3` `expr $kernel \* $2 / $3 - 1`"
    
   let "kernel++"
done